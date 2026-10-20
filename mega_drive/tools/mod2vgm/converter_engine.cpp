#include "converter_engine.h"
#include <cmath>
#include <iostream>

namespace mod2vgm {

// Operator register offsets for YM2612 channels 0..2 (Port 0) and 3..5 (Port 1)
static const uint8_t OP_OFFSET[4] = {0x00, 0x08, 0x04, 0x0C};

ConverterEngine::ConverterEngine() {
}

void ConverterEngine::calculate_ym2612_pitch(double freq_hz, uint8_t& block, uint16_t& f_num) {
	if (freq_hz <= 0.0) {
		block = 0;
		f_num = 0;
		return;
	}

	// YM2612 NTSC Master Clock: 7670453 Hz
	// Yamaha OPN2 standard formula:
	// FNUM = (freq_hz * 144 * 2^(21 - block)) / phi
	// Find the block (0..7) where FNUM fits within 11-bit range (0..2047)
	const double phi = 7670453.0;
	block = 0;
	for (uint8_t b = 0; b < 8; ++b) {
		double fn = (freq_hz * 144.0 * static_cast<double>(1ULL << (21 - b))) / phi;
		if (fn < 2048.0 || b == 7) {
			block = b;
			f_num = (fn > 2047.0) ? 2047 : static_cast<uint16_t>(std::round(fn));
			break;
		}
	}
}

void ConverterEngine::apply_fm_patch(VgmWriter& vgm, uint8_t channel, const FmPatch& patch) {
	uint8_t port = (channel >= 3) ? 1 : 0;
	uint8_t ch_offset = channel % 3;

	// Algorithm and Feedback (0xB0 + ch_offset)
	uint8_t fb_algo = ((patch.feedback & 0x07) << 3) | (patch.algorithm & 0x07);
	vgm.write_ym2612(port, 0xB0 + ch_offset, fb_algo);

	// Pan L/R + AMS/FMS (0xB4 + ch_offset) -> Default stereo center (0xC0)
	vgm.write_ym2612(port, 0xB4 + ch_offset, 0xC0);

	for (int op = 0; op < 4; ++op) {
		uint8_t reg_base = OP_OFFSET[op] + ch_offset;
		const FmOperator& o = patch.op[op];

		// DT / MUL (0x30)
		vgm.write_ym2612(port, 0x30 + reg_base, ((o.dt & 0x07) << 4) | (o.mul & 0x0F));
		// TL (0x40)
		vgm.write_ym2612(port, 0x40 + reg_base, o.tl & 0x7F);
		// KS / AR (0x50)
		vgm.write_ym2612(port, 0x50 + reg_base, ((o.ks & 0x03) << 6) | (o.ar & 0x1F));
		// AM / DR (0x60)
		vgm.write_ym2612(port, 0x60 + reg_base, o.dr & 0x1F);
		// SR (0x70)
		vgm.write_ym2612(port, 0x70 + reg_base, o.sr & 0x1F);
		// SL / RR (0x80)
		vgm.write_ym2612(port, 0x80 + reg_base, ((o.sl & 0x0F) << 4) | (o.rr & 0x0F));
		// SSG-EG (0x90)
		vgm.write_ym2612(port, 0x90 + reg_base, o.ssg_eg & 0x0F);
	}
}

void ConverterEngine::note_on(VgmWriter& vgm, uint8_t channel, uint16_t period) {
	if (period == 0) return;

	double freq = ModFile::period_to_hz(period);
	uint8_t block = 0;
	uint16_t f_num = 0;
	calculate_ym2612_pitch(freq, block, f_num);

	uint8_t port = (channel >= 3) ? 1 : 0;
	uint8_t ch_offset = channel % 3;
	uint8_t ch_val = (channel >= 3) ? (channel + 1) : channel;

	// 1. Key OFF first so the hardware envelope re-attacks cleanly
	vgm.write_ym2612(0, 0x28, 0x00 | ch_val);

	// 2. Set Frequency registers
	vgm.write_ym2612(port, 0xA0 + ch_offset, static_cast<uint8_t>(f_num & 0xFF));
	uint8_t blk_fnum_hi = ((block & 0x07) << 3) | static_cast<uint8_t>((f_num >> 8) & 0x07);
	vgm.write_ym2612(port, 0xA4 + ch_offset, blk_fnum_hi);

	// 3. Key ON
	vgm.write_ym2612(0, 0x28, 0xF0 | ch_val);
}

void ConverterEngine::note_off(VgmWriter& vgm, uint8_t channel) {
	uint8_t ch_val = (channel >= 3) ? (channel + 1) : channel;
	vgm.write_ym2612(0, 0x28, 0x00 | ch_val); // Turn off all 4 operators
}

void ConverterEngine::update_channel_frequency(VgmWriter& vgm, uint8_t channel, uint16_t period) {
	if (period == 0) return;
	double freq = ModFile::period_to_hz(period);
	uint8_t block = 0;
	uint16_t f_num = 0;
	calculate_ym2612_pitch(freq, block, f_num);

	uint8_t port = (channel >= 3) ? 1 : 0;
	uint8_t ch_offset = channel % 3;

	vgm.write_ym2612(port, 0xA0 + ch_offset, static_cast<uint8_t>(f_num & 0xFF));
	uint8_t blk_fnum_hi = ((block & 0x07) << 3) | static_cast<uint8_t>((f_num >> 8) & 0x07);
	vgm.write_ym2612(port, 0xA4 + ch_offset, blk_fnum_hi);
}

void ConverterEngine::set_channel_volume(VgmWriter& vgm, uint8_t channel, uint8_t mod_volume, uint8_t algorithm) {
	if (mod_volume > 64) mod_volume = 64;

	// In YM2612: Each step of TL is 0.75 dB of attenuation.
	// MOD volume 0..64 is linear amplitude: gain_dB = 20 * log10(vol / 64)
	// TL attenuation = -gain_dB / 0.75 = -26.6667 * log10(vol / 64)
	uint8_t tl_attenuation = 127;
	if (mod_volume > 0) {
		double tl = -26.6667 * std::log10(static_cast<double>(mod_volume) / 64.0);
		tl_attenuation = (tl > 127.0) ? 127 : static_cast<uint8_t>(std::round(tl));
	}

	uint8_t port = (channel >= 3) ? 1 : 0;
	uint8_t ch_offset = channel % 3;

	// In YM2612, which operators are carriers depends on the algorithm:
	// Algo 0..3: Op 4 (offset 0x0C)
	// Algo 4:    Op 2 (0x08), Op 4 (0x0C)
	// Algo 5..6: Op 2 (0x08), Op 3 (0x04), Op 4 (0x0C)
	// Algo 7:    Op 1 (0x00), Op 2 (0x08), Op 3 (0x04), Op 4 (0x0C)
	const bool is_carrier[8][4] = {
		{false, false, false, true},  // 0
		{false, false, false, true},  // 1
		{false, false, false, true},  // 2
		{false, false, false, true},  // 3
		{false, true,  false, true},  // 4
		{false, true,  true,  true},  // 5
		{false, true,  true,  true},  // 6
		{true,  true,  true,  true}   // 7
	};

	uint8_t algo = algorithm & 0x07;
	for (int op = 0; op < 4; ++op) {
		if (is_carrier[algo][op]) {
			uint8_t base_reg = 0x40 + OP_OFFSET[op] + ch_offset;
			vgm.write_ym2612(port, base_reg, tl_attenuation);
		}
	}
}

void ConverterEngine::set_instrument_patch(uint8_t sample_id, const FmPatch& patch) {
	sample_to_patch_[sample_id] = patch;
}

bool ConverterEngine::convert(const ModFile& mod, const std::string& out_vgm_path, const std::string& config_path) {
	VgmWriter vgm;
	vgm.set_clocks(7670453, 3579545);

	// Setup intelligent instrument patches based on sample waveform analytics and names
	for (size_t i = 1; i <= 31; ++i) {
		if (sample_to_patch_.find(i) != sample_to_patch_.end()) {
			continue; // User custom patch already set
		}

		const ModSample& s = mod.samples[i];
		std::string n = s.name;
		for (auto& c : n) c = std::tolower(c);

		// Acoustic analysis of raw PCM waveform if available
		double zcr = 0.0;
		if (s.pcm_data.size() > 1) {
			size_t crossings = 0;
			for (size_t b = 1; b < s.pcm_data.size(); ++b) {
				if ((s.pcm_data[b] >= 0 && s.pcm_data[b - 1] < 0) ||
				    (s.pcm_data[b] < 0 && s.pcm_data[b - 1] >= 0)) {
					crossings++;
				}
			}
			zcr = static_cast<double>(crossings) / static_cast<double>(s.pcm_data.size());
		}

		if (n.find("bass") != std::string::npos) {
			sample_to_patch_[i] = FmPatchLibrary::get_slap_bass();
		} else if (n.find("pad") != std::string::npos || n.find("string") != std::string::npos) {
			sample_to_patch_[i] = FmPatchLibrary::get_default_pad();
		} else if (n.find("sn") != std::string::npos) {
			sample_to_patch_[i] = FmPatchLibrary::get_snare();
		} else if (n.find("hh") != std::string::npos || n.find("hat") != std::string::npos) {
			sample_to_patch_[i] = FmPatchLibrary::get_hihat();
		} else if (n.find("bd") != std::string::npos || n.find("drum") != std::string::npos) {
			sample_to_patch_[i] = FmPatchLibrary::get_default_percussion();
		} else if (n.find("clean") != std::string::npos) {
			sample_to_patch_[i] = FmPatchLibrary::get_clean_guitar();
		} else {
			// Waveform heuristic:
			if (s.pcm_data.size() > 0 && s.pcm_data.size() < 120) {
				// Single-cycle synth wave (like sample 23, 46 bytes) -> bright Lead guitar
				sample_to_patch_[i] = FmPatchLibrary::get_distortion_guitar();
			} else if (zcr > 0.4) {
				// Very high noise rate -> HiHat / Cymbals (like sample 29)
				sample_to_patch_[i] = FmPatchLibrary::get_hihat();
			} else if (zcr < 0.1 && s.repeat_length_words <= 1 && s.pcm_data.size() < 5000) {
				// Low pitch transient without loop -> Kick drum / Percussion (like sample 2)
				sample_to_patch_[i] = FmPatchLibrary::get_default_percussion();
			} else if (s.pcm_data.size() > 10000) {
				// Very large sample -> sustained chords / power pad (like sample 31)
				sample_to_patch_[i] = FmPatchLibrary::get_default_pad();
			} else if (s.pcm_data.size() > 4000) {
				// Medium large looped sample -> Bass / Rhythm guitar (like sample 28)
				sample_to_patch_[i] = FmPatchLibrary::get_slap_bass();
			} else {
				sample_to_patch_[i] = FmPatchLibrary::get_distortion_guitar();
			}
		}
	}

	// Initialize YM2612 Channels 0, 1, 2, 3 with Lead patch
	for (uint8_t ch = 0; ch < 4; ++ch) {
		apply_fm_patch(vgm, ch, FmPatchLibrary::get_default_lead());
	}

	uint8_t current_speed = 6;     // Default ProTracker ticks per row
	uint8_t current_bpm = 125;     // Default BPM

	// Active state per channel
	uint8_t active_sample[4] = {0, 0, 0, 0};
	uint16_t base_period[4] = {0, 0, 0, 0};
	uint8_t channel_volume[4] = {64, 64, 64, 64};
	uint8_t vibrato_speed[4] = {0, 0, 0, 0};
	uint8_t vibrato_depth[4] = {0, 0, 0, 0};
	uint8_t vibrato_pos[4] = {0, 0, 0, 0};
	uint8_t volslide_val[4] = {0, 0, 0, 0};

	// Standard ProTracker 32-step sine table for vibrato (normalized -127 to +127)
	const int16_t sine_table[32] = {
		0,  24,  49,  74,  97, 120, 141, 161,
		180, 197, 212, 224, 235, 244, 250, 253,
		255, 253, 250, 244, 235, 224, 212, 197,
		180, 161, 141, 120,  97,  74,  49,  24
	};

	vgm.mark_loop_point();

	for (size_t ord = 0; ord < mod.song_length; ++ord) {
		uint8_t pat_idx = mod.order_table[ord];
		if (pat_idx >= mod.patterns.size()) continue;

		const ModPattern& pat = mod.patterns[pat_idx];

		for (size_t row = 0; row < 64; ++row) {
			// Process notes on tick 0
			for (size_t ch = 0; ch < 4; ++ch) {
				const ModCell& cell = pat.rows[row][ch];

				// Handle effect F (Set Speed / Tempo)
				if (cell.effect_code == 0x0F) {
					if (cell.effect_param > 0 && cell.effect_param <= 32) {
						current_speed = cell.effect_param;
					} else if (cell.effect_param > 32) {
						current_bpm = cell.effect_param;
					}
				}

				// Instrument change
				if (cell.sample_id > 0 && cell.sample_id <= 31) {
					active_sample[ch] = cell.sample_id;
					apply_fm_patch(vgm, static_cast<uint8_t>(ch), sample_to_patch_[cell.sample_id]);
					if (cell.effect_code != 0x0C) {
						channel_volume[ch] = mod.samples[cell.sample_id].default_volume;
						set_channel_volume(vgm, static_cast<uint8_t>(ch), channel_volume[ch], sample_to_patch_[cell.sample_id].algorithm);
					}
				}

				// Volume change (Effect C)
				if (cell.effect_code == 0x0C) {
					channel_volume[ch] = cell.effect_param;
					uint8_t algo = (active_sample[ch] > 0) ? sample_to_patch_[active_sample[ch]].algorithm : 4;
					set_channel_volume(vgm, static_cast<uint8_t>(ch), channel_volume[ch], algo);
				}

				// Vibrato effect setup (4xy)
				if (cell.effect_code == 0x04) {
					uint8_t spd = (cell.effect_param >> 4) & 0x0F;
					uint8_t dep = cell.effect_param & 0x0F;
					if (spd > 0) vibrato_speed[ch] = spd;
					if (dep > 0) vibrato_depth[ch] = dep;
				} else if (cell.effect_code != 0x06) {
					vibrato_speed[ch] = 0;
					vibrato_depth[ch] = 0;
				}

				// Vibrato + Volume Slide (6xy)
				if (cell.effect_code == 0x06) {
					volslide_val[ch] = cell.effect_param;
				} else {
					volslide_val[ch] = 0;
				}

				// Note trigger
				if (cell.period > 0) {
					base_period[ch] = cell.period;
					vibrato_pos[ch] = 0;
					note_on(vgm, static_cast<uint8_t>(ch), cell.period);
				}
			}

			// Calculate tick duration in 44100Hz samples:
			uint32_t samples_per_tick = static_cast<uint32_t>((44100.0 * 2.5) / static_cast<double>(current_bpm));

			// Tick 0 already processed
			vgm.wait_samples(samples_per_tick);

			// Intermediate ticks (1 to current_speed - 1): handle vibrato, volume slides
			for (uint8_t tick = 1; tick < current_speed; ++tick) {
				for (size_t ch = 0; ch < 4; ++ch) {
					// Handle volume slides (6xy)
					if (volslide_val[ch] > 0) {
						uint8_t up = (volslide_val[ch] >> 4) & 0x0F;
						uint8_t down = volslide_val[ch] & 0x0F;
						if (up > 0) {
							channel_volume[ch] = (channel_volume[ch] + up > 64) ? 64 : (channel_volume[ch] + up);
						} else if (down > 0) {
							channel_volume[ch] = (channel_volume[ch] < down) ? 0 : (channel_volume[ch] - down);
						}
						uint8_t algo = (active_sample[ch] > 0) ? sample_to_patch_[active_sample[ch]].algorithm : 4;
						set_channel_volume(vgm, static_cast<uint8_t>(ch), channel_volume[ch], algo);
					}

					// Handle vibrato (4xy / 6xy)
					if (vibrato_depth[ch] > 0 && base_period[ch] > 0) {
						int8_t sine_val = sine_table[(vibrato_pos[ch] >> 2) & 31];
						int16_t delta = (sine_val * vibrato_depth[ch]) / 128;
						uint16_t vib_period = static_cast<uint16_t>(base_period[ch] + delta);
						update_channel_frequency(vgm, static_cast<uint8_t>(ch), vib_period);
						vibrato_pos[ch] = (vibrato_pos[ch] + vibrato_speed[ch]) & 127;
					}
				}
				vgm.wait_samples(samples_per_tick);
			}
		}
	}

	// Silence all channels at the end
	for (uint8_t ch = 0; ch < 4; ++ch) {
		note_off(vgm, ch);
	}

	return vgm.write_to_file(out_vgm_path);
}

} // namespace mod2vgm
