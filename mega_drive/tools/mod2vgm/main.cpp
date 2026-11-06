#include <iostream>
#include <string>
#include "mod_parser.h"
#include "converter_engine.h"

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cout << "MOD to Mega Drive VGM Converter (mod2vgm)\n";
		std::cout << "Usage: " << argv[0] << " <input.mod> <output.vgm>\n";
		return 1;
	}

	std::string in_mod_path = argv[1];
	std::string out_vgm_path = argv[2];

	std::cout << "Loading MOD: " << in_mod_path << std::endl;
	mod2vgm::ModFile mod;
	if (!mod.load_from_file(in_mod_path)) {
		std::cerr << "Failed to parse MOD file.\n";
		return 1;
	}

	std::cout << "Title: " << mod.title << "\n";
	std::cout << "Tag: " << mod.tag << "\n";
	std::cout << "Song length (orders): " << static_cast<int>(mod.song_length) << "\n";
	std::cout << "Patterns: " << mod.patterns.size() << "\n";

	std::cout << "Converting to VGM: " << out_vgm_path << " ...\n";
	mod2vgm::ConverterEngine engine;
	if (!engine.convert(mod, out_vgm_path)) {
		std::cerr << "Conversion failed.\n";
		return 1;
	}

	std::cout << "Conversion completed successfully!\n";
	return 0;
}
