#pragma once

#include "mod_parser.h"
#include "vgm_writer.h"
#include "sample_analysis.h"
#include "voice.h"
#include <string>

namespace mod2vgm {

/// Picks the voice for a sample: the fitted table (voice_table.inc) first,
/// built-in heuristics for samples the fitter has not seen.
Voice voice_for_sample(const ModSample& s, const SampleInfo& info);

/// True when `key` (SampleInfo::key) has a fitted voice.
bool fitted_voice(uint32_t key, Voice& out);

/**
 * @brief Replays a MOD and writes the matching YM2612 / SN76489 stream as a VGM.
 */
class ConverterEngine {
public:
	bool convert(const ModFile& mod, const std::string& out_vgm_path);
};

} // namespace mod2vgm
