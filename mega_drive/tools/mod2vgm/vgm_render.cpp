#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <string>
#include "ym_player.h"

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
		std::cout << "Usage: vgm_render <input.vgm> <output.wav> [max_seconds]\n";
		return 1;
	}
	const double max_secs = (argc > 3) ? atof(argv[3]) : 1e9;
	std::ifstream file(argv[1], std::ios::binary);
	if (!file.is_open()) { std::cerr << "Failed to open VGM: " << argv[1] << std::endl; return 1; }
	std::vector<uint8_t> vgm((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	if (vgm.size() < 0x40) { std::cerr << "Invalid VGM file.\n"; return 1; }

	size_t pos = *reinterpret_cast<uint32_t*>(&vgm[0x34]) + 0x34;
	mod2vgm::YmPlayer ym;
	std::vector<int16_t> pcm;
	std::vector<uint8_t> bank;
	size_t bank_pos = 0;
	const size_t max_samples = static_cast<size_t>(max_secs * 44100.0);
	auto wait = [&](uint32_t n) {
		if (pcm.size() / 2 + n > max_samples) n = static_cast<uint32_t>(max_samples - pcm.size() / 2);
		ym.run(n, pcm);
	};

	while (pos < vgm.size() && pcm.size() / 2 < max_samples) {
		const uint8_t cmd = vgm[pos++];
		if (cmd == 0x66) break;
		else if (cmd == 0x52 || cmd == 0x53) { ym.write_ym(cmd - 0x52, vgm[pos], vgm[pos + 1]); pos += 2; }
		else if (cmd == 0x50) ym.write_psg(vgm[pos++]);
		else if (cmd == 0x61) { wait(vgm[pos] | (vgm[pos + 1] << 8)); pos += 2; }
		else if (cmd == 0x62) wait(735);
		else if (cmd == 0x63) wait(882);
		else if ((cmd & 0xF0) == 0x70) wait((cmd & 0x0F) + 1);
		else if (cmd == 0x67) {
			pos += 2;
			uint32_t sz = vgm[pos] | (vgm[pos + 1] << 8) | (vgm[pos + 2] << 16) | ((uint32_t)vgm[pos + 3] << 24);
			pos += 4;
			bank.assign(vgm.begin() + pos, vgm.begin() + pos + sz);
			pos += sz;
		} else if (cmd == 0xE0) {
			bank_pos = vgm[pos] | (vgm[pos + 1] << 8) | (vgm[pos + 2] << 16) | ((size_t)vgm[pos + 3] << 24);
			pos += 4;
		} else if ((cmd & 0xF0) == 0x80) {
			if (bank_pos < bank.size()) ym.write_ym(0, 0x2A, bank[bank_pos++]);
			wait(cmd & 0x0F);
		}
	}
	write_wav(argv[2], pcm, 44100);
	std::cout << "Rendered " << pcm.size() / 2 << " stereo samples to " << argv[2] << std::endl;
	return 0;
}
