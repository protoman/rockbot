#include "converter_engine.h"
#include "replayer.h"
#include <cmath>
#include <iostream>

namespace mod2vgm {

namespace {

// Operator register offsets: YM2612 slot order is OP1, OP3, OP2, OP4.
const uint8_t OP_OFFSET[4] = {0x00, 0x08, 0x04, 0x0C};

const bool IS_CARRIER[8][4] = {
	{false, false, false, true},  {false, false, false, true},
	{false, false, false, true},  {false, false, false, true},
	{false, true,  false, true},  {false, true,  true,  true},
	{false, true,  true,  true},  {true,  true,  true,  true},
};

void calc_pitch(double hz, uint8_t& block, uint16_t& fnum) {
	const double phi = 7670453.0;
	block = 0; fnum = 0;
	if (hz <= 0.0) return;
	for (uint8_t b = 0; b < 8; ++b) {
		const double fn = (hz * 144.0 * static_cast<double>(1ULL << (21 - b))) / phi;
		if (fn < 2048.0 || b == 7) {
			block = b;
			fnum = (fn > 2047.0) ? 2047 : static_cast<uint16_t>(std::lround(fn));
			return;
		}
	}
}

/// One YM2612 voice channel with a shadow of what the chip already holds, so a
/// register is only written when it changes.
struct FmChannel {
	int index = 0;            // 0..5
	bool loaded = false;
	Voice voice;
	int tl_att = -1;          // volume attenuation currently applied to carriers
	int fnum_block = -1;
	bool keyed = false;
	double trigger_time = 0.0;
	double end_time = 1e30;   // one-shot sample runs out
	uint8_t sample = 0;
};

class Chip {
public:
	explicit Chip(VgmWriter& v) : vgm(v) {}

	void patch(FmChannel& c, const Voice& voice) {
		const FmPatch& p = voice.fm;
		const uint8_t port = c.index >= 3 ? 1 : 0, o = c.index % 3;
		vgm.write_ym2612(port, 0xB0 + o, ((p.feedback & 7) << 3) | (p.algorithm & 7));
		vgm.write_ym2612(port, 0xB4 + o, 0xC0);
		for (int op = 0; op < 4; ++op) {
			const uint8_t r = OP_OFFSET[op] + o;
			const FmOperator& f = p.op[op];
			vgm.write_ym2612(port, 0x30 + r, ((f.dt & 7) << 4) | (f.mul & 15));
			vgm.write_ym2612(port, 0x50 + r, ((f.ks & 3) << 6) | (f.ar & 31));
			vgm.write_ym2612(port, 0x60 + r, f.dr & 31);
			vgm.write_ym2612(port, 0x70 + r, f.sr & 31);
			vgm.write_ym2612(port, 0x80 + r, ((f.sl & 15) << 4) | (f.rr & 15));
			vgm.write_ym2612(port, 0x90 + r, f.ssg_eg & 15);
		}
		c.voice = voice;
		c.loaded = true;
		c.tl_att = -1; // TL has to be rewritten with the new patch levels
	}

	void level(FmChannel& c, int att) {
		if (att == c.tl_att) return;
		c.tl_att = att;
		const uint8_t port = c.index >= 3 ? 1 : 0, o = c.index % 3;
		const FmPatch& p = c.voice.fm;
		for (int op = 0; op < 4; ++op) {
			int tl = p.op[op].tl;
			if (IS_CARRIER[p.algorithm & 7][op]) tl += c.voice.gain_tl + att;
			if (tl > 127) tl = 127;
			vgm.write_ym2612(port, 0x40 + OP_OFFSET[op] + o, tl);
		}
	}

	void freq(FmChannel& c, double hz) {
		uint8_t block; uint16_t fnum;
		calc_pitch(hz, block, fnum);
		const int packed = (block << 11) | fnum;
		if (packed == c.fnum_block) return;
		c.fnum_block = packed;
		const uint8_t port = c.index >= 3 ? 1 : 0, o = c.index % 3;
		// high byte first: the chip latches it until the low byte arrives
		vgm.write_ym2612(port, 0xA4 + o, (block << 3) | ((fnum >> 8) & 7));
		vgm.write_ym2612(port, 0xA0 + o, fnum & 0xFF);
	}

