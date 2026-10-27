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

void VgmWriter::wait_samples(uint32_t samples) {
	total_samples_ += samples;
	while (samples > 0) {
		if (samples == 735) { // 60Hz frame
			data_.push_back(0x62);
			break;
		} else if (samples == 882) { // 50Hz frame
			data_.push_back(0x63);
			break;
		} else if (samples <= 16) {
			data_.push_back(static_cast<uint8_t>(0x70 + (samples - 1)));
			break;
		} else {
			uint16_t chunk = (samples > 65535) ? 65535 : static_cast<uint16_t>(samples);
			data_.push_back(0x61);
			data_.push_back(static_cast<uint8_t>(chunk & 0xFF));
			data_.push_back(static_cast<uint8_t>((chunk >> 8) & 0xFF));
			samples -= chunk;
		}
	}
}

void VgmWriter::mark_loop_point() {
	loop_offset_byte_ = static_cast<uint32_t>(data_.size() - 0x1C);
	loop_samples_ = total_samples_;
}

void VgmWriter::advance_to(double time) {
	const uint32_t target = static_cast<uint32_t>(std::floor(time / 735.0 + 0.5));
	while (frame_ < target) {
		data_.push_back(0x62);
		total_samples_ += 735;
		++frame_;
	}
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
	write_u32_le(data_, 0x18, total_samples_);

	// Loop points
	if (loop_offset_byte_ > 0) {
		write_u32_le(data_, 0x1C, loop_offset_byte_);
		write_u32_le(data_, 0x20, total_samples_ - loop_samples_);
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
