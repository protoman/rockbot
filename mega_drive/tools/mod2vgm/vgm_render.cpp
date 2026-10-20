#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cmath>
#include "ym3438.h"

static void write_wav(const std::string& path, const std::vector<int16_t>& pcm, int sample_rate) {
	std::ofstream out(path, std::ios::binary);
	uint32_t data_size = pcm.size() * sizeof(int16_t);
	uint32_t file_size = 36 + data_size;

	out.write("RIFF", 4);
	out.write(reinterpret_cast<const char*>(&file_size), 4);
	out.write("WAVEfmt ", 8);
	uint32_t fmt_size = 16;
	uint16_t fmt_type = 1; // PCM
	uint16_t channels = 2; // Stereo
	uint32_t s_rate = sample_rate;
	uint32_t byte_rate = sample_rate * channels * sizeof(int16_t);
	uint16_t block_align = channels * sizeof(int16_t);
	uint16_t bits = 16;
	out.write(reinterpret_cast<const char*>(&fmt_size), 4);
	out.write(reinterpret_cast<const char*>(&fmt_type), 2);
	out.write(reinterpret_cast<const char*>(&channels), 2);
	out.write(reinterpret_cast<const char*>(&s_rate), 4);
	out.write(reinterpret_cast<const char*>(&byte_rate), 4);
	out.write(reinterpret_cast<const char*>(&block_align), 2);
	out.write(reinterpret_cast<const char*>(&bits), 2);
	out.write("data", 4);
	out.write(reinterpret_cast<const char*>(&data_size), 4);
	out.write(reinterpret_cast<const char*>(pcm.data()), data_size);
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cout << "Usage: vgm_render <input.vgm> <output.wav>\n";
		return 1;
	}

	std::ifstream file(argv[1], std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open VGM: " << argv[1] << std::endl;
		return 1;
	}

	std::vector<uint8_t> vgm((std::istreambuf_iterator<char>(file)),
	                         std::istreambuf_iterator<char>());
	file.close();

	if (vgm.size() < 0x40) {
		std::cerr << "Invalid VGM file.\n";
		return 1;
	}

	uint32_t data_offset = *reinterpret_cast<uint32_t*>(&vgm[0x34]) + 0x34;
	ym3438_t chip;
	OPN2_Reset(&chip);

	std::vector<int16_t> output_pcm;
	size_t pos = data_offset;
	int16_t buffer[2];

	double fm_sample_accum = 0.0;
	const double fm_rate = 7670453.0 / 144.0; // 53267.0347 Hz native YM2612 rate
	const double vgm_rate = 44100.0;
	const double fm_per_vgm_sample = fm_rate / vgm_rate;

	auto clock_chip = [&](uint32_t samples_44k) {
		fm_sample_accum += samples_44k * fm_per_vgm_sample;
		while (fm_sample_accum >= 1.0) {
			for (int cycle = 0; cycle < 24; ++cycle) {
				OPN2_Clock(&chip, buffer);
			}
			output_pcm.push_back(buffer[0]);
			output_pcm.push_back(buffer[1]);
			fm_sample_accum -= 1.0;
		}
	};

	auto ym_write = [&](uint32_t port, uint8_t reg, uint8_t val) {
		OPN2_Write(&chip, port, reg);
		for (int c = 0; c < 4; ++c) {
			OPN2_Clock(&chip, buffer);
		}
		OPN2_Write(&chip, port + 1, val);
		for (int c = 0; c < 4; ++c) {
			OPN2_Clock(&chip, buffer);
		}
	};

	while (pos < vgm.size()) {
		uint8_t cmd = vgm[pos++];
		if (cmd == 0x66) { // End of sound
			break;
		} else if (cmd == 0x52) { // YM2612 Port 0
			uint8_t reg = vgm[pos++];
			uint8_t val = vgm[pos++];
			ym_write(0, reg, val);
		} else if (cmd == 0x53) { // YM2612 Port 1
			uint8_t reg = vgm[pos++];
			uint8_t val = vgm[pos++];
			ym_write(2, reg, val);
		} else if (cmd == 0x50) { // PSG write (ignored in pure YM renderer)
			pos++;
		} else if (cmd == 0x61) { // Wait n samples
			uint16_t n = vgm[pos] | (vgm[pos+1] << 8);
			pos += 2;
			clock_chip(n);
		} else if (cmd == 0x62) { // Wait 735 samples
			clock_chip(735);
		} else if (cmd == 0x63) { // Wait 882 samples
			clock_chip(882);
		} else if ((cmd & 0xF0) == 0x70) { // Wait 1..16 samples
			clock_chip((cmd & 0x0F) + 1);
		}
	}

	write_wav(argv[2], output_pcm, 53267);
	std::cout << "Rendered " << output_pcm.size() / 2 << " stereo samples to " << argv[2] << std::endl;
	return 0;
}
