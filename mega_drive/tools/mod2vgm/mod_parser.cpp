#include "mod_parser.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace mod2vgm {

static uint16_t read_u16_be(const uint8_t* buf) {
	return (static_cast<uint16_t>(buf[0]) << 8) | static_cast<uint16_t>(buf[1]);
}

bool ModFile::load_from_file(const std::string& file_path) {
	std::ifstream file(file_path, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open MOD file: " << file_path << std::endl;
		return false;
	}

	std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
	                            std::istreambuf_iterator<char>());
	file.close();

	if (buffer.size() < 1084) {
		std::cerr << "File too small to be a valid MOD file." << std::endl;
		return false;
	}

	// 1. Song title (20 bytes)
	title = std::string(reinterpret_cast<char*>(&buffer[0]), 20);
	title.erase(std::find(title.begin(), title.end(), '\0'), title.end());

	// 2. 31 Sample headers (each 30 bytes, total 930 bytes, offset 20 to 950)
	samples.clear();
	samples.resize(32); // index 0 unused, 1..31 used
	size_t offset = 20;

	for (size_t i = 1; i <= 31; ++i) {
		ModSample& s = samples[i];
		s.name = std::string(reinterpret_cast<char*>(&buffer[offset]), 22);
		s.name.erase(std::find(s.name.begin(), s.name.end(), '\0'), s.name.end());

		s.length_words = read_u16_be(&buffer[offset + 22]);
		s.length_bytes = static_cast<uint32_t>(s.length_words) * 2;
		s.fine_tune = buffer[offset + 24] & 0x0F;
		s.default_volume = buffer[offset + 25];
		s.repeat_offset_words = read_u16_be(&buffer[offset + 26]);
		s.repeat_length_words = read_u16_be(&buffer[offset + 28]);
		offset += 30;
	}

	// 3. Song length and Order table (offset 950)
	song_length = buffer[950];
	restart_byte = buffer[951]; // unused or 127
	order_table.clear();
	uint8_t max_pattern_idx = 0;
	for (size_t i = 0; i < 128; ++i) {
		uint8_t pat_idx = buffer[952 + i];
		order_table.push_back(pat_idx);
		if (i < song_length && pat_idx > max_pattern_idx) {
			max_pattern_idx = pat_idx;
		}
	}

	// 4. Format tag (offset 1080, 4 bytes)
	tag = std::string(reinterpret_cast<char*>(&buffer[1080]), 4);
	// ProTracker standard tags: M.K., M!K!, 4CHN, FLT4
	if (tag != "M.K." && tag != "M!K!" && tag != "4CHN" && tag != "FLT4") {
		std::cerr << "Warning: Tag '" << tag << "' might not be 4-channel standard ProTracker." << std::endl;
	}

	// 5. Pattern data (offset 1084)
	size_t num_patterns = static_cast<size_t>(max_pattern_idx) + 1;
	patterns.clear();
	patterns.resize(num_patterns);
	offset = 1084;

	for (size_t p = 0; p < num_patterns; ++p) {
		for (size_t row = 0; row < 64; ++row) {
			for (size_t ch = 0; ch < 4; ++ch) {
				if (offset + 4 > buffer.size()) {
					std::cerr << "Truncated pattern data." << std::endl;
					return false;
				}
				uint8_t b0 = buffer[offset];
				uint8_t b1 = buffer[offset + 1];
				uint8_t b2 = buffer[offset + 2];
				uint8_t b3 = buffer[offset + 3];
				offset += 4;

				ModCell& cell = patterns[p].rows[row][ch];
				// Sample high 4 bits in b0, low 4 bits in b2
				cell.sample_id = (b0 & 0xF0) | ((b2 & 0xF0) >> 4);
				// Period is 12-bit in (b0 & 0x0F) and b1
				cell.period = ((static_cast<uint16_t>(b0 & 0x0F)) << 8) | b1;
				// Effect code: low 4 bits of b2
				cell.effect_code = b2 & 0x0F;
				cell.effect_param = b3;
			}
		}
	}

	// 6. PCM Sample Data follows patterns
	for (size_t i = 1; i <= 31; ++i) {
		ModSample& s = samples[i];
		if (s.length_bytes > 0) {
			if (offset + s.length_bytes <= buffer.size()) {
				s.pcm_data.resize(s.length_bytes);
				for (size_t b = 0; b < s.length_bytes; ++b) {
					s.pcm_data[b] = static_cast<int8_t>(buffer[offset + b]);
				}
				offset += s.length_bytes;
			}
		}
	}

	return true;
}

double ModFile::period_to_hz(uint16_t period) {
	if (period == 0) return 0.0;
	// In ProTracker: period 428 is C-3 (MIDI 48 = 130.8128 Hz), period 214 is C-4 (MIDI 60 = 261.6256 Hz).
	// Musical note = 48.0 + 12.0 * log2(428.0 / period)
	// Musical frequency = 440.0 * 2^((note - 69.0) / 12.0)
	// Simplified direct formula:
	// freq = 130.81278 * (428.0 / period)
	return 130.81278 * (428.0 / static_cast<double>(period));
}

} // namespace mod2vgm
