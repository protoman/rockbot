#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mod2vgm {

/**
 * @brief Structure representing a single ProTracker sample header.
 */
struct ModSample {
	std::string name;
	uint16_t length_words = 0;       ///< Length in 16-bit words
	uint32_t length_bytes = 0;       ///< Length in bytes (length_words * 2)
	uint8_t fine_tune = 0;           ///< Finetune (0-15)
	uint8_t default_volume = 0;      ///< Volume (0-64)
	uint16_t repeat_offset_words = 0;
	uint16_t repeat_length_words = 0;
	std::vector<int8_t> pcm_data;    ///< Raw 8-bit signed sample data
};

/**
 * @brief Structure representing a single pattern cell (one note in one channel).
 */
struct ModCell {
	uint8_t sample_id = 0;           ///< 1-31 (0 = no sample trigger)
	uint16_t period = 0;             ///< Period value (0 = no note)
	uint8_t effect_code = 0;         ///< 0x0 to 0xF
	uint8_t effect_param = 0;        ///< Parameter byte
};

/**
 * @brief Structure representing a 4-channel, 64-row pattern.
 */
struct ModPattern {
	ModCell rows[64][4];
};

/**
 * @brief Complete in-memory representation of an Amiga MOD file.
 */
class ModFile {
public:
	/**
	 * @brief Loads and parses a MOD file from disk.
	 * @param file_path Path to the .mod file.
	 * @return True if parsing succeeded, false otherwise.
	 */
	bool load_from_file(const std::string& file_path);

	/**
	 * @brief Converts ProTracker period to frequency in Hz.
	 * @param period Amiga period value.
	 * @return Frequency in Hz.
	 */
	static double period_to_hz(uint16_t period);

	std::string title;
	std::string tag;                 ///< e.g. "M.K."
	std::vector<ModSample> samples;   ///< Indices 1-31 (sample 0 is empty dummy)
	uint8_t song_length = 0;         ///< Number of orders (1-128)
	uint8_t restart_byte = 0;
	std::vector<uint8_t> order_table;
	std::vector<ModPattern> patterns;
};

} // namespace mod2vgm
