#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mod2vgm {

/**
 * @brief Handles generation of standard VGM files (v1.50).
 */
class VgmWriter {
public:
	VgmWriter();

	/**
	 * @brief Set the hardware chip clocks.
	 * @param ym2612_clock Clock frequency in Hz (typically 7670453).
	 * @param sn76489_clock Clock frequency in Hz (typically 3579545).
	 */
	void set_clocks(uint32_t ym2612_clock, uint32_t sn76489_clock);

	/**
	 * @brief Write a register to YM2612.
	 * @param port 0 for channels 1-3, 1 for channels 4-6.
	 * @param reg Register address.
	 * @param val Value to write.
	 */
	void write_ym2612(uint8_t port, uint8_t reg, uint8_t val);

	/**
	 * @brief Write a byte to the SN76489 PSG.
	 * @param val Register / data byte.
	 */
	void write_psg(uint8_t val);

	/**
	 * @brief Wait for a specific number of 44100Hz samples.
	 * @param samples Number of samples to wait.
	 */
	void wait_samples(uint32_t samples);

	/**
	 * @brief End sound data and save to file.
	 * @param file_path Path where .vgm will be written.
	 * @return True on success.
	 */
	bool write_to_file(const std::string& file_path);

	/**
	 * @brief Mark current write position as loop point.
	 */
	void mark_loop_point();

private:
	uint32_t ym_clock_ = 7670453;
	uint32_t psg_clock_ = 3579545;
	uint32_t total_samples_ = 0;
	uint32_t loop_samples_ = 0;
	uint32_t loop_offset_byte_ = 0;
	std::vector<uint8_t> data_;
};

} // namespace mod2vgm
