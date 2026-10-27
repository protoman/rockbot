#include "vgm_writer.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>

namespace mod2vgm {

static void write_u32_le(std::vector<uint8_t>& buf, size_t offset, uint32_t val) {
	buf[offset + 0] = static_cast<uint8_t>(val & 0xFF);
	buf[offset + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
	buf[offset + 2] = static_cast<uint8_t>((val >> 16) & 0xFF);
	buf[offset + 3] = static_cast<uint8_t>((val >> 24) & 0xFF);
}

VgmWriter::VgmWriter() {
	// Pre-reserve standard 256-byte header
	data_.resize(0x100, 0x00);

	// "Vgm " identification
	data_[0] = 'V';
	data_[1] = 'g';
	data_[2] = 'm';
	data_[3] = ' ';

	// Version 1.50 (0x00000150)
	write_u32_le(data_, 0x08, 0x00000150);

	// Rate: 60Hz or 50Hz (default 60)
	write_u32_le(data_, 0x24, 60);

	// VGM data offset: relative to 0x34. For 0x100 header, 0x100 - 0x34 = 0xCC
	write_u32_le(data_, 0x34, 0x000000CC);
}

void VgmWriter::set_clocks(uint32_t ym2612_clock, uint32_t sn76489_clock) {
	ym_clock_ = ym2612_clock;
	psg_clock_ = sn76489_clock;
	write_u32_le(data_, 0x2C, ym_clock_);
	write_u32_le(data_, 0x0C, psg_clock_);
}

void VgmWriter::write_ym2612(uint8_t port, uint8_t reg, uint8_t val) {
	data_.push_back(port == 0 ? 0x52 : 0x53);
	data_.push_back(reg);
	data_.push_back(val);
}

void VgmWriter::write_psg(uint8_t val) {
	data_.push_back(0x50);
	data_.push_back(val);
}

void VgmWriter::set_pcm_bank(const std::vector<uint8_t>& bank) {
	bank_ = bank;
	if (bank.empty()) return;
	data_.push_back(0x67);
	data_.push_back(0x66);
	data_.push_back(0x00); // YM2612 PCM data
	const uint32_t n = static_cast<uint32_t>(bank.size());
	for (int i = 0; i < 4; ++i) data_.push_back(static_cast<uint8_t>((n >> (8 * i)) & 0xFF));
	data_.insert(data_.end(), bank.begin(), bank.end());
	write_ym2612(0, 0x2B, 0x80); // DAC on (takes over FM channel 6)
}

void VgmWriter::play_pcm(uint32_t offset, uint32_t len, double rate) {
	pcm_active_ = len > 0;
	pcm_offset_ = offset;
	pcm_len_ = len;
	pcm_idx_ = 0;
	pcm_rate_ = rate;
	pcm_t0_ = static_cast<double>(now_);
	data_.push_back(0xE0);
	for (int i = 0; i < 4; ++i) data_.push_back(static_cast<uint8_t>((offset >> (8 * i)) & 0xFF));
}

void VgmWriter::wait_idle(int64_t samples) {
	while (samples > 0) {
		if (samples >= 735) {
			data_.push_back(0x62);
			now_ += 735;
			samples -= 735;
		} else if (samples <= 16) {
			data_.push_back(static_cast<uint8_t>(0x70 + (samples - 1)));
			now_ += samples;
			samples = 0;
		} else {
			data_.push_back(0x61);
			data_.push_back(static_cast<uint8_t>(samples & 0xFF));
			data_.push_back(static_cast<uint8_t>((samples >> 8) & 0xFF));
			now_ += samples;
			samples = 0;
		}
	}
}

void VgmWriter::advance_to(double time) {
	const int64_t target = static_cast<int64_t>(std::floor(time + 0.5));
	while (now_ < target) {
		if (pcm_active_) {
			// byte i is due at t0 + i / rate; the 0x8n wait lands the next byte on its slot
			const int64_t due = static_cast<int64_t>(std::floor(pcm_t0_ + (pcm_idx_ + 1) * 44100.0 / pcm_rate_ + 0.5));
			int64_t w = due - now_;
			if (w < 0) w = 0;
			if (w > 15) w = 15;
			data_.push_back(static_cast<uint8_t>(0x80 + w));
			now_ += w;
			if (++pcm_idx_ >= pcm_len_) pcm_active_ = false;
		} else {
			wait_idle(target - now_);
		}
	}
}

void VgmWriter::mark_loop_point() {
	loop_offset_byte_ = static_cast<uint32_t>(data_.size() - 0x1C);
	loop_start_ = now_;
}

bool VgmWriter::write_to_file(const std::string& file_path) {
	// End of Sound Data command
	data_.push_back(0x66);

	// Update header fields
	// 0x04: EoF offset (File size - 4)
	write_u32_le(data_, 0x04, static_cast<uint32_t>(data_.size() - 4));

	// Clocks
	write_u32_le(data_, 0x2C, ym_clock_);
	write_u32_le(data_, 0x0C, psg_clock_);

	// Total samples
	write_u32_le(data_, 0x18, static_cast<uint32_t>(now_));

	// Loop points
	if (loop_offset_byte_ > 0) {
		write_u32_le(data_, 0x1C, loop_offset_byte_);
		write_u32_le(data_, 0x20, static_cast<uint32_t>(now_ - loop_start_));
	}

	std::ofstream out(file_path, std::ios::binary);
	if (!out.is_open()) {
		std::cerr << "Failed to write VGM file: " << file_path << std::endl;
		return false;
	}
	out.write(reinterpret_cast<const char*>(data_.data()), data_.size());
	out.close();

	return true;
}

} // namespace mod2vgm
