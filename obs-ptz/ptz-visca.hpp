/* Pan Tilt Zoom camera instance
 *
 * Copyright 2020 Grant Likely <grant.likely@secretlab.ca>
 *
 * SPDX-License-Identifier: GPLv2
 */
#pragma once

#include <QObject>
#include <QTimer>
#ifdef WITH_PTZ_SERIAL
#include <QSerialPort>
#endif // WITH_PTZ_SERIAL
#include <QUdpSocket>
#include "ptz-device.hpp"


#define visca_encoding visca_encoding_ft
#define visca_u4 visca_u4_ft
#define visca_flag visca_flag_ft
#define visca_u7 visca_u7_ft
#define visca_s7 visca_s7_ft
#define visca_u8 visca_u8_ft
#define visca_s16 visca_s16_ft
#define visca_u16 visca_u16_ft
#define ViscaCmd ViscaCmd_ft
#define ViscaInq ViscaInq_ft
#define PTZVisca PTZVisca_ft
#ifdef WITH_PTZ_SERIAL
#define ViscaUART ViscaUART_ft
#define PTZViscaSerial PTZViscaSerial_ft
#endif // WITH_PTZ_SERIAL
#define ViscaUDPSocket ViscaUDPSocket_ft
#define PTZViscaOverIP PTZViscaOverIP_ft

class visca_encoding {
public:
	const char *name;
	int offset;
	visca_encoding(const char *name, int offset) : name(name), offset(offset) { }
	virtual void encode(QByteArray &data, int val) = 0;
	virtual int decode(QByteArray &data) = 0;
};
class visca_u4 : public visca_encoding {
public:
	visca_u4(const char *name, int offset) : visca_encoding(name, offset) { }
	void encode(QByteArray &data, int val) {
		if (data.size() < offset + 1)
			return;
		data[offset] = data[offset] & 0xf0 | val & 0x0f;
	}
	int decode(QByteArray &data) {
		return (data.size() < offset + 1) ? 0 : data[offset] & 0xf;
	}
};
class visca_flag : public visca_encoding {
public:
	visca_flag(const char *name, int offset) : visca_encoding(name, offset) { }
	void encode(QByteArray &data, int val) {
		if (data.size() < offset + 1)
			return;
		data[offset] = val ? 0x2 : 0x3;
	}
	int decode(QByteArray &data) {
		return (data.size() < offset + 1) ? 0 : 0x02 == data[offset];
	}
};
class visca_u7 : public visca_encoding {
public:
	visca_u7(const char *name, int offset) : visca_encoding(name, offset) { }
	void encode(QByteArray &data, int val) {
		if (data.size() < offset + 1)
			return;
		data[offset] = val & 0x7f;
	}
	int decode(QByteArray &data) {
		return (data.size() < offset + 1) ? 0 : data[offset] & 0x7f;
	}
};
class visca_s7 : public visca_encoding {
public:
	visca_s7(const char *name, int offset) : visca_encoding(name, offset) { }
	virtual void encode(QByteArray &data, int val) {
		if (data.size() < offset + 3)
			return;
		data[offset] = abs(val) & 0x7f;
		data[offset+2] = 3;
		if (val)
			data[offset+2] = val < 0 ? 1 : 2;

	}
	virtual int decode(QByteArray &data) {
		return (data.size() < offset + 3) ? 0 : data[offset] & 0x7f;
	}
};
class visca_u8 : public visca_encoding {
public:
	visca_u8(const char *name, int offset) : visca_encoding(name, offset) { }
	void encode(QByteArray &data, int val) {
		if (data.size() < offset + 2)
			return;
		data[offset]   = (val >> 4) & 0x0f;
		data[offset+1] = val & 0x0f;
	}
	int decode(QByteArray &data) {
		if (data.size() < offset + 2)
			return 0;
		int16_t val = (data[offset] & 0xf) << 4 |
			      (data[offset+1] & 0xf);
		return val;
	}
};
class visca_s16 : public visca_encoding {
public:
	visca_s16(const char *name, int offset) : visca_encoding(name, offset) { }
	void encode(QByteArray &data, int val) {
		if (data.size() < offset + 4)
			return;
		data[offset] = (val >> 12) & 0x0f;
		data[offset+1] = (val >> 8) & 0x0f;
		data[offset+2] = (val >> 4) & 0x0f;
		data[offset+3] = val & 0x0f;
	}
	int decode(QByteArray &data) {
		if (data.size() < offset + 4)
			return 0;
		int16_t val = (data[offset] & 0xf) << 12 |
			      (data[offset+1] & 0xf) << 8 |
			      (data[offset+2] & 0xf) << 4 |
			      (data[offset+3] & 0xf);
		return val;
	}
};
class visca_u16 : public visca_encoding {
public:
	visca_u16(const char *name, int offset) : visca_encoding(name, offset) { }
	void encode(QByteArray &data, int val) {
		if (data.size() < offset + 4)
			return;
		data[offset] = (val >> 12) & 0x0f;
		data[offset+1] = (val >> 8) & 0x0f;
		data[offset+2] = (val >> 4) & 0x0f;
		data[offset+3] = val & 0x0f;
	}
	int decode(QByteArray &data) {
		if (data.size() < offset + 4)
			return 0;
		uint16_t val = (data[offset] & 0xf) << 12 |
			       (data[offset+1] & 0xf) << 8 |
			       (data[offset+2] & 0xf) << 4 |
			       (data[offset+3] & 0xf);
		return val;
	}
};

