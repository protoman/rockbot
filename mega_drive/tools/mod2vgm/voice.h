#pragma once
// How a MOD sample is played on the Mega Drive, and the driver that turns a
// voice into YM2612 / SN76489 register writes. Shared by the converter (writes
// a VGM) and the voice fitter (writes straight into the emulated chips).

#include "fm_patch.h"
#include <cmath>
#include <cstdint>

namespace mod2vgm {

/// Optional SN76489 noise burst layered over the FM voice (snare / hat body).
struct NoiseLayer {
	bool on = false;
	uint8_t rate = 0;        ///< 0..2 = fixed shift rate (high..low pitch), 3 = follows tone 2
	bool white = true;       ///< white (true) or periodic noise
	uint8_t att0 = 4;        ///< initial attenuation, 0 (loudest)..15 (off), 2 dB per step
	uint8_t step = 2;        ///< frames per extra attenuation step (decay speed)
};

struct Voice {
	FmPatch fm;
	bool pitched = true;
	double fixed_hz = 0.0;     ///< unpitched voices: FM frequency at period 428
	double pitch_follow = 1.0; ///< unpitched voices: exponent applied to 428/period
	NoiseLayer noise;
	bool dac = false;          ///< played as real sample data on the YM2612 DAC (percussion)
};

/// Carrier operators of each algorithm (op1..op4).
inline bool is_carrier(int algorithm, int op) {
	static const bool tab[8][4] = {
		{false, false, false, true},  {false, false, false, true},
		{false, false, false, true},  {false, false, false, true},
		{false, true,  false, true},  {false, true,  true,  true},
		{false, true,  true,  true},  {true,  true,  true,  true},
	};
	return tab[algorithm & 7][op];
}

/// Frequency multiplier of the slowest carrier: the note pitch the patch is built around.
inline double anchor_mul(const FmPatch& p) {
	double best = 1e9;
	for (int op = 0; op < 4; ++op) {
		if (!is_carrier(p.algorithm, op)) continue;
		const double m = p.op[op].mul == 0 ? 0.5 : p.op[op].mul;
		if (m < best) best = m;
	}
	return best;
}

/// Receives raw chip writes.
struct RegSink {
	virtual ~RegSink() {}
	virtual void ym(int port, uint8_t reg, uint8_t val) = 0;
	virtual void psg(uint8_t val) = 0;
};

inline int attenuation_tl(int vol) {
	if (vol >= 64) return 0;
	if (vol <= 0) return 127;
	const double tl = -26.6667 * std::log10(static_cast<double>(vol) / 64.0);
	return tl > 127.0 ? 127 : static_cast<int>(std::lround(tl));
}

inline void fm_pitch(double hz, uint8_t& block, uint16_t& fnum) {
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

/// One YM2612 channel with a shadow of what the chip holds, so a register is
/// only written when its value changes.
class FmDriver {
public:
	FmDriver(RegSink& s, int index) : sink_(s), index_(index) {}

	void load(const Voice& v) {
		voice_ = v;
		const FmPatch& p = v.fm;
		const uint8_t port = index_ >= 3 ? 1 : 0, o = index_ % 3;
		static const uint8_t OFF[4] = {0x00, 0x08, 0x04, 0x0C}; // slot order OP1, OP3, OP2, OP4
		sink_.ym(port, 0xB0 + o, ((p.feedback & 7) << 3) | (p.algorithm & 7));
		sink_.ym(port, 0xB4 + o, 0xC0);
		for (int op = 0; op < 4; ++op) {
			const uint8_t r = OFF[op] + o;
			const FmOperator& f = p.op[op];
			sink_.ym(port, 0x30 + r, ((f.dt & 7) << 4) | (f.mul & 15));
			sink_.ym(port, 0x50 + r, ((f.ks & 3) << 6) | (f.ar & 31));
			sink_.ym(port, 0x60 + r, f.dr & 31);
			sink_.ym(port, 0x70 + r, f.sr & 31);
			sink_.ym(port, 0x80 + r, ((f.sl & 15) << 4) | (f.rr & 15));
			sink_.ym(port, 0x90 + r, f.ssg_eg & 15);
		}
		att_ = -1;
		fnum_block_ = -1;
	}

	/// `att` is the MOD volume attenuation in TL steps (0 = full volume).
	void level(int att) {
		if (att == att_) return;
		att_ = att;
		const uint8_t port = index_ >= 3 ? 1 : 0, o = index_ % 3;
		static const uint8_t OFF[4] = {0x00, 0x08, 0x04, 0x0C};
		for (int op = 0; op < 4; ++op) {
			int tl = voice_.fm.op[op].tl;
			if (is_carrier(voice_.fm.algorithm, op)) tl += att;
			sink_.ym(port, 0x40 + OFF[op] + o, tl > 127 ? 127 : tl);
		}
	}

	/// `hz` is the note pitch the listener should hear.
	void pitch(double hz) {
		uint8_t block; uint16_t fnum;
		fm_pitch(hz / anchor_mul(voice_.fm), block, fnum);
		const int packed = (block << 11) | fnum;
		if (packed == fnum_block_) return;
		fnum_block_ = packed;
		const uint8_t port = index_ >= 3 ? 1 : 0, o = index_ % 3;
		sink_.ym(port, 0xA4 + o, (block << 3) | ((fnum >> 8) & 7)); // high byte latches until the low byte
		sink_.ym(port, 0xA0 + o, fnum & 0xFF);
	}

	void key(bool on) {
		const uint8_t ch = index_ >= 3 ? index_ + 1 : index_;
		sink_.ym(0, 0x28, (on ? 0xF0 : 0x00) | ch);
		keyed_ = on;
	}

	bool keyed() const { return keyed_; }
	bool loaded() const { return att_ != -2; }
	const Voice& voice() const { return voice_; }

private:
	RegSink& sink_;
	int index_;
	Voice voice_;
	int att_ = -2;
	int fnum_block_ = -1;
	bool keyed_ = false;
};

/// The single SN76489 noise channel: attenuation follows the voice's decay.
class NoiseDriver {
public:
	explicit NoiseDriver(RegSink& s) : sink_(s) {}

	void start(const NoiseLayer& n, double time) {
		layer_ = n; start_ = time; active_ = n.on; last_ = -1;
		if (!active_) return;
		sink_.psg(0xE0 | (n.white ? 4 : 0) | (n.rate & 3));
	}
	void stop() {
		if (active_) write(15);
		active_ = false;
	}
	/// `vol_att` is the MOD volume in PSG steps (2 dB each).
	void update(double time, int vol_att) {
		if (!active_) return;
		const int frames = static_cast<int>((time - start_) / 735.0 + 0.5);
		int a = layer_.att0 + frames / (layer_.step ? layer_.step : 1) + vol_att;
		if (a > 15) a = 15;
		write(a);
		if (a >= 15) active_ = false;
	}
	bool active() const { return active_; }

private:
	void write(int a) {
		if (a == last_) return;
		last_ = a;
		sink_.psg(0xF0 | a); // noise channel (3) volume
	}
	RegSink& sink_;
	NoiseLayer layer_;
	double start_ = 0;
	bool active_ = false;
	int last_ = -1;
};

inline int psg_steps_for_volume(int vol) {
	if (vol >= 64) return 0;
	if (vol <= 0) return 15;
	const int s = static_cast<int>(std::lround(-20.0 * std::log10(vol / 64.0) / 2.0));
	return s > 15 ? 15 : s;
}

} // namespace mod2vgm
