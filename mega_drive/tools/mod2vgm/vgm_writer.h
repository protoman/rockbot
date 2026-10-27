#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace mod2vgm {

/**
 * @brief Minimal VGM 1.50 writer for YM2612 + SN76489 with a YM2612 PCM bank.
 *
 * Time is tracked in exact 44.1 kHz samples. Callers quantise event times to
 * the 60 Hz frame grid themselves (XGM2 is frame based); while a PCM sample
 * streams, waits are spent on the DAC bytes (0x8n commands) so the stream
 * keeps its rate and other events interleave between them.
 */
class VgmWriter {
public:
	VgmWriter();

	void set_clocks(uint32_t ym2612_clock, uint32_t sn76489_clock);

	void write_ym2612(uint8_t port, uint8_t reg, uint8_t val);
	void write_psg(uint8_t val);

	/// Emits the YM2612 PCM data block. Call once, before the loop point.
	void set_pcm_bank(const std::vector<uint8_t>& bank);

	/// Starts streaming `len` bytes of the bank from `offset` at `rate` Hz.
	/// A stream already running is cut.
	void play_pcm(uint32_t offset, uint32_t len, double rate);

	/// Moves the stream to `time` (44.1 kHz samples), feeding PCM bytes on the way.
	void advance_to(double time);

	void mark_loop_point();
	bool write_to_file(const std::string& file_path);

	int64_t now() const { return now_; }

private:
	void wait_idle(int64_t samples);

	uint32_t ym_clock_ = 7670453;
	uint32_t psg_clock_ = 3579545;
	int64_t now_ = 0;
	int64_t loop_start_ = 0;
	uint32_t loop_offset_byte_ = 0;
	std::vector<uint8_t> data_;
	std::vector<uint8_t> bank_;

	bool pcm_active_ = false;
	uint32_t pcm_offset_ = 0, pcm_len_ = 0, pcm_idx_ = 0;
	double pcm_rate_ = 13300.0, pcm_t0_ = 0.0;
};

} // namespace mod2vgm
