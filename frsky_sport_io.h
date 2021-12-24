/****************************************************************************
**
** Copyright (C) 2021 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the frsky_sport_tool Application.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#ifndef FRSKY_SPORT_IO_H
#define FRSKY_SPORT_IO_H

#include "PersistentSettings.h"

#include "defs.h"

#include <QObject>
#include <QString>
#include <QByteArray>

#include <QSerialPort>

#include <assert.h>

// ============================================================================

enum FrskyFirmwareProductFamily {
	FIRMWARE_FAMILY_INTERNAL_MODULE,
	FIRMWARE_FAMILY_EXTERNAL_MODULE,
	FIRMWARE_FAMILY_RECEIVER,
	FIRMWARE_FAMILY_SENSOR,
	FIRMWARE_FAMILY_BLUETOOTH_CHIP,
	FIRMWARE_FAMILY_POWER_MANAGEMENT_UNIT,
	FIRMWARE_FAMILY_FLIGHT_CONTROLLER,
};

enum FrskyFirmwareModuleProductId {
	FIRMWARE_ID_MODULE_NONE,
	FIRMWARE_ID_MODULE_XJT = 0x01,
	FIRMWARE_ID_MODULE_ISRM = 0x02,
};

enum FrskyFirmwareReceiverProductId {
	FIRMWARE_ID_RECEIVER_NONE,
	FIRMWARE_ID_RECEIVER_X8R = 0x01,
	FIRMWARE_ID_RECEIVER_RX8R = 0x02,
	FIRMWARE_ID_RECEIVER_RX8R_PRO = 0x03,
	FIRMWARE_ID_RECEIVER_RX6R = 0x04,
	FIRMWARE_ID_RECEIVER_RX4R = 0x05,
	FIRMWARE_ID_RECEIVER_G_RX8 = 0x06,
	FIRMWARE_ID_RECEIVER_G_RX6 = 0x07,
	FIRMWARE_ID_RECEIVER_X6R = 0x08,
	FIRMWARE_ID_RECEIVER_X4R = 0x09,
	FIRMWARE_ID_RECEIVER_X4R_SB = 0x0A,
	FIRMWARE_ID_RECEIVER_XSR = 0x0B,
	FIRMWARE_ID_RECEIVER_XSR_M = 0x0C,
	FIRMWARE_ID_RECEIVER_RXSR = 0x0D,
	FIRMWARE_ID_RECEIVER_S6R = 0x0E,
	FIRMWARE_ID_RECEIVER_S8R = 0x0F,
	FIRMWARE_ID_RECEIVER_XM = 0x10,
	FIRMWARE_ID_RECEIVER_XMP = 0x11,
	FIRMWARE_ID_RECEIVER_XMR = 0x12,
	FIRMWARE_ID_RECEIVER_R9 = 0x13,
	FIRMWARE_ID_RECEIVER_R9_SLIM = 0x14,
	FIRMWARE_ID_RECEIVER_R9_SLIMP = 0x15,
	FIRMWARE_ID_RECEIVER_R9_MINI = 0x16,
	FIRMWARE_ID_RECEIVER_R9_MM = 0x17,
	FIRMWARE_ID_RECEIVER_R9_STAB = 0x18, // R9_STAB has OTA
	FIRMWARE_ID_RECEIVER_R9_MINI_OTA = 0x19, // this one has OTA (different bootloader)
	FIRMWARE_ID_RECEIVER_R9_MM_OTA = 0x1A, // this one has OTA (different bootloader)
	FIRMWARE_ID_RECEIVER_R9_SLIMP_OTA = 0x1B, // this one has OTA (different bootloader)
	FIRMWARE_ID_RECEIVER_ARCHER_X = 0x1C, // this one has OTA (internal module)
	FIRMWARE_ID_RECEIVER_R9MX = 0x1D, // this one has OTA
	FIRMWARE_ID_RECEIVER_R9SX = 0x1E, // this one has OTA
};

// ============================================================================

// Device Physical IDs (Physical IDs + CRC)		// ID, CRC(bits)
constexpr uint8_t PHYS_ID_VARIO =	0x00;		// 00, 000 : Vario2 Altimeter (high precision)
constexpr uint8_t PHYS_ID_FLVSS =	0xA1;		// 01, 101 : Lipo sensor (can be sent with one or two cell voltages)
constexpr uint8_t PHYS_ID_FAS =		0x22;		// 02, 001 : FAS-40S current sensor
constexpr uint8_t PHYS_ID_GPS =		0x83;		// 03, 100 : GPS / altimeter (normal precision)
constexpr uint8_t PHYS_ID_RPM =		0xE4;		// 04, 111 : RPM
constexpr uint8_t PHYS_ID_SP2UH =	0x45;		// 05, 010 : SP2UART(Host)
constexpr uint8_t PHYS_ID_SP2UR =	0xC6;		// 06, 110 : SP2UART(Remote)
constexpr uint8_t PHYS_ID_07 =		0x67;		// 07, 011 :
// ----
constexpr uint8_t PHYS_ID_08 =		0x48;		// 08, 010 :
constexpr uint8_t PHYS_ID_09 =		0xE9;		// 09, 111 :
constexpr uint8_t PHYS_ID_10 =		0x6A;		// 0A, 011 :
constexpr uint8_t PHYS_ID_11 =		0xCB;		// 0B, 110 :
constexpr uint8_t PHYS_ID_12 =		0xAC;		// 0C, 101 :
constexpr uint8_t PHYS_ID_13 =		0x0D;		// 0D, 000 :
constexpr uint8_t PHYS_ID_14 =		0x8E;		// 0E, 100 :
constexpr uint8_t PHYS_ID_15 =		0x2F;		// 0F, 001 :
// ----
constexpr uint8_t PHYS_ID_16 =		0xD0;		// 10, 110 :
constexpr uint8_t PHYS_ID_17 =		0x71;		// 11, 011 :
constexpr uint8_t PHYS_ID_18 =		0xF2;		// 12, 111 :
constexpr uint8_t PHYS_ID_19 =		0x53;		// 13, 010 :
constexpr uint8_t PHYS_ID_20 =		0x34;		// 14, 001 :
constexpr uint8_t PHYS_ID_21 =		0x95;		// 15, 100 :
constexpr uint8_t PHYS_ID_22 =		0x16;		// 16, 000 :
constexpr uint8_t PHYS_ID_IMU =		0xB7;		// 17, 101 : IMU ACC (x,y,z)
// ----
constexpr uint8_t PHYS_ID_24 =		0x98;		// 18, 100 :
constexpr uint8_t PHYS_ID_PWRBOX =	0x39;		// 19, 001 : Power Box
constexpr uint8_t PHYS_ID_TEMP =	0xBA;		// 1A, 101 : Temp
constexpr uint8_t PHYS_ID_FUEL =	0x1B;		// 1B, 000 : Fuel (ArduPilot/Betaflight)
// ~~~~
constexpr uint8_t PHYS_ID_28 =		0x7C;		// 1C, 011 : Reserved?
constexpr uint8_t PHYS_ID_29 =		0xDD;		// 1D, 110 : Reserved?
constexpr uint8_t PHYS_ID_FIRMRSP =	0x5E;		// 1E, 010 : Firmware Response
constexpr uint8_t PHYS_ID_FIRMCMD =	0xFF;		// 1F, 111 : Firmware Command


// FrSky PRIM IDs (1 byte)
constexpr uint8_t PRIM_ID_DATA_FRAME = 0x10;
constexpr uint8_t PRIM_ID_FIRMWARE_FRAME = 0x50;



PACK(union CSportTelemetryPacket
{
	CSportTelemetryPacket(uint8_t nPhysID, uint16_t nDataID, uint32_t nValue)
	{
		m_physicalId = nPhysID;
		m_primId = PRIM_ID_DATA_FRAME;
		m_dataId = nDataID;
		m_value = nValue;
	}
	uint8_t crc() const;
	struct {
		uint8_t m_physicalId;		// Device Physical ID (with included 3-bit CRC)
		uint8_t m_primId;			// Communications Primitive
		uint16_t m_dataId;			// Data Identifier
		uint32_t m_value;			// Data Value
	};
	uint8_t m_raw[8];
});

PACK(union CSportFirmwarePacket
{
	CSportFirmwarePacket(uint8_t nCmd, uint32_t nData = 0, uint8_t nPacket = 0)
	{
		m_physicalId = PHYS_ID_FIRMCMD;
		m_primId = PRIM_ID_FIRMWARE_FRAME;
		m_cmd = nCmd;
		m_data[0] = (nData & 0xFF);
		m_data[1] = ((nData >> 8) & 0xFF);
		m_data[2] = ((nData >> 16) & 0xFF);
		m_data[3] = ((nData >> 24) & 0xFF);
		m_packet = nPacket;
	}
	CSportFirmwarePacket(uint8_t nCmd, uint8_t *pData, uint8_t nPacket = 0)
	{
		assert(pData != nullptr);
		m_physicalId = PHYS_ID_FIRMCMD;
		m_primId = PRIM_ID_FIRMWARE_FRAME;
		m_cmd = nCmd;
		m_data[0] = pData[0];
		m_data[1] = pData[1];
		m_data[2] = pData[2];
		m_data[3] = pData[3];
		m_packet = nPacket;
	}
	uint32_t dataValue() const
	{
		return ( m_data[0] +
				(m_data[1] << 8) +
				(m_data[2] << 16) +
				(m_data[3] << 24));
	}
	uint8_t crc() const;
	struct {
		uint8_t m_physicalId;		// Device Physical ID (with included 3-bit CRC)
		uint8_t m_primId;			// Communications Primitive
		uint8_t m_cmd;				// Flash Command Primitive
		uint8_t m_data[4];			// 32-bits of data on data xfer or 32-bit address on xfer request (not defined as uint32_t due to alignment problems with some host micros)
		uint8_t m_packet;			// index of this 32-bit packet inside 1024-byte block [though FrSky seems to have broken this in their protocol -- see comments in CFrskyDeviceFirmwareUpdate::nextState()]
	};
	uint8_t m_raw[8];
});
// Double-check sizing to make cross-platform builds easier to debug:
static_assert(sizeof(CSportTelemetryPacket) == sizeof(CSportFirmwarePacket), "Packet Structures are broken!");
static_assert(sizeof(CSportTelemetryPacket) == 8, "Packet Structures are broken!");


// ============================================================================

class CSportTxBuffer {
public:
	CSportTxBuffer()
	{
		reset();
	}

	void reset()
	{
		m_data.clear();
	}

	int size() const { return m_data.size(); }
	const QByteArray &data() const { return m_data; }

	void pushByte(uint8_t byte);
	void pushByteWithByteStuffing(uint8_t byte);
	void pushTelemetryPacketWithByteStuffing(const CSportTelemetryPacket & packet);
	void pushFirmwarePacketWithByteStuffing(const CSportFirmwarePacket & packet);

protected:
	QByteArray m_data;
};

// ----------------------------------------------------------------------------


class CSportRxBuffer {
	static constexpr uint8_t SPORT_BUFFER_SIZE = 64;
public:
	CSportRxBuffer()
	{
		reset();
	}

	void reset()
	{
		m_size = 0;
		m_bInEscape = false;
		m_bHaveFrameStart = false;
		m_baExtraneous.clear();
	}

	bool haveCompletePacket() const { return m_size >= (sizeof(CSportFirmwarePacket)+1); }	// Note: all packets are same size, so doesn't matter which sizeof() we use here.  +1 for CRC
	uint8_t size() const { return m_size; }
	const uint8_t *data() const { return m_data; }
	uint8_t crc() const
	{
		assert(m_size > sizeof(CSportFirmwarePacket));	// Note: all packets are same size, so doesn't matter which sizeof() we use here.
		return m_data[sizeof(CSportFirmwarePacket)];
	}
	const CSportTelemetryPacket &telemetryPacket() const
	{
		assert(m_size >= sizeof(CSportTelemetryPacket));
		return m_sportTelemetry;
	}
	const CSportFirmwarePacket &firmwarePacket() const
	{
		assert(m_size >= sizeof(CSportFirmwarePacket));
		return m_sportFirmware;
	}

	bool isTelemetryPacket() const
	{
		if ((m_size == sizeof(CSportTelemetryPacket)) &&
			(m_sportTelemetry.m_primId == PRIM_ID_DATA_FRAME))
			return true;
		return false;
	}

	bool isFirmwarePacket() const
	{
		if ((m_size == sizeof(CSportFirmwarePacket)) &&
			(m_sportFirmware.m_primId == PRIM_ID_FIRMWARE_FRAME))
			return true;
		return false;
	}

	QByteArray pushByte(uint8_t byte);		// Returns any extraneous discarded bytes from before a valid receive so they can be logged

protected:
	union {
		CSportTelemetryPacket m_sportTelemetry;
		CSportFirmwarePacket m_sportFirmware;
		uint8_t m_data[SPORT_BUFFER_SIZE];
	};
	uint8_t m_size = 0;
	bool m_bInEscape = false;			// Set to true when we receive a 0x7D stuff byte
	bool m_bHaveFrameStart = false;		// Set to true when we receive a 0x7E start byte, used so we don't capture incomplete frame before frame start
	QByteArray m_baExtraneous;			// Extraneous received bytes
};


// ============================================================================

class CFrskySportIO : public QObject
{
	Q_OBJECT
public:
	enum LOG_TYPE {
		LT_RX = 0,			// Received Message Log
		LT_TX = 1,			// Transmit Message Log
		LT_TXECHO = 2,		// Transmit Echo Message Log
	};

	CFrskySportIO(SPORT_ID_ENUM nSport, QObject *pParent = nullptr);
	virtual ~CFrskySportIO();

	SPORT_ID_ENUM getSportID() const { return m_nSportID; }
	QSerialPort &port() { return m_serialPort; }

	bool openPort(const QString &strSerialPort = QString());
	void closePort();
	bool isOpen() const { return m_serialPort.isOpen(); }

	QString getLastError() const { return m_strLastError; }

	void logMessage(LOG_TYPE nLT, const QByteArray &baMsg, const QString &strExtraMsg = QString());

signals:
	void writeLogString(SPORT_ID_ENUM nSport, const QString &strLogString);

protected:
	QString m_strLastError;
	SPORT_ID_ENUM m_nSportID;
	QSerialPort m_serialPort;
};

// ============================================================================

#endif	// FRSKY_SPORT_IO_H

