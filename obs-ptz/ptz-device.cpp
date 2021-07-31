/* Pan Tilt Zoom device factory
 *
 * Copyright 2020 Grant Likely <grant.likely@secretlab.ca>
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <obs.hpp>
#include "ptz-device.hpp"
#include "ptz-visca.hpp"

PTZDevice *PTZDevice::make_device(OBSData config)
{
	PTZDevice *ptz = nullptr;
	std::string type = obs_data_get_string(config, "type");

	if (type == "sim")
		ptz = new PTZSimulator(config);
#ifdef WITH_PTZ_SERIAL
	if (type == "visca")
		ptz = new PTZViscaSerial(config);
#endif // WITH_PTZ_SERIAL
	if (type == "visca-over-ip")
		ptz = new PTZViscaOverIP(config);
	return ptz;
}

void PTZDevice::set_config(OBSData config)
{
	/* Update the list of preset names */
	OBSDataArray preset_array = obs_data_get_array(config, "presets");
	obs_data_array_release(preset_array);
	if (preset_array) {
		QStringList preset_names = default_preset_names;
		for (int i = 0; i < obs_data_array_count(preset_array); i++) {
			OBSData preset = obs_data_array_item(preset_array, i);
			obs_data_release(preset);
			int preset_id = obs_data_get_int(preset, "id");
			const char *preset_name = obs_data_get_string(preset, "name");
			if ((preset_id >= 0) && (preset_id < preset_names.size()) && preset_name)
					preset_names[preset_id] = preset_name;
		}
		preset_names_model.setStringList(preset_names);
	}
}

OBSData PTZDevice::get_config()
{
	OBSData config = obs_data_create();
	obs_data_release(config);

	obs_data_set_string(config, "type", type.c_str());
	QStringList list = preset_names_model.stringList();
	OBSDataArray preset_array = obs_data_array_create();
	obs_data_array_release(preset_array);
	for (int i = 0; i < list.size(); i++) {
		OBSData preset = obs_data_create();
		obs_data_release(preset);
		obs_data_set_int(preset, "id", i);
		obs_data_set_string(preset, "name", qPrintable(list[i]));
		obs_data_array_push_back(preset_array, preset);
	}
	obs_data_set_array(config, "presets", preset_array);
	return config;
}
