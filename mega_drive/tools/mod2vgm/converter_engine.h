#pragma once

#include "mod_parser.h"
#include "vgm_writer.h"
#include "fm_patch.h"
#include "sample_analysis.h"
#include <string>

namespace mod2vgm {

/// How one MOD sample is rendered on the Mega Drive.
struct Voice {
	FmPatch fm;
	double freq_mult = 1.0;   ///< FM fundamental = played pitch * freq_mult
	int gain_tl = 0;          ///< extra attenuation added to every carrier
	double fixed_hz = 0.0;    ///< unpitched voices: FM frequency at period 428
	double pitch_follow = 1.0;///< unpitched voices: exponent of 428/period applied to fixed_hz
};

/// Picks the voice for a sample (fitted table first, heuristics otherwise).
Voice voice_for_sample(const ModSample& s, const SampleInfo& info);

/**
 * @brief Replays a MOD and writes the matching YM2612 stream as a VGM.
 */
class ConverterEngine {
public:
	bool convert(const ModFile& mod, const std::string& out_vgm_path);
};

} // namespace mod2vgm
