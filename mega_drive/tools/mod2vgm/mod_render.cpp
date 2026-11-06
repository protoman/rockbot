// Reference renderer: plays a module through the replayer with the original
// sample data (no FM). Used to check the replayer against libxmp and to give
// the voice fitter its target waveforms.
#include "mod_parser.h"
#include "replayer.h"
#include "sample_analysis.h"
#include <string>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

using namespace mod2vgm;

int main(int argc, char** argv) {
	if (argc < 3) { std::cerr << "usage: mod_render in.mod out.wav [secs]\n"; return 1; }
	const double max_secs = argc > 3 ? atof(argv[3]) : 1e9;
	ModFile mod;
	if (!mod.load_from_file(argv[1])) return 1;
	const Song song = replay(mod);
	const char* only = std::getenv("MR_ONLY"); // "unpitched": render just the percussive samples
	bool keep[32];
	for (int i = 0; i < 32; ++i) keep[i] = true;
	if (only) for (int i = 1; i <= 31; ++i) keep[i] = (analyze_sample(mod.samples[i]).pitched == (std::string(only) == "pitched"));
	std::vector<int16_t> out;
	struct Voice { const ModSample* s = nullptr; double pos = 0; bool on = false; } v[4];
	for (size_t i = 0; i < song.ticks.size(); ++i) {
		const Tick& tk = song.ticks[i];
		const double t1 = (i + 1 < song.ticks.size()) ? song.ticks[i + 1].time : song.end_time;
		const size_t n0 = static_cast<size_t>(tk.time), n1 = static_cast<size_t>(t1);
		for (int c = 0; c < 4; ++c) {
			const ChannelTick& ct = tk.ch[c];
			if (ct.trigger && ct.sample && keep[ct.sample]) { v[c].s = &mod.samples[ct.sample]; v[c].pos = 0; v[c].on = true; }
		}
		for (size_t n = n0; n < n1; ++n) {
			double mix = 0;
			for (int c = 0; c < 4; ++c) {
				const ChannelTick& ct = tk.ch[c];
				Voice& vc = v[c];
				if (!vc.on || !ct.period) continue;
				const ModSample& s = *vc.s;
				const double step = (3546894.6 / ct.period) / 44100.0;
				const long len = static_cast<long>(s.pcm_data.size());
				const long ls = s.repeat_offset_words * 2L, ll = s.repeat_length_words * 2L;
				const bool loop = ll > 2 && ls + ll <= len;
				if (loop) { while (vc.pos >= ls + ll) vc.pos -= ll; }
				else if (vc.pos >= len - 1) { vc.on = false; continue; }
				const long i0 = static_cast<long>(vc.pos);
				const long i1 = (loop && i0 + 1 >= ls + ll) ? ls : i0 + 1;
				const double f = vc.pos - i0;
				const double a = s.pcm_data[i0], b = (i1 < len) ? s.pcm_data[i1] : 0;
				mix += (a * (1 - f) + b * f) * ct.volume / 64.0;
				vc.pos += step;
			}
			out.push_back(static_cast<int16_t>(std::max(-32768.0, std::min(32767.0, mix * 32.0))));
			if (out.size() >= max_secs * 44100) goto done;
		}
	}
done:
	std::ofstream w(argv[2], std::ios::binary);
	uint32_t ds = out.size() * 4, fs = 36 + ds, sr = 44100, br = 44100 * 4, fmt = 16;
	uint16_t t = 1, ch = 2, ba = 4, bits = 16;
	w.write("RIFF", 4); w.write((char*)&fs, 4); w.write("WAVEfmt ", 8); w.write((char*)&fmt, 4);
	w.write((char*)&t, 2); w.write((char*)&ch, 2); w.write((char*)&sr, 4); w.write((char*)&br, 4);
	w.write((char*)&ba, 2); w.write((char*)&bits, 2); w.write("data", 4); w.write((char*)&ds, 4);
	for (int16_t s : out) { w.write((char*)&s, 2); w.write((char*)&s, 2); }
	std::cout << out.size() << " samples\n";
}
