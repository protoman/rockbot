#pragma once

#include <cstdint>
#include <string>

namespace mod2vgm {

/**
 * @brief YM2612 4-Operator voice patch.
 */
struct FmOperator {
	uint8_t dt = 0;     ///< Detune (0-7)
	uint8_t mul = 1;    ///< Multiple (0-15)
	uint8_t tl = 0;     ///< Total Level / Volume (0-127, 0 = maximum volume)
	uint8_t ks = 0;     ///< Key Scale (0-3)
	uint8_t ar = 31;    ///< Attack Rate (0-31)
	uint8_t dr = 10;    ///< Decay Rate (0-31)
	uint8_t sr = 0;     ///< Sustain Rate (0-31)
	uint8_t rr = 8;     ///< Release Rate (0-15)
	uint8_t sl = 2;     ///< Sustain Level (0-15)
	uint8_t ssg_eg = 0; ///< SSG-EG (0-15)
};

struct FmPatch {
	std::string name;
	uint8_t algorithm = 4; ///< Algorithm (0-7)
	uint8_t feedback = 0;  ///< Feedback (0-7)
	FmOperator op[4];      ///< Operators 1, 2, 3, 4
};

/**
 * @brief Helper to load/generate standard FM instruments.
 */
class FmPatchLibrary {
public:
	static FmPatch get_default_lead();
	static FmPatch get_distortion_guitar();
	static FmPatch get_clean_guitar();
	static FmPatch get_default_bass();
	static FmPatch get_slap_bass();
	static FmPatch get_default_pad();
	static FmPatch get_default_percussion();
	static FmPatch get_snare();
	static FmPatch get_hihat();
};

} // namespace mod2vgm
