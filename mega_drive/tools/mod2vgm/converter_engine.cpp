#include "converter_engine.h"
#include "replayer.h"
#include "voice_table.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

namespace mod2vgm {

namespace {

const VoiceRow kVoiceTable[] = {
#include "voice_table.inc"
	{0, 0, 0, 0, 0, 0, {{0}}, 0, 0, 0, 0, 0, 0},
};

struct VgmSink : RegSink {
	explicit VgmSink(VgmWriter& v) : vgm(v) {}
	void ym(int port, uint8_t reg, uint8_t val) override { vgm.write_ym2612(port, reg, val); }
	void psg(uint8_t val) override { vgm.write_psg(val); }
	VgmWriter& vgm;
};

}

bool fitted_voice(uint32_t key, Voice& out) {
	for (const VoiceRow& r : kVoiceTable) {
		if (r.key != key || key == 0) continue;
		out = Voice();
		out.pitched = r.pitched;
		out.fixed_hz = r.fixed_hz;
		out.pitch_follow = r.pitch_follow;
		out.fm.algorithm = r.alg;
		out.fm.feedback = r.fb;
		for (int i = 0; i < 4; ++i) {
			FmOperator& o = out.fm.op[i];
			o.dt = r.op[i][0]; o.mul = r.op[i][1]; o.tl = r.op[i][2]; o.ks = r.op[i][3];
			o.ar = r.op[i][4]; o.dr = r.op[i][5]; o.sr = r.op[i][6]; o.rr = r.op[i][7]; o.sl = r.op[i][8];
		}
		out.noise.on = r.noise_on;
		out.noise.rate = r.noise_rate;
		out.noise.white = r.noise_white;
		out.noise.att0 = r.noise_att0;
		out.noise.step = r.noise_step;
		return true;
	}
	return false;
}

Voice voice_for_sample(const ModSample& s, const SampleInfo& info) {
	Voice v;
	if (fitted_voice(info.key, v) && v.pitched) return v;
	v = Voice();
	(void)s;
	v.pitched = info.pitched;
	if (info.looped && info.pitched) {
		v.fm = info.centroid > 0.12 ? FmPatchLibrary::get_distortion_guitar() : FmPatchLibrary::get_default_lead();
	} else if (!info.pitched) {
		if (info.centroid > 0.25) { v.fm = FmPatchLibrary::get_hihat(); v.fixed_hz = 3000; }
		else if (info.centroid > 0.10) { v.fm = FmPatchLibrary::get_snare(); v.fixed_hz = 400; }
		else { v.fm = FmPatchLibrary::get_default_percussion(); v.fixed_hz = 90; }
	} else {
		v.fm = FmPatchLibrary::get_slap_bass();
	}
	return v;
}

namespace {

const double kPcmRate = 13300.0;   // XGM2 full rate
const double kDacGain = 0.54;      // sample full scale (127) -> same level as the fitted FM voices
const double kFrame = 735.0;

double quantise(double t) { return std::floor(t / kFrame + 0.5) * kFrame; }

/// One PCM stream: a run of overlapping percussion hits mixed into one buffer.
struct PcmRun {
	double start = 0;          // quantised time
	std::vector<double> mix;   // signed, in 8-bit units, at kPcmRate
	uint32_t offset = 0, len = 0;
};

void add_hit(PcmRun& run, double t, const ModSample& s, int period, int vol, double cut_time) {
	const double step = (3546894.6 / period) / kPcmRate;   // source samples per output byte
	size_t n = static_cast<size_t>(s.pcm_data.size() / step);
	if (cut_time > 0) n = std::min(n, static_cast<size_t>(std::max(0.0, (cut_time - t) * kPcmRate / 44100.0)));
	const size_t base = static_cast<size_t>(std::floor((t - run.start) * kPcmRate / 44100.0 + 0.5));
	if (run.mix.size() < base + n) run.mix.resize(base + n, 0.0);
	const double g = vol / 64.0 * kDacGain;
	for (size_t i = 0; i < n; ++i) {
		const double p = i * step;
		const size_t i0 = static_cast<size_t>(p);
		const double f = p - i0;
		const double a = s.pcm_data[i0], b = (i0 + 1 < s.pcm_data.size()) ? s.pcm_data[i0 + 1] : 0.0;
		run.mix[base + i] += (a * (1 - f) + b * f) * g;
	}
}

} // namespace

bool ConverterEngine::convert(const ModFile& mod, const std::string& out_vgm_path) {
	VgmWriter vgm;
	vgm.set_clocks(7670453, 3579545);
	VgmSink sink(vgm);

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

	// ---- percussion = one-shots this song plays at few pitches: real samples on the DAC
	{
		std::set<int> periods[32];
		for (const Tick& tk : song.ticks)
			for (int c = 0; c < 4; ++c)
				if (tk.ch[c].trigger && tk.ch[c].sample) periods[tk.ch[c].sample].insert(tk.ch[c].period);
		for (int i = 1; i <= 31; ++i)
			voices[i].dac = !periods[i].empty() &&
			                is_percussion(info[i], mod.samples[i].pcm_data.size(), periods[i].size());
	}

	// ---- pass 1: percussion hits -> PCM runs -> bank
	std::vector<PcmRun> runs;
	std::vector<uint8_t> bank;
	{
		std::map<std::vector<uint8_t>, uint32_t> seen;
		PcmRun cur; bool open = false; double cur_end = 0;
		auto close = [&]() {
			if (!open) return;
			std::vector<uint8_t> bytes;
			bytes.reserve(cur.mix.size() + 1);
			for (double x : cur.mix) {
				long v = std::lround(x);
				bytes.push_back(static_cast<uint8_t>(128 + std::max(-127L, std::min(127L, v))));
			}
			bytes.push_back(0x80);
			auto it = seen.find(bytes);
			if (it == seen.end()) {
				it = seen.emplace(bytes, static_cast<uint32_t>(bank.size())).first;
				bank.insert(bank.end(), bytes.begin(), bytes.end());
			}
			cur.offset = it->second;
			cur.len = static_cast<uint32_t>(bytes.size());
			cur.mix.clear();
			runs.push_back(cur);
			open = false;
		};
		const char* mode_env = std::getenv("M2V_PCM_MODE");
		const int mode = mode_env ? std::atoi(mode_env) : 2;   // 0 = mix overlaps, 2 = mix simultaneous hits only
		const char* vq_env = std::getenv("M2V_VOLQ");
		const int vq = vq_env ? std::atoi(vq_env) : 8;
		for (size_t i = 0; i < song.ticks.size(); ++i) {
			const Tick& tk = song.ticks[i];
			for (int c = 0; c < 4; ++c) {
				const ChannelTick& ct = tk.ch[c];
				if (!ct.trigger || !ct.sample || !voices[ct.sample].dac || !ct.period) continue;
				const double t = quantise(tk.time);
				double cut = 0;
				if (mode == 0) {
					for (size_t j = i + 1; j < song.ticks.size(); ++j) {
						const ChannelTick& nx = song.ticks[j].ch[c];
						if (nx.trigger || nx.volume == 0) { cut = quantise(song.ticks[j].time); break; }
					}
				}
				const bool joins = mode == 0 ? (open && t < cur_end) : (open && t == cur.start);
				if (!joins) {
					close();
					cur = PcmRun();
					cur.start = t;
					open = true;
					cur_end = t;
				}
				int vol = ct.volume;
				if (vq > 1) vol = std::max(vq, ((vol + vq / 2) / vq) * vq);
				add_hit(cur, t, mod.samples[ct.sample], ct.period, std::min(64, vol), cut);
				cur_end = std::max(cur_end, cur.start + cur.mix.size() * 44100.0 / kPcmRate);
			}
		}
		close();
	}
	if (!bank.empty()) vgm.set_pcm_bank(bank);
	std::cerr << "mod2vgm: " << runs.size() << " PCM runs, bank " << bank.size() << " bytes\n";

	FmDriver fm[4] = {FmDriver(sink, 0), FmDriver(sink, 1), FmDriver(sink, 2), FmDriver(sink, 3)};
	NoiseDriver noise(sink);
	Voice silent;
	for (int c = 0; c < 4; ++c) fm[c].load(silent);

	uint8_t playing[4] = {0, 0, 0, 0};
	double end_time[4] = {1e30, 1e30, 1e30, 1e30};
	int noise_owner = -1;
	size_t next_run = 0;

	for (size_t i = 0; i < song.ticks.size(); ++i) {
		const Tick& tk = song.ticks[i];
		const double qt = quantise(tk.time);
		vgm.advance_to(qt);
		if (i == song.loop_tick) vgm.mark_loop_point();
		while (next_run < runs.size() && runs[next_run].start <= qt) {
			vgm.play_pcm(runs[next_run].offset, runs[next_run].len, kPcmRate);
			++next_run;
		}

		for (int c = 0; c < 4; ++c) {
			const ChannelTick& ct = tk.ch[c];
			if (ct.sample == 0 || ct.period == 0) continue;
			const ModSample& smp = mod.samples[ct.sample];
			const SampleInfo& si = info[ct.sample];
			const Voice& v = voices[ct.sample];

			if (ct.trigger) {
				if (fm[c].keyed()) fm[c].key(false);
				if (v.dac) { playing[c] = 0; continue; }
				fm[c].load(v);
				playing[c] = ct.sample;
				const double rate = 3546894.6 / ct.period;
				end_time[c] = (si.looped || smp.pcm_data.empty()) ? 1e30 : tk.time + 44100.0 * smp.pcm_data.size() / rate;
				if (v.noise.on) { noise.start(v.noise, qt); noise_owner = c; }
				else if (noise_owner == c) { noise.stop(); noise_owner = -1; }
			}
			if (playing[c] != ct.sample) continue;

			double hz;
			if (v.pitched) hz = sample_hz(smp, ct.period, si.cycle_len);
			else hz = v.fixed_hz * std::pow(428.0 / ct.period, v.pitch_follow);

			const bool ended = tk.time >= end_time[c];
			fm[c].level(attenuation_tl(ct.volume));
			fm[c].pitch(hz);
			if (ct.trigger) fm[c].key(true);
			if (ended && fm[c].keyed()) fm[c].key(false);
			if (noise_owner == c) noise.update(qt, psg_steps_for_volume(ended ? 0 : ct.volume));
		}
	}

	vgm.advance_to(quantise(song.end_time));
	for (int c = 0; c < 4; ++c) if (fm[c].keyed()) fm[c].key(false);
	noise.stop();
	return vgm.write_to_file(out_vgm_path);
}

} // namespace mod2vgm