class ViscaCmd
{
public:
	QByteArray cmd;
	QList<visca_encoding*> args;
	QList<visca_encoding*> results;
	ViscaCmd(const char *cmd_hex) : cmd(QByteArray::fromHex(cmd_hex)) { }
	ViscaCmd(const char *cmd_hex, QList<visca_encoding*> args) :
		cmd(QByteArray::fromHex(cmd_hex)), args(args) { }
	ViscaCmd(const char *cmd_hex, QList<visca_encoding*> args, QList<visca_encoding*> rslts) :
		cmd(QByteArray::fromHex(cmd_hex)), args(args), results(rslts) { }
	void encode(const QList<int> &arglist) {
		for (int i = 0; i < arglist.size(), i < args.size(); i++)
			args[i]->encode(cmd, arglist[i]);
	}
	void decode(QObject *target, QByteArray msg) {
		for (int i = 0; i < results.size(); i++)
			target->setProperty(results[i]->name, results[i]->decode(msg));
	}
};
class ViscaInq : public ViscaCmd
{
public:
	ViscaInq(const char *cmd_hex) : ViscaCmd(cmd_hex) { }
	ViscaInq(const char *cmd_hex, QList<visca_encoding*> rslts) : ViscaCmd(cmd_hex, {}, rslts) {}
};

/*
 * VISCA Abstract base class, used for both Serial UART and UDP implementations
 */
class PTZVisca : public PTZDevice
{
	Q_OBJECT

protected:
	unsigned int address;
	QList<ViscaCmd> pending_cmds;
	bool active_cmd[8];
	bool sent_cmd[8];

	virtual void send_immediate(const QByteArray &msg) = 0;
	void send(const ViscaCmd &cmd);
	void send(const ViscaCmd &cmd, const QList<int> &args);
	void send_pending();

protected slots:
	void receive(const QByteArray &msg);

public:
	PTZVisca(std::string type);

	virtual void set_config(OBSData ptz_data) = 0;
	virtual OBSData get_config() = 0;

	void cmd_get_camera_info();
	void pantilt(int pan, int tilt);
	void pantilt_rel(int pan, int tilt);
	void pantilt_stop();
	void pantilt_home();
	void zoom_stop();
	void zoom_tele(int speed);
	void zoom_wide(int speed);
	void memory_reset(int i);
	void memory_set(int i);
	void memory_recall(int i);

	void pantilt_inquiry() override;
	void zoom_inquiry() override;
	bool got_inquiry() override;
	int get_pan() override;
	int get_tilt() override;
	int get_zoom() override;
	void timeout() override;
};

#ifdef WITH_PTZ_SERIAL
/*
 * VISCA over Serial UART classes
 */
class ViscaUART : public QObject
{
	Q_OBJECT

private:
	/* Global lookup table of UART instances, used to eliminate duplicates */
	static std::map<QString, ViscaUART*> interfaces;

	QString port_name;
	QSerialPort uart;
	QByteArray rxbuffer;
	int camera_count;

signals:
	void receive(const QByteArray &packet);
	void reset();

public:
	ViscaUART(QString &port_name);
	void open();
	void close();
	void send(const QByteArray &packet);
	void receive_datagram(const QByteArray &packet);
	QString portName() { return port_name; }

	static ViscaUART *get_interface(QString port_name);

public slots:
	void poll();
};

class PTZViscaSerial : public PTZVisca
{
	Q_OBJECT

private:
	ViscaUART *iface;
	void attach_interface(ViscaUART *iface);

protected:
	void send_immediate(const QByteArray &msg) override;
	void reset();

public:
	PTZViscaSerial(OBSData config);
	~PTZViscaSerial();

	void set_config(OBSData ptz_data);
	OBSData get_config();
};
#endif // WITH_PTZ_SERIAL

/*
 * VISCA over IP classes
 */
class ViscaUDPSocket : public QObject {
	Q_OBJECT

private:
	QUdpSocket visca_socket;

signals:
	void receive(const QByteArray &packet);
	void reset();

public:
	ViscaUDPSocket();
	void receive_datagram(QNetworkDatagram &datagram);
	void send(QHostAddress ip_address, int visca_port, const QByteArray &packet);

	static ViscaUDPSocket *get_interface();

public slots:
	void poll();
};

class PTZViscaOverIP : public PTZVisca {
	Q_OBJECT

private:
	int sequence;
	QHostAddress ip_address;
	int visca_port;
	ViscaUDPSocket *iface;
	void attach_interface(ViscaUDPSocket *iface);

protected:
	void send_immediate(const QByteArray &msg) override;
	void reset();

public:
	PTZViscaOverIP(OBSData config);
	~PTZViscaOverIP();

	void set_config(OBSData ptz_data);
	OBSData get_config();
};
