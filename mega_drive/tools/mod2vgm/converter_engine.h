#pragma once

#include "mod_parser.h"
#include "vgm_writer.h"
#include "fm_patch.h"
#include <map>

namespace mod2vgm {

/**
 * @brief High-level engine that maps MOD tracks to YM2612 and SN76489 VGM streams.
 */
class ConverterEngine {
public:
	ConverterEngine();

	/**
	 * @brief Converts a loaded MOD file to a VGM file.
	 * @param mod Reference to loaded MOD file.
	 * @param out_vgm_path Path to the destination .vgm file.
	 * @param config_path Optional path to custom mapping JSON.
	 * @return True on success.
	 */
	bool convert(const ModFile& mod, const std::string& out_vgm_path, const std::string& config_path = "");

	/**
	 * @brief Override patch for a specific sample ID.
	 */
	void set_instrument_patch(uint8_t sample_id, const FmPatch& patch);

private:
	void apply_fm_patch(VgmWriter& vgm, uint8_t channel, const FmPatch& patch);
	void note_on(VgmWriter& vgm, uint8_t channel, uint16_t period);
	void note_off(VgmWriter& vgm, uint8_t channel);
	void update_channel_frequency(VgmWriter& vgm, uint8_t channel, uint16_t period);
	void set_channel_volume(VgmWriter& vgm, uint8_t channel, uint8_t mod_volume, uint8_t algorithm);

	void calculate_ym2612_pitch(double freq_hz, uint8_t& block, uint16_t& f_num);

	// Voice patch assigned to each of the 31 sample slots
	std::map<uint8_t, FmPatch> sample_to_patch_;
};

} // namespace mod2vgm
