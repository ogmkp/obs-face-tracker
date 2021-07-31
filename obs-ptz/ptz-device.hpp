/* Pan Tilt Zoom device base object
 *
 * Copyright 2020 Grant Likely <grant.likely@secretlab.ca>
 *
 * SPDX-License-Identifier: GPLv2
 */
#pragma once

#include <memory>
#include <QDebug>
#include <QObject>
#include <QStringListModel>
#include <QtGlobal>
#include <obs.hpp>
#include <obs-frontend-api.h>

#define default_preset_names default_preset_names_ft
#define PTZDevice PTZDevice_ft
#define PTZSimulator PTZSimulator_ft

const QStringList default_preset_names({"Preset 1", "Preset 2", "Preset 3", "Preset 4", "Preset 5",
					"Preset 6", "Preset 7", "Preset 8", "Preset 9", "Preset 10"});

class PTZDevice : public QObject
{
	Q_OBJECT

protected:
	std::string type;
	QStringListModel preset_names_model;

public:
	PTZDevice(std::string type) : QObject(), type(type)
	{
		preset_names_model.setStringList(default_preset_names);
	};
	~PTZDevice()
	{
	};

	static PTZDevice* make_device(OBSData config);

	virtual void pantilt(int pan, int tilt) { Q_UNUSED(pan); Q_UNUSED(tilt); }
	virtual void pantilt_rel(int pan, int tilt) { Q_UNUSED(pan); Q_UNUSED(tilt); }
	virtual void pantilt_stop() { }
	virtual void pantilt_home() { }
	virtual void zoom_stop() { }
	virtual void zoom_tele(int speed) { Q_UNUSED(speed); }
	virtual void zoom_wide(int speed) { Q_UNUSED(speed); }
	virtual void memory_set(int i) { Q_UNUSED(i); }
	virtual void memory_recall(int i) { Q_UNUSED(i); }
	virtual void memory_reset(int i) { Q_UNUSED(i); }
	virtual QAbstractListModel * presetModel() { return &preset_names_model; }

	virtual void set_config(OBSData ptz_config);
	virtual OBSData get_config();

	virtual void zoom_inquiry() = 0;
	virtual void pantilt_inquiry() = 0;
	virtual bool got_inquiry() = 0;
	virtual int get_pan() = 0;
	virtual int get_tilt() = 0;
	virtual int get_zoom() = 0;
	virtual void timeout() {}
};

class PTZSimulator : public PTZDevice
{
	Q_OBJECT

public:
	PTZSimulator() : PTZDevice("sim") { };
	PTZSimulator(OBSData config) : PTZDevice("sim") { set_config(config); };

	void pantilt(int pan, int tilt) override { blog(LOG_INFO, "%s %f %f", __func__, pan, tilt); }
	void pantilt_stop() override { qDebug() << __func__; }
	void pantilt_home() override { qDebug() << __func__; }
	void zoom_stop() override { qDebug() << __func__; }
	void zoom_tele(int speed) override { blog(LOG_INFO, "%s %f", __func__, speed); }
	void zoom_wide(int speed) override { blog(LOG_INFO, "%s %f", __func__, -speed); }

	void zoom_inquiry() override {}
	void pantilt_inquiry() override {}
	bool got_inquiry() override { return 1; }
	int get_pan() override { return 0; }
	int get_tilt() override { return 0; }
	int get_zoom() override { return 0; }
};
