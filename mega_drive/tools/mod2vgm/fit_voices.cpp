// Fits a YM2612 (+ SN76489 noise) voice to every sample the PC modules use.
//
//   fit_voices out.inc a.mod b.mod ...
//
// For each distinct sample (by audio hash) the tool renders the original
// sample the way libxmp would play it at the pitch / length it is typically
// used with, then searches FM patch parameters whose cycle-accurate chip
// render has the closest log-spectrogram. The result is voice_table.inc,
// which the converter compiles in.

#include "mod_parser.h"
#include "replayer.h"
#include "sample_analysis.h"
#include "voice.h"
#include "voice_table.h"
#include "ym_player.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdio>
#include <map>
#include <set>
#include <mutex>
#include <random>
#include <thread>

using namespace mod2vgm;

namespace {

const double kRefScale = 32.0;     // one pcm unit -> 16-bit units (128 -> 4096)
const double kSampleRate = 44100.0;

// ---------------------------------------------------------------- usage stats
struct Usage {
	ModSample sample;
	SampleInfo info;
	std::vector<std::pair<int, double>> notes; // (period, hold seconds)
	int count = 0;
	bool fm_somewhere = false;
	std::string where;
};

// ---------------------------------------------------------------- references
std::vector<float> render_reference(const ModSample& s, int period, size_t n) {
	std::vector<float> out(n, 0.f);
	const double step = (3546894.6 / period) / kSampleRate;
	const long len = static_cast<long>(s.pcm_data.size());
	const long ls = s.repeat_offset_words * 2L, ll = s.repeat_length_words * 2L;
	const bool loop = ll > 2 && ls + ll <= len;
	double pos = 0;
	for (size_t i = 0; i < n; ++i) {
		if (loop) { while (pos >= ls + ll) pos -= ll; }
		else if (pos >= len - 1) break;
		const long i0 = static_cast<long>(pos);
		const long i1 = (loop && i0 + 1 >= ls + ll) ? ls : i0 + 1;
		const double f = pos - i0;
		const double a = s.pcm_data[i0], b = i1 < len ? s.pcm_data[i1] : 0;
		out[i] = static_cast<float>((a * (1 - f) + b * f) * kRefScale);
		pos += step;
	}
	return out;
}

// ---------------------------------------------------------------- chip render
struct PlayerSink : RegSink {
	explicit PlayerSink(YmPlayer& p) : y(p) {}
	void ym(int port, uint8_t reg, uint8_t val) override { y.write_ym(port, reg, val); }
	void psg(uint8_t val) override { y.write_psg(val); }
	YmPlayer& y;
};

std::vector<float> render_voice(const Voice& v, double hz, size_t n) {
	YmPlayer ym;
	PlayerSink sink(ym);
	FmDriver fm(sink, 0);
	NoiseDriver noise(sink);
	fm.load(v);
	fm.level(0);
	fm.pitch(hz);
	fm.key(true);
	if (v.noise.on) noise.start(v.noise, 0.0);
	std::vector<int16_t> pcm;
	pcm.reserve(n * 2);
	size_t done = 0;
	while (done < n) {
		if (v.noise.on) noise.update(static_cast<double>(done), 0);
		const size_t chunk = std::min<size_t>(735, n - done);
		ym.run(static_cast<uint32_t>(chunk), pcm);
		done += chunk;
	}
	std::vector<float> out(n);
	for (size_t i = 0; i < n; ++i) out[i] = 0.5f * (pcm[2 * i] + pcm[2 * i + 1]);
	return out;
}

// ---------------------------------------------------------------- loss
using cplx = std::complex<float>;

void fft(std::vector<cplx>& a) {
	const size_t n = a.size();
	for (size_t i = 1, j = 0; i < n; ++i) {
		size_t bit = n >> 1;
		for (; j & bit; bit >>= 1) j ^= bit;
		j ^= bit;
		if (i < j) std::swap(a[i], a[j]);
	}
	for (size_t len = 2; len <= n; len <<= 1) {
		const float ang = -2.0f * static_cast<float>(M_PI) / len;
		const cplx wl(std::cos(ang), std::sin(ang));
		for (size_t i = 0; i < n; i += len) {
			cplx w(1);
			for (size_t k = 0; k < len / 2; ++k) {
				const cplx u = a[i + k], t = a[i + k + len / 2] * w;
				a[i + k] = u + t; a[i + k + len / 2] = u - t;
				w *= wl;
			}
		}
	}
}

// magnitude^2 (amplitude units) of Hann-windowed frames
std::vector<std::vector<float>> stft_power(const std::vector<float>& x, size_t n, size_t hop) {
	std::vector<std::vector<float>> frames;
	std::vector<float> win(n);
	double wsum = 0;
	for (size_t i = 0; i < n; ++i) { win[i] = 0.5f - 0.5f * std::cos(2 * M_PI * i / n); wsum += win[i]; }
	std::vector<cplx> buf(n);
	for (size_t s = 0; s + n <= x.size(); s += hop) {
		for (size_t i = 0; i < n; ++i) buf[i] = x[s + i] * win[i];
		fft(buf);
		std::vector<float> p(n / 2);
		for (size_t k = 0; k < n / 2; ++k) p[k] = static_cast<float>(std::norm(buf[k]) * 4.0 / (wsum * wsum));
		frames.push_back(std::move(p));
	}
	return frames;
}

struct Target {
	std::vector<float> wave;
	std::vector<std::vector<float>> pa; // 1024 frames, hop 256
	std::vector<std::vector<float>> pb; // 4096 frames, hop 2048
	std::vector<float> weight_a;
	std::vector<int> edges;             // band edges (bins of the 1024 analysis)
};

const float kFloor = 4.0f; // amplitude^2 floor (~ -60 dB below a 4096 peak)

std::vector<int> band_edges() {
	std::vector<int> e;
	const double lo = 3, hi = 372;
	int last = -1;
	for (int i = 0; i <= 30; ++i) {
		int b = static_cast<int>(std::lround(lo * std::pow(hi / lo, i / 30.0)));
		if (b <= last) b = last + 1;
		e.push_back(b); last = b;
	}
	return e;
}

void analyse(Target& t) {
	t.edges = band_edges();
	t.pa = stft_power(t.wave, 1024, 256);
	t.pb = stft_power(t.wave, 4096, 2048);
	float maxe = 1e-9f;
	std::vector<float> fe(t.pa.size(), 0.f);
	for (size_t f = 0; f < t.pa.size(); ++f) {
		for (int k = t.edges.front(); k < t.edges.back(); ++k) fe[f] += t.pa[f][k];
		maxe = std::max(maxe, fe[f]);
	}
	t.weight_a.resize(t.pa.size());
	for (size_t f = 0; f < t.pa.size(); ++f) {
		const double db = 10 * std::log10(fe[f] / maxe + 1e-9);
		t.weight_a[f] = static_cast<float>(0.25 + 0.75 * std::max(0.0, std::min(1.0, (db + 40) / 40)));
	}
}

double loss(const Target& ref, const std::vector<float>& cand) {
	std::vector<float> c = cand;
	double mean = 0;
	for (float v : c) mean += v;
	mean /= c.size();
	for (float& v : c) v -= static_cast<float>(mean);
	const auto ca = stft_power(c, 1024, 256);
	const auto cb = stft_power(c, 4096, 2048);
	double la = 0, wa = 0;
	for (size_t f = 0; f < ref.pa.size() && f < ca.size(); ++f) {
		double acc = 0;
		for (size_t b = 0; b + 1 < ref.edges.size(); ++b) {
			double er = 0, ec = 0;
			for (int k = ref.edges[b]; k < ref.edges[b + 1]; ++k) { er += ref.pa[f][k]; ec += ca[f][k]; }
			acc += std::fabs(10 * std::log10((ec + kFloor) / (er + kFloor)));
		}
		double etot_r = 0, etot_c = 0;
		for (int k = ref.edges.front(); k < ref.edges.back(); ++k) { etot_r += ref.pa[f][k]; etot_c += ca[f][k]; }
		// overall level of the frame counts on top of the band shape: a patch
		// that is 5 dB too loud everywhere would otherwise trade it for detail
		acc += 1.5 * std::fabs(10 * std::log10((etot_c + kFloor) / (etot_r + kFloor))) * (ref.edges.size() - 1);
		la += ref.weight_a[f] * acc / (ref.edges.size() - 1);
		wa += ref.weight_a[f];
	}
	la /= std::max(wa, 1e-9);
	double lb = 0; size_t nb = 0;
	for (size_t f = 0; f < ref.pb.size() && f < cb.size(); ++f) {
		for (int k = 3; k < 140; ++k) {
			const double er = std::max(ref.pb[f][k], ref.pb[f][k - 1] * 0.0f);
			const double dr = 10 * std::log10(er + kFloor), dc = 10 * std::log10(cb[f][k] + kFloor);
			lb += std::fabs(dr - dc); ++nb;
		}
	}
	lb /= std::max<size_t>(nb, 1);
	return la + 0.5 * lb;
}

// ---------------------------------------------------------------- search
struct Range { int lo, hi; };

struct Cand {
	Voice v;
	double loss = 1e9;
};

const Range kMul{0, 15}, kDt{0, 7}, kTl{0, 127}, kKs{0, 3}, kAr{24, 31}, kDr{0, 31}, kSr{0, 31}, kRr{0, 15}, kSl{0, 15};

int clampi(int v, Range r) { return std::max(r.lo, std::min(r.hi, v)); }

struct Rng {
	std::mt19937 g;
	explicit Rng(unsigned seed) : g(seed) {}
	int i(int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(g); }
	double u() { return std::uniform_real_distribution<double>(0, 1)(g); }
	double n() { return std::normal_distribution<double>(0, 1)(g); }
};

Voice random_voice(Rng& r, bool pitched, double hz0) {
	Voice v;
	v.pitched = pitched;
	v.fixed_hz = hz0;
	v.fm.algorithm = r.i(0, 7);
	v.fm.feedback = r.i(0, 7);
	static const int muls[] = {1, 1, 1, 1, 2, 2, 3, 4, 0, 5, 6, 7, 8, 10, 12, 15};
	for (int op = 0; op < 4; ++op) {
		FmOperator& o = v.fm.op[op];
		const bool car = is_carrier(v.fm.algorithm, op);
		o.mul = muls[r.i(0, 15)];
		if (car && r.u() < 0.6) o.mul = 1;
		o.dt = r.u() < 0.7 ? 0 : r.i(0, 7);
		o.tl = car ? r.i(0, 40) : r.i(10, 80);
		o.ks = r.i(0, 1);
		o.ar = r.i(24, 31);
		o.dr = r.i(0, 31);
		o.sr = r.i(0, 12);
		o.rr = r.i(4, 15);
		o.sl = r.i(0, 15);
	}
	return v;
}

void mutate(Voice& v, Rng& r, double sigma, bool allow_noise) {
	const int n = r.i(1, 4);
	for (int m = 0; m < n; ++m) {
		const int what = r.i(0, 99);
		if (what < 3) { v.fm.algorithm = r.i(0, 7); continue; }
		if (what < 10) { v.fm.feedback = clampi(v.fm.feedback + (r.u() < 0.5 ? -1 : 1) * r.i(1, 2), {0, 7}); continue; }
		if (what < 14 && !v.pitched) { v.fixed_hz = std::max(30.0, std::min(8000.0, v.fixed_hz * std::pow(2.0, r.n() * 0.5 * sigma))); continue; }
		if (what < 20 && allow_noise) {
			NoiseLayer& nl = v.noise;
			const int w = r.i(0, 4);
			if (w == 0) nl.on = !nl.on;
			else if (w == 1) nl.rate = r.i(0, 3);
			else if (w == 2) nl.att0 = clampi(nl.att0 + r.i(-3, 3), {0, 14});
			else if (w == 3) nl.step = clampi(nl.step + r.i(-2, 2), {1, 12});
			else nl.white = !nl.white;
			continue;
		}
		FmOperator& o = v.fm.op[r.i(0, 3)];
		const int p = r.i(0, 8);
		const int d = std::max(1, static_cast<int>(std::lround(std::fabs(r.n()) * sigma * 8)));
		const int sgn = r.u() < 0.5 ? -1 : 1;
		switch (p) {
		case 0: o.mul = clampi(r.u() < 0.5 ? o.mul + sgn : r.i(0, 15), kMul); break;
		case 1: o.dt = clampi(r.i(0, 7), kDt); break;
		case 2: o.tl = clampi(o.tl + sgn * d * 2, kTl); break;
		case 3: o.ks = clampi(o.ks + sgn, kKs); break;
		case 4: o.ar = clampi(o.ar + sgn * d, kAr); break;
		case 5: o.dr = clampi(o.dr + sgn * d, kDr); break;
		case 6: o.sr = clampi(o.sr + sgn * d, kSr); break;
		case 7: o.rr = clampi(o.rr + sgn * d, kRr); break;
		default: o.sl = clampi(o.sl + sgn * (d / 2 + 1), kSl); break;
		}
	}
}

struct Job {
	Usage* u = nullptr;
	int period = 428;
	double secs = 0.5;
	double hz = 0;
};

Cand fit(const Job& job, unsigned seed, int budget) {
	Rng rng(seed);
	const size_t n = static_cast<size_t>(job.secs * kSampleRate);
	Target tgt;
	tgt.wave = render_reference(job.u->sample, job.period, n);
	analyse(tgt);

	const bool pitched = job.u->info.pitched;
	const bool allow_noise = false; // percussion with noise went to the DAC
	const double hz0 = pitched ? 0.0 : job.hz;

	auto evaluate = [&](Cand& c) {
		const double hz = c.v.pitched ? job.hz : c.v.fixed_hz;
		c.loss = loss(tgt, render_voice(c.v, hz, n));
	};

	std::vector<Cand> pool;
	{
		const FmPatch lib[] = {FmPatchLibrary::get_default_lead(), FmPatchLibrary::get_distortion_guitar(),
		                       FmPatchLibrary::get_clean_guitar(), FmPatchLibrary::get_default_bass(),
		                       FmPatchLibrary::get_slap_bass(), FmPatchLibrary::get_default_pad(),
		                       FmPatchLibrary::get_default_percussion(), FmPatchLibrary::get_snare(),
		                       FmPatchLibrary::get_hihat()};
		for (const FmPatch& p : lib) {
			Cand c;
			c.v.fm = p;
			for (int op = 0; op < 4; ++op) if (c.v.fm.op[op].ar < 24) c.v.fm.op[op].ar = 24;
			c.v.pitched = pitched; c.v.fixed_hz = hz0 > 0 ? hz0 : 200.0;
			evaluate(c);
			pool.push_back(c);
		}
	}
	const int seeds = budget / 12;
	for (int i = 0; i < seeds; ++i) {
		Cand c;
		c.v = random_voice(rng, pitched, hz0 > 0 ? hz0 * std::pow(2.0, rng.n() * 0.5) : 200.0);
		evaluate(c);
		pool.push_back(c);
	}
	std::sort(pool.begin(), pool.end(), [](const Cand& a, const Cand& b) { return a.loss < b.loss; });
	pool.resize(std::min<size_t>(pool.size(), 4));

	const int refine = budget / 6;
	for (Cand& c : pool) {
		for (int it = 0; it < refine; ++it) {
			const double sigma = 1.0 * (1.0 - 0.8 * it / refine);
			Cand t = c;
			mutate(t.v, rng, sigma, allow_noise);
			evaluate(t);
			if (t.loss < c.loss) c = t;
		}
	}
	return *std::min_element(pool.begin(), pool.end(), [](const Cand& a, const Cand& b) { return a.loss < b.loss; });
}

double weighted_median(std::vector<std::pair<double, double>> v) { // (value, weight)
	std::sort(v.begin(), v.end());
	double tot = 0;
	for (auto& p : v) tot += p.second;
	double acc = 0;
	for (auto& p : v) { acc += p.second; if (acc >= tot / 2) return p.first; }
	return v.empty() ? 0 : v.back().first;
}

} // namespace

