#include "sample_analysis.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace mod2vgm {

namespace {

double autocorr(const std::vector<double>& x, size_t begin, size_t n, double lag) {
	// normalised correlation of x[begin..begin+n) with itself shifted by `lag`
	// (linear interpolation, so fractional lags work)
	double num = 0, d0 = 0, d1 = 0;
	for (size_t i = 0; i < n; ++i) {
		const double p = begin + i + lag;
		const size_t i0 = static_cast<size_t>(p);
		if (i0 + 1 >= x.size()) break;
		const double f = p - i0;
		const double b = x[i0] * (1 - f) + x[i0 + 1] * f;
		const double a = x[begin + i];
		num += a * b; d0 += a * a; d1 += b * b;
	}
	return (d0 > 0 && d1 > 0) ? num / std::sqrt(d0 * d1) : 0.0;
}

} // namespace

SampleInfo analyze_sample(const ModSample& s) {
	SampleInfo info;
	const size_t len = s.pcm_data.size();
	uint32_t h = 2166136261u;
	for (size_t i = 0; i < len; ++i) { h ^= static_cast<uint8_t>(s.pcm_data[i]); h *= 16777619u; }
	for (uint16_t v : {s.repeat_offset_words, s.repeat_length_words}) { h ^= v; h *= 16777619u; }
	info.key = h;
	if (len < 4) return info;

	std::vector<double> x(len);
	double sq = 0, pk = 0;
	for (size_t i = 0; i < len; ++i) {
		x[i] = s.pcm_data[i] / 128.0;
		sq += x[i] * x[i];
		pk = std::max(pk, std::fabs(x[i]));
	}
	info.rms = std::sqrt(sq / len);
	info.peak = pk;

	// first-difference energy ratio = brightness (centroid proxy)
	double dd = 0;
	for (size_t i = 1; i < len; ++i) dd += (x[i] - x[i - 1]) * (x[i] - x[i - 1]);
	info.centroid = std::min(0.5, std::sqrt(dd / std::max(sq, 1e-9)) / (2.0 * M_PI));

	const size_t loop_start = static_cast<size_t>(s.repeat_offset_words) * 2;
	const size_t loop_len = static_cast<size_t>(s.repeat_length_words) * 2;
	info.looped = loop_len > 2 && loop_start + loop_len <= len;

	if (info.looped && loop_len >= 8) {
		// A looped waveform holds k whole cycles: the smallest cycle that still
		// repeats exactly gives the played pitch rate * k / loop_len.
		std::vector<double> r(x.begin() + loop_start, x.begin() + loop_start + loop_len);
		std::vector<double> rr(r);
		rr.insert(rr.end(), r.begin(), r.end());
		double best_conf = 0; double best_len = static_cast<double>(loop_len);
		for (size_t k = 1; k <= loop_len / 4; ++k) {
			if (loop_len % k) continue;
			const double L = static_cast<double>(loop_len) / k;
			const double c = autocorr(rr, 0, loop_len, L);
			if (c >= 0.97) { best_conf = c; best_len = L; }
		}
		info.cycle_len = best_len;
		info.confidence = best_conf > 0 ? best_conf : 1.0;
		info.pitched = true;
		return info;
	}

	// One-shot: look for a stable period in the body right after the attack.
	const size_t begin = std::min<size_t>(len / 8, 600);
	const size_t n = std::min<size_t>(len - begin, 1536);
	if (n < 128) return info;
	double best = 0, best_lag = 0;
	const size_t max_lag = std::min<size_t>(n / 2, 700);
	std::vector<double> ac(max_lag + 2, 0.0);
	for (size_t lag = 6; lag <= max_lag; ++lag) ac[lag] = autocorr(x, begin, n - lag, static_cast<double>(lag));
	for (size_t lag = 7; lag < max_lag; ++lag) {
		if (ac[lag] > ac[lag - 1] && ac[lag] >= ac[lag + 1] && ac[lag] > best * 1.02) {
			best = ac[lag]; best_lag = static_cast<double>(lag);
		}
	}
	// prefer the shortest lag that is nearly as good (avoid octave-down errors)
	if (best > 0) {
		for (size_t lag = 7; lag < static_cast<size_t>(best_lag); ++lag) {
			if (ac[lag] > ac[lag - 1] && ac[lag] >= ac[lag + 1] && ac[lag] >= 0.93 * best) { best_lag = static_cast<double>(lag); best = ac[lag]; break; }
		}
		const size_t l = static_cast<size_t>(best_lag);
		const double a = ac[l - 1], b = ac[l], c = ac[l + 1];
		const double den = a - 2 * b + c;
		if (std::fabs(den) > 1e-12) best_lag += 0.5 * (a - c) / den;
	}
	info.confidence = best;
	if (best >= 0.6) { info.pitched = true; info.cycle_len = best_lag; return info; }

	// Distorted / chordal material has no single repeating cycle: look for a
	// harmonic series instead (harmonic product spectrum on the body).
	if (info.centroid < 0.2) {
		const size_t N = 16384, M = std::min<size_t>(n, 4096);
		std::vector<double> re(N, 0.0), im(N, 0.0);
		for (size_t i = 0; i < M; ++i) re[i] = x[begin + i] * (0.5 - 0.5 * std::cos(2 * M_PI * i / M));
		// naive in-place radix-2 FFT
		for (size_t i = 1, j = 0; i < N; ++i) {
			size_t bit = N >> 1;
			for (; j & bit; bit >>= 1) j ^= bit;
			j ^= bit;
			if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
		}
		for (size_t len2 = 2; len2 <= N; len2 <<= 1) {
			const double ang = -2 * M_PI / len2;
			for (size_t i = 0; i < N; i += len2)
				for (size_t k = 0; k < len2 / 2; ++k) {
					const double wr = std::cos(ang * k), wi = std::sin(ang * k);
					const size_t a = i + k, b2 = i + k + len2 / 2;
					const double tr = re[b2] * wr - im[b2] * wi, ti = re[b2] * wi + im[b2] * wr;
					re[b2] = re[a] - tr; im[b2] = im[a] - ti; re[a] += tr; im[a] += ti;
				}
		}
		std::vector<double> mag(N / 2);
		for (size_t k = 0; k < N / 2; ++k) mag[k] = std::sqrt(re[k] * re[k] + im[k] * im[k]);
		// f0 between 1/700 and 1/8 cycles per byte
		const size_t klo = N / 700, khi = N / 8;
		double bestp = 0, meanp = 0; size_t bestk = 0, cnt = 0;
		for (size_t k = klo; k < khi && 4 * k < N / 2; ++k) {
			const double p = mag[k] * mag[2 * k] * mag[3 * k] * mag[4 * k];
			meanp += p; ++cnt;
			if (p > bestp) { bestp = p; bestk = k; }
		}
		meanp /= std::max<size_t>(cnt, 1);
		if (bestk && bestp > 30.0 * meanp) {
			info.pitched = true;
			info.cycle_len = static_cast<double>(N) / bestk;
			info.confidence = 0.5;
		}
	}
	return info;
}

} // namespace mod2vgm