	void key(FmChannel& c, bool on) {
		const uint8_t ch = c.index >= 3 ? c.index + 1 : c.index;
		vgm.write_ym2612(0, 0x28, (on ? 0xF0 : 0x00) | ch);
		c.keyed = on;
	}

	VgmWriter& vgm;
};

int attenuation(int vol) {
	if (vol >= 64) return 0;
	if (vol <= 0) return 127;
	const double tl = -26.6667 * std::log10(static_cast<double>(vol) / 64.0);
	return tl > 127.0 ? 127 : static_cast<int>(std::lround(tl));
}

} // namespace

Voice voice_for_sample(const ModSample& s, const SampleInfo& info) {
	Voice v;
	(void)s;
	if (info.looped && info.pitched) {
		v.fm = info.centroid > 0.12 ? FmPatchLibrary::get_distortion_guitar() : FmPatchLibrary::get_default_lead();
	} else if (!info.pitched) {
		if (info.centroid > 0.25) { v.fm = FmPatchLibrary::get_hihat(); v.fixed_hz = 3000; }
		else if (info.centroid > 0.10) { v.fm = FmPatchLibrary::get_snare(); v.fixed_hz = 400; }
		else { v.fm = FmPatchLibrary::get_default_percussion(); v.fixed_hz = 90; }
		v.pitch_follow = 0.5;
	} else {
		v.fm = FmPatchLibrary::get_slap_bass();
	}
	return v;
}

bool ConverterEngine::convert(const ModFile& mod, const std::string& out_vgm_path) {
	VgmWriter vgm;
	vgm.set_clocks(7670453, 3579545);
	Chip chip(vgm);

	SampleInfo info[32];
	Voice voices[32];
	for (int i = 1; i <= 31; ++i) {
		info[i] = analyze_sample(mod.samples[i]);
		voices[i] = voice_for_sample(mod.samples[i], info[i]);
	}

	const Song song = replay(mod);
	if (song.ticks.empty()) {
		std::cerr << "mod2vgm: nothing to play\n";
		return false;
	}

	FmChannel fm[4];
	for (int c = 0; c < 4; ++c) fm[c].index = c;
	Voice silent;
	for (int c = 0; c < 4; ++c) chip.patch(fm[c], silent);

	for (size_t i = 0; i < song.ticks.size(); ++i) {
		const Tick& tk = song.ticks[i];
		vgm.advance_to(tk.time);
		if (i == song.loop_tick) vgm.mark_loop_point();

		for (int c = 0; c < 4; ++c) {
			FmChannel& f = fm[c];
			const ChannelTick& ct = tk.ch[c];
			if (ct.sample == 0 || ct.period == 0) continue;
			const ModSample& smp = mod.samples[ct.sample];
			const SampleInfo& si = info[ct.sample];
			const Voice& v = voices[ct.sample];

			if (ct.trigger) {
				if (f.keyed) chip.key(f, false);
				chip.patch(f, v);
				f.sample = ct.sample;
				f.trigger_time = tk.time;
				const double rate = 3546894.6 / ct.period;
				f.end_time = (si.looped || smp.pcm_data.empty())
				    ? 1e30 : tk.time + 44100.0 * smp.pcm_data.size() / rate;
				f.fnum_block = -1;
			}
			if (!f.loaded || f.sample != ct.sample) continue;

			double hz;
			if (si.pitched) hz = sample_hz(smp, ct.period, si.cycle_len) * v.freq_mult;
			else hz = v.fixed_hz * std::pow(428.0 / ct.period, v.pitch_follow);

			const bool ended = tk.time >= f.end_time;
			chip.level(f, attenuation(ct.volume));
			chip.freq(f, hz);
			if (ct.trigger) chip.key(f, true);
			if (ended && f.keyed) chip.key(f, false);
		}
	}

	vgm.advance_to(song.end_time);
	for (int c = 0; c < 4; ++c) if (fm[c].keyed) chip.key(fm[c], false);
	return vgm.write_to_file(out_vgm_path);
}

} // namespace mod2vgm