int main(int argc, char** argv) {
	if (argc < 3) { std::fprintf(stderr, "usage: fit_voices out.inc [-b budget] [-j threads] [-k keyhex] a.mod ...\n"); return 1; }
	int budget = 600, threads = static_cast<int>(std::thread::hardware_concurrency());
	uint32_t only_key = 0;
	std::string dump_dir;
	std::string out_path = argv[1];
	std::vector<std::string> mods;
	for (int i = 2; i < argc; ++i) {
		std::string a = argv[i];
		if (a == "-b") budget = atoi(argv[++i]);
		else if (a == "-j") threads = atoi(argv[++i]);
		else if (a == "-d") dump_dir = argv[++i];
		else if (a == "-k") only_key = static_cast<uint32_t>(strtoul(argv[++i], nullptr, 16));
		else mods.push_back(a);
	}

	std::map<uint32_t, Usage> uses;
	for (const std::string& path : mods) {
		ModFile mod;
		if (!mod.load_from_file(path)) continue;
		const Song song = replay(mod);
		SampleInfo infos[32];
		for (int s = 1; s <= 31; ++s) infos[s] = analyze_sample(mod.samples[s]);
		std::set<int> periods[32];
		for (const Tick& tk : song.ticks)
			for (int c = 0; c < 4; ++c)
				if (tk.ch[c].trigger && tk.ch[c].sample) periods[tk.ch[c].sample].insert(tk.ch[c].period);
		for (size_t i = 0; i < song.ticks.size(); ++i) {
			for (int c = 0; c < 4; ++c) {
				const ChannelTick& ct = song.ticks[i].ch[c];
				if (!ct.trigger || !ct.sample || !ct.period) continue;
				const SampleInfo& si = infos[ct.sample];
				Usage& u = uses[si.key];
				if (!is_percussion(si, mod.samples[ct.sample].pcm_data.size(), periods[ct.sample].size())) u.fm_somewhere = true;
				if (u.count == 0) { u.sample = mod.samples[ct.sample]; u.info = si; u.where = path.substr(path.rfind('/') + 1) + "#" + std::to_string(ct.sample); }
				double hold = 0.6;
				for (size_t j = i + 1; j < song.ticks.size(); ++j) {
					if (song.ticks[j].ch[c].trigger) { hold = (song.ticks[j].time - song.ticks[i].time) / kSampleRate; break; }
					if (j - i > 400) break;
				}
				u.notes.push_back({ct.period, hold});
				++u.count;
			}
		}
	}
	std::fprintf(stderr, "%zu distinct samples used\n", uses.size());

	std::vector<Job> jobs;
	for (auto& kv : uses) {
		if (only_key && kv.first != only_key) continue;
		if (!kv.second.fm_somewhere) continue; // percussion only: played from the DAC
		Usage& u = kv.second;
		std::vector<std::pair<double, double>> per, hold;
		for (auto& n : u.notes) { per.push_back({double(n.first), 1.0}); hold.push_back({n.second, 1.0}); }
		Job j;
		j.u = &u;
		j.period = static_cast<int>(weighted_median(per));
		std::sort(hold.begin(), hold.end());
		double h = hold.empty() ? 0.4 : hold[std::min(hold.size() - 1, hold.size() * 3 / 4)].first;
		const double rate = 3546894.6 / j.period;
		if (!u.info.looped && !u.sample.pcm_data.empty()) h = std::min(h, u.sample.pcm_data.size() / rate);
		j.secs = std::max(0.12, std::min(0.6, h));
		j.hz = u.info.pitched ? sample_hz(u.sample, j.period, u.info.cycle_len) : 0.0;
		if (!u.info.pitched) {
			// unpitched: start the FM at the sample's own dominant rate of change
			j.hz = 100.0 * std::pow(2.0, 6.0 * std::min(1.0, u.info.centroid * 3.0));
		}
		jobs.push_back(j);
	}
	std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) { return a.u->count > b.u->count; });

	std::vector<Cand> results(jobs.size());
	std::atomic<size_t> next{0};
	std::mutex io;
	auto worker = [&]() {
		for (;;) {
			const size_t i = next++;
			if (i >= jobs.size()) return;
			results[i] = fit(jobs[i], 1234u + static_cast<unsigned>(i), budget);
			std::lock_guard<std::mutex> lk(io);
			std::fprintf(stderr, "[%3zu/%zu] %08x %-34s uses %4d period %3d hold %.2fs %s loss %.2f%s\n",
			             i + 1, jobs.size(), jobs[i].u->info.key, jobs[i].u->where.c_str(), jobs[i].u->count,
			             jobs[i].period, jobs[i].secs, jobs[i].u->info.pitched ? "pitched  " : "unpitched",
			             results[i].loss, results[i].v.noise.on ? " +noise" : "");
		}
	};
	std::vector<std::thread> pool;
	for (int t = 0; t < threads; ++t) pool.emplace_back(worker);
	for (auto& t : pool) t.join();

	if (!dump_dir.empty()) {
		for (size_t i = 0; i < jobs.size(); ++i) {
			const size_t n = static_cast<size_t>(jobs[i].secs * kSampleRate);
			const Voice& v = results[i].v;
			const auto ref = render_reference(jobs[i].u->sample, jobs[i].period, n);
			const auto fitted = render_voice(v, v.pitched ? jobs[i].hz : v.fixed_hz, n);
			char name[512];
			for (int w = 0; w < 2; ++w) {
				std::snprintf(name, sizeof name, "%s/%08x_%s.raw", dump_dir.c_str(), jobs[i].u->info.key, w ? "fit" : "ref");
				FILE* o = std::fopen(name, "wb");
				const auto& x = w ? fitted : ref;
				std::fwrite(x.data(), sizeof(float), x.size(), o);
				std::fclose(o);
			}
		}
	}
	FILE* f = std::fopen(out_path.c_str(), "w");
	std::fprintf(f, "// generated by fit_voices (budget %d): key, pitched, fixed_hz, follow, alg, fb, ops{dt mul tl ks ar dr sr rr sl}, noise{on rate white att0 step}, loss\n", budget);
	for (size_t i = 0; i < jobs.size(); ++i) {
		const Voice& v = results[i].v;
		double fixed = v.fixed_hz;
		if (!v.pitched) fixed = v.fixed_hz / std::pow(428.0 / jobs[i].period, v.pitch_follow);
		std::fprintf(f, "\t{0x%08xu, %d, %.2ff, %.2ff, %d, %d, {", jobs[i].u->info.key, v.pitched, fixed, v.pitch_follow, v.fm.algorithm, v.fm.feedback);
		for (int op = 0; op < 4; ++op) {
			const FmOperator& o = v.fm.op[op];
			std::fprintf(f, "{%d,%d,%d,%d,%d,%d,%d,%d,%d}%s", o.dt, o.mul, o.tl, o.ks, o.ar, o.dr, o.sr, o.rr, o.sl, op < 3 ? "," : "");
		}
		std::fprintf(f, "}, %d, %d, %d, %d, %d, %.2ff}, // %s\n", v.noise.on, v.noise.rate, v.noise.white, v.noise.att0, v.noise.step,
		             results[i].loss, jobs[i].u->where.c_str());
	}
	std::fclose(f);
	return 0;
}
