#pragma once
// Row layout of voice_table.inc (written by fit_voices).
#include <cstdint>

namespace mod2vgm {

struct VoiceRow {
	uint32_t key;
	uint8_t pitched;
	float fixed_hz, pitch_follow;
	uint8_t alg, fb;
	uint8_t op[4][9];          // dt, mul, tl, ks, ar, dr, sr, rr, sl
	uint8_t noise_on, noise_rate, noise_white, noise_att0, noise_step;
	float fit_loss;
};

} // namespace mod2vgm
