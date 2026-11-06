#pragma once

#include "mod_parser.h"
#include <cstdint>
#include <vector>

namespace mod2vgm {

/// State of one MOD channel at one replayer tick, after every effect ran.
struct ChannelTick {
	bool trigger = false;   ///< a note starts (or retriggers) on this tick
	uint8_t sample = 0;     ///< sample that is (or will be) playing, 0 = none yet
	uint16_t period = 0;    ///< effective Amiga period (arpeggio / vibrato / slides applied)
	uint8_t volume = 0;     ///< effective volume 0..64 (tremolo, slides applied)
};

struct Tick {
	double time = 0.0;      ///< start of the tick, in 44.1 kHz samples
	uint8_t order = 0;
	uint8_t row = 0;
	uint8_t tick = 0;       ///< tick inside the row (0 = the row's first tick)
	ChannelTick ch[4];
};

/// A whole pass of a module, flattened to ticks. Pattern jumps / breaks / the
/// song-end wrap are resolved here, so a backend only walks `ticks`.
struct Song {
	std::vector<Tick> ticks;
	size_t loop_tick = 0;   ///< tick where the song restarts after the last one
	double end_time = 0.0;  ///< time just after the last tick
};

/// ProTracker replayer. Mirrors the behaviour libxmp has for 4-channel
/// "M.K." modules, which is what the PC game's mixer plays.
Song replay(const ModFile& mod);

/// Period -> Hz for a sample, from the Amiga (PAL) clock, the sample's
/// finetune and the length in bytes of one waveform cycle.
double sample_hz(const ModSample& s, uint16_t period, double cycle_len);

} // namespace mod2vgm
