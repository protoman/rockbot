#pragma once
// Cycle-accurate YM2612 (Nuked-OPN2) + SN76489 driven like the console does:
// register writes go through a queue that respects the chip's busy time, and
// output is produced on a 44.1 kHz timeline (FM native 53267 Hz averaged down).

#include <cstdint>
#include <cstdlib>
#include <deque>
#include <vector>

extern "C" {
#include "ym3438.h"
}

namespace mod2vgm {

// SN76489 as found in the Mega Drive VDP (3 tone + 1 noise, 3.579545 MHz).
struct Psg {
	uint16_t tone[3] = {1, 1, 1}, count[3] = {1, 1, 1};
	uint8_t vol[4] = {15, 15, 15, 15}, out[3] = {1, 1, 1};
	uint8_t noise_ctl = 0; uint16_t noise_count = 16, lfsr = 0x8000; uint8_t noise_out = 0;
	int latch = 0; bool latch_vol = false;
	double accum = 0;

	void write(uint8_t v) {
		if (v & 0x80) {
			latch = (v >> 5) & 3; latch_vol = (v >> 4) & 1;
			uint8_t d = v & 15;
			if (latch_vol) vol[latch] = d;
			else if (latch == 3) { noise_ctl = d & 7; lfsr = 0x8000; }
			else tone[latch] = (tone[latch] & 0x3F0) | d;
		} else if (!latch_vol && latch < 3) {
			tone[latch] = (tone[latch] & 0x00F) | ((v & 0x3F) << 4);
		} else if (latch_vol) {
			vol[latch] = v & 15;
		}
	}
	double level() const {
		static const double tab[16] = {1, 0.794, 0.631, 0.501, 0.398, 0.316, 0.251, 0.199,
		                               0.158, 0.126, 0.1, 0.079, 0.063, 0.05, 0.04, 0};
		double s = 0;
		for (int c = 0; c < 3; ++c) s += (out[c] ? 1 : -1) * tab[vol[c]];
		s += (noise_out ? 1 : -1) * tab[vol[3]];
		return s;
	}
	double step(double psg_ticks) {
		accum += psg_ticks;
		double sum = 0; int n = 0;
		while (accum >= 1.0) {
			accum -= 1.0;
			for (int c = 0; c < 3; ++c)
				if (--count[c] <= 0) { count[c] = tone[c] ? tone[c] : 1; out[c] ^= 1; }
			if (--noise_count <= 0) {
				int r = noise_ctl & 3;
				noise_count = (r == 3) ? (tone[2] ? tone[2] : 1) : (16 << r);
				int fb = (noise_ctl & 4) ? ((lfsr ^ (lfsr >> 3)) & 1) : (lfsr & 1);
				lfsr = (lfsr >> 1) | (fb << 15);
				noise_out = lfsr & 1;
			}
			sum += level(); ++n;
		}
		return n ? sum / n : level();
	}
};

class YmPlayer {
public:
	static constexpr int kFmGain = 10;
	static constexpr double kPsgGain = 1800.0; // one full-volume PSG square, in 16-bit units

	YmPlayer() { OPN2_Reset(&chip_); }

	void write_ym(int port, uint8_t reg, uint8_t val) { q_.push_back({port, reg, val}); }
	void write_psg(uint8_t v) { psg_q_.push_back(v); }

	/// Appends `n` output samples (interleaved L,R) to `out`.
	void run(uint32_t n, std::vector<int16_t>& out) {
		const double fm_per_out = (7670453.0 / 144.0) / 44100.0;
		const double psg_per_out = 3579545.0 / 16.0 / 44100.0;
		int16_t buf[2];
		for (uint32_t i = 0; i < n; ++i) {
			while (!psg_q_.empty()) { psg_.write(psg_q_.front()); psg_q_.pop_front(); }
			fm_accum_ += fm_per_out;
			int32_t sl = 0, sr = 0, cnt = 0;
			while (fm_accum_ >= 1.0) {
				feed();
				// the core emits one time-multiplexed slot per clock: a sample is the sum of all 24
				for (int c = 0; c < 24; ++c) { OPN2_Clock(&chip_, buf); sl += buf[0]; sr += buf[1]; }
				++cnt;
				fm_accum_ -= 1.0;
			}
			if (cnt) { last_l_ = sl * kFmGain / cnt; last_r_ = sr * kFmGain / cnt; }
			const int32_t p = static_cast<int32_t>(psg_.step(psg_per_out) * kPsgGain);
			out.push_back(clamp(last_l_ + p));
			out.push_back(clamp(last_r_ + p));
		}
	}

	Psg& psg() { return psg_; }

private:
	struct W { int port; uint8_t reg, val; };

	static int16_t clamp(int32_t v) { return v > 32767 ? 32767 : (v < -32768 ? -32768 : v); }

	// One register write per ~40 chip cycles (address + data + busy time).
	void feed() {
		if (wait_ > 0) { --wait_; return; }
		if (q_.empty()) return;
		const W w = q_.front(); q_.pop_front();
		int16_t buf[2];
		OPN2_Write(&chip_, w.port * 2, w.reg);
		for (int c = 0; c < 3; ++c) OPN2_Clock(&chip_, buf);
		OPN2_Write(&chip_, w.port * 2 + 1, w.val);
		for (int c = 0; c < 3; ++c) OPN2_Clock(&chip_, buf);
		wait_ = 1; // 24 clocks per FM sample: the rest of the busy time elapses in run()
	}

	ym3438_t chip_;
	Psg psg_;
	std::deque<W> q_;
	std::deque<uint8_t> psg_q_;
	double fm_accum_ = 0.0;
	int wait_ = 0;
	int32_t last_l_ = 0, last_r_ = 0;
};

} // namespace mod2vgm
