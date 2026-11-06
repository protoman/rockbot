#pragma once

#include "mod_parser.h"
#include <cstdint>
#include <string>

namespace mod2vgm {

/// What a ProTracker sample is, as far as an FM voice needs to know.
struct SampleInfo {
	bool pitched = false;     ///< has a stable cycle (tonal), as opposed to noise / drum hit
	bool looped = false;      ///< repeats after its attack (sustains for as long as the note lasts)
	double cycle_len = 64.0;  ///< waveform cycle in bytes; played pitch = rate / cycle_len
	double confidence = 0.0;  ///< normalised autocorrelation at the chosen cycle
	double rms = 0.0;         ///< 0..1 of full scale
	double peak = 0.0;        ///< 0..1 of full scale
	double centroid = 0.0;    ///< spectral centroid in cycles per byte (0..0.5)
	uint32_t key = 0;         ///< hash of the audio (identifies the sample across modules)
};

SampleInfo analyze_sample(const ModSample& s);

/// Percussion goes through the DAC as real sample data: a one-shot that the
/// song plays at only a few different pitches. Anything melodic stays FM.
inline bool is_percussion(const SampleInfo& info, size_t length_bytes, size_t distinct_periods) {
	return !info.looped && distinct_periods <= 3 && length_bytes <= 8000;
}

} // namespace mod2vgm
