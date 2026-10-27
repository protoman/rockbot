#include "replayer.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>

namespace mod2vgm {

namespace {

const int kMinPeriod = 113;
const int kMaxPeriod = 856;

const int16_t kSine[32] = {
	0, 24, 49, 74, 97, 120, 141, 161, 180, 197, 212, 224, 235, 244, 250, 253,
	255, 253, 250, 244, 235, 224, 212, 197, 180, 161, 141, 120, 97, 74, 49, 24
};

struct Chan {
	uint8_t sample = 0;
	uint16_t period = 0;        // value before arpeggio / vibrato
	uint16_t target = 0;        // tone portamento target
	int volume = 0;
	int ft_adjust = 0;
	uint8_t porta_up = 0, porta_down = 0, tone_speed = 0;
	uint8_t vib_speed = 0, vib_depth = 0, vib_pos = 0;
	uint8_t trem_speed = 0, trem_depth = 0, trem_pos = 0;
	uint8_t vol_slide = 0;
	uint8_t effect = 0, param = 0;
	// pending delayed note (ED)
	bool delayed = false;
	uint16_t delay_period = 0;
	uint8_t delay_sample = 0;
	bool trig = false;
};

int clamp_period(int p) { return std::max(kMinPeriod, std::min(kMaxPeriod, p)); }

} // namespace

double sample_hz(const ModSample& s, uint16_t period, double cycle_len) {
	if (period == 0 || cycle_len <= 0.0) return 0.0;
	const double ft = (s.fine_tune < 8) ? s.fine_tune : s.fine_tune - 16;
	return (3546894.6 / period) / cycle_len * std::pow(2.0, ft / 96.0);
}

Song replay(const ModFile& mod) {
	Song song;
	Chan ch[4];
	int speed = 6, bpm = 125;
	int order = 0, row = 0;
	double time = 0.0;
	std::map<std::pair<int, int>, size_t> first_tick; // (order,row) -> tick index
	const int restart = (mod.restart_byte < mod.song_length) ? mod.restart_byte : 0;

	bool done = false;
	int guard = 0;
	while (!done && guard++ < 200000) {
		if (order >= mod.song_length) order = restart;
		const uint8_t pat_idx = mod.order_table[order];
		if (pat_idx >= mod.patterns.size()) break;
		const ModPattern& pat = mod.patterns[pat_idx];

		auto key = std::make_pair(order, row);
		auto seen = first_tick.find(key);
		if (seen != first_tick.end()) {
			song.loop_tick = seen->second;
			break;
		}
		first_tick[key] = song.ticks.size();

		int next_order = order, next_row = row + 1;
		bool jump = false, brk = false;
		int jump_order = 0, brk_row = 0;
		int pattern_delay = 0;

		// ---- tick 0: read the row
		for (int c = 0; c < 4; ++c) {
			const ModCell& cell = pat.rows[row][c];
			Chan& s = ch[c];
			s.trig = false;
			s.effect = cell.effect_code;
			s.param = cell.effect_param;
			const uint8_t fx = cell.effect_code, fp = cell.effect_param;
			const uint8_t sub = fp >> 4, low = fp & 15;

			const bool is_porta = (fx == 0x3 || fx == 0x5);
			const bool note_delay = (fx == 0xE && sub == 0xD && low > 0);

			if (cell.sample_id > 0 && cell.sample_id <= 31 && !note_delay) {
				s.sample = cell.sample_id;
				s.volume = mod.samples[cell.sample_id].default_volume;
			}

			if (cell.period > 0) {
				if (note_delay) {
					s.delayed = true;
					s.delay_period = cell.period;
					s.delay_sample = cell.sample_id;
				} else if (is_porta) {
					s.target = cell.period;
				} else {
					s.period = cell.period;
					s.trig = (s.sample != 0);
					s.vib_pos = 0;
					s.trem_pos = 0;
				}
			}

			// effect set-up on tick 0
			switch (fx) {
			case 0x1: if (fp) s.porta_up = fp; break;
			case 0x2: if (fp) s.porta_down = fp; break;
			case 0x3: if (fp) s.tone_speed = fp; break;
			case 0x4:
				if (sub) s.vib_speed = sub;
				if (low) s.vib_depth = low;
				break;
			case 0x5: case 0x6: case 0xA:
				if (fx == 0x6) { /* vibrato continues with memory */ }
				s.vol_slide = fp;
				break;
			case 0x7:
				if (sub) s.trem_speed = sub;
				if (low) s.trem_depth = low;
				break;
			case 0x9: break; // sample offset: FM voices cannot start mid-sample
			case 0xB: jump = true; jump_order = fp; break;
			case 0xC: s.volume = std::min<int>(fp, 64); break;
			case 0xD: brk = true; brk_row = (fp >> 4) * 10 + (fp & 15); break;
			case 0xE:
				switch (sub) {
				case 0x1: s.period = clamp_period(s.period - low); break;
				case 0x2: s.period = clamp_period(s.period + low); break;
				case 0xA: s.volume = std::min(64, s.volume + low); break;
				case 0xB: s.volume = std::max(0, s.volume - low); break;
				case 0xE: pattern_delay = low; break;
				}
				break;
			case 0xF:
				if (fp == 0) { /* PT: stop; treat as no change */ }
				else if (fp <= 32) speed = fp;
				else bpm = fp;
				break;
			}
		}

		if (jump || brk) {
			next_order = jump ? jump_order : order + 1;
			next_row = brk ? brk_row : 0;
			if (next_row > 63) next_row = 0;
		}
		if (row == 63 && !jump && !brk) { next_order = order + 1; next_row = 0; }

		const int rows_repeat = 1 + pattern_delay;
		for (int rep = 0; rep < rows_repeat; ++rep) {
			const double tick_len = 44100.0 * 2.5 / bpm;
			for (int t = 0; t < speed; ++t) {
				Tick tk;
				tk.time = time;
				tk.order = order; tk.row = row; tk.tick = t;
				for (int c = 0; c < 4; ++c) {
					Chan& s = ch[c];
					const uint8_t fx = s.effect, fp = s.param;
					const uint8_t sub = fp >> 4, low = fp & 15;
					bool retrig = false;
					int cur_period = s.period;
					if (rep > 0 && t == 0) s.trig = false;

					if (t > 0) {
						switch (fx) {
						case 0x1: s.period = clamp_period(s.period - s.porta_up); cur_period = s.period; break;
						case 0x2: s.period = clamp_period(s.period + s.porta_down); cur_period = s.period; break;
						case 0x3: case 0x5:
							if (s.target && s.period != s.target) {
								if (s.period > s.target) s.period = std::max<int>(s.target, s.period - s.tone_speed);
								else s.period = std::min<int>(s.target, s.period + s.tone_speed);
							}
							cur_period = s.period;
							break;
						case 0x0:
							if (fp) {
								const int n = (t % 3 == 0) ? 0 : (t % 3 == 1 ? sub : low);
								cur_period = static_cast<int>(std::lround(s.period / std::pow(2.0, n / 12.0)));
							}
							break;
						case 0xE:
							if (sub == 0x9 && low > 0 && (t % low) == 0) retrig = true;
							if (sub == 0xC && t == low) s.volume = 0;
							if (sub == 0xD && t == low && s.delayed) {
								s.period = s.delay_period;
								if (s.delay_sample) {
									s.sample = s.delay_sample;
									s.volume = mod.samples[s.sample].default_volume;
								}
								s.delayed = false;
								retrig = true;
								s.vib_pos = 0;
								cur_period = s.period;
							}
							break;
						}
						if (fx == 0x4 || fx == 0x6) {
							const int d = (kSine[(s.vib_pos >> 0) & 31] * s.vib_depth) / 128;
							cur_period = s.period + (((s.vib_pos & 32) ? -d : d));
							s.vib_pos = (s.vib_pos + s.vib_speed) & 63;
						}
						if (fx == 0x5 || fx == 0x6 || fx == 0xA) {
							const int up = s.vol_slide >> 4, down = s.vol_slide & 15;
							if (up) s.volume = std::min(64, s.volume + up);
							else s.volume = std::max(0, s.volume - down);
						}
					} else if (fx == 0xE && sub == 0xC && low == 0) {
						s.volume = 0;
					}

					int vol = s.volume;
					if (fx == 0x7 && t > 0) {
						const int d = (kSine[s.trem_pos & 31] * s.trem_depth) / 64;
						vol = std::max(0, std::min(64, s.volume + ((s.trem_pos & 32) ? -d : d)));
						s.trem_pos = (s.trem_pos + s.trem_speed) & 63;
					}

					ChannelTick& ct = tk.ch[c];
					ct.trigger = (t == 0 && rep == 0 && s.trig) || retrig;
					ct.sample = s.sample;
					ct.period = static_cast<uint16_t>(std::max(0, cur_period));
					ct.volume = static_cast<uint8_t>(std::max(0, std::min(64, vol)));
				}
				song.ticks.push_back(tk);
				time += tick_len;
			}
		}

		order = next_order;
		row = next_row;
		if (order >= mod.song_length) {
			order = restart;
			if (!jump && !brk) row = 0;
		}
	}
	song.end_time = time;
	return song;
}

} // namespace mod2vgm
