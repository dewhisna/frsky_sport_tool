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

#ifndef FRSKY_SPORT_TELEMETRY_H
#define FRSKY_SPORT_TELEMETRY_H

#include "frsky_sport_io.h"

#include <QObject>
#include <QString>
#include <QPointer>
#include <QQueue>

#include <assert.h>

// Forward Declarations
class CUICallback;

// ============================================================================

constexpr uint16_t DATA_ID_ALT_FIRST              = 0x0100;
constexpr uint16_t DATA_ID_ALT_LAST               = 0x010F;
constexpr uint16_t DATA_ID_VARIO_FIRST            = 0x0110;
constexpr uint16_t DATA_ID_VARIO_LAST             = 0x011F;
constexpr uint16_t DATA_ID_CURR_FIRST             = 0x0200;
constexpr uint16_t DATA_ID_CURR_LAST              = 0x020F;
constexpr uint16_t DATA_ID_VFAS_FIRST             = 0x0210;
constexpr uint16_t DATA_ID_VFAS_LAST              = 0x021F;
constexpr uint16_t DATA_ID_CELLS_FIRST            = 0x0300;
constexpr uint16_t DATA_ID_CELLS_LAST             = 0x030F;
constexpr uint16_t DATA_ID_T1_FIRST               = 0x0400;
constexpr uint16_t DATA_ID_T1_LAST                = 0x040F;
constexpr uint16_t DATA_ID_T2_FIRST               = 0x0410;
constexpr uint16_t DATA_ID_T2_LAST                = 0x041F;
constexpr uint16_t DATA_ID_RPM_FIRST              = 0x0500;
constexpr uint16_t DATA_ID_RPM_LAST               = 0x050F;
constexpr uint16_t DATA_ID_FUEL_FIRST             = 0x0600;
constexpr uint16_t DATA_ID_FUEL_LAST              = 0x060F;
constexpr uint16_t DATA_ID_ACCX_FIRST             = 0x0700;
constexpr uint16_t DATA_ID_ACCX_LAST              = 0x070F;
constexpr uint16_t DATA_ID_ACCY_FIRST             = 0x0710;
constexpr uint16_t DATA_ID_ACCY_LAST              = 0x071F;
constexpr uint16_t DATA_ID_ACCZ_FIRST             = 0x0720;
constexpr uint16_t DATA_ID_ACCZ_LAST              = 0x072F;
constexpr uint16_t DATA_ID_GPS_LONG_LATI_FIRST    = 0x0800;
constexpr uint16_t DATA_ID_GPS_LONG_LATI_LAST     = 0x080F;
constexpr uint16_t DATA_ID_GPS_ALT_FIRST          = 0x0820;
constexpr uint16_t DATA_ID_GPS_ALT_LAST           = 0x082F;
constexpr uint16_t DATA_ID_GPS_SPEED_FIRST        = 0x0830;
constexpr uint16_t DATA_ID_GPS_SPEED_LAST         = 0x083F;
constexpr uint16_t DATA_ID_GPS_COURS_FIRST        = 0x0840;
constexpr uint16_t DATA_ID_GPS_COURS_LAST         = 0x084F;
constexpr uint16_t DATA_ID_GPS_TIME_DATE_FIRST    = 0x0850;
constexpr uint16_t DATA_ID_GPS_TIME_DATE_LAST     = 0x085F;
constexpr uint16_t DATA_ID_A3_FIRST               = 0x0900;
constexpr uint16_t DATA_ID_A3_LAST                = 0x090F;
constexpr uint16_t DATA_ID_A4_FIRST               = 0x0910;
constexpr uint16_t DATA_ID_A4_LAST                = 0x091F;
constexpr uint16_t DATA_ID_AIR_SPEED_FIRST        = 0x0A00;
constexpr uint16_t DATA_ID_AIR_SPEED_LAST         = 0x0A0F;
constexpr uint16_t DATA_ID_FUEL_QTY_FIRST         = 0x0A10;
constexpr uint16_t DATA_ID_FUEL_QTY_LAST          = 0x0A1F;
constexpr uint16_t DATA_ID_RBOX_BATT1_FIRST       = 0x0B00;
constexpr uint16_t DATA_ID_RBOX_BATT1_LAST        = 0x0B0F;
constexpr uint16_t DATA_ID_RBOX_BATT2_FIRST       = 0x0B10;
constexpr uint16_t DATA_ID_RBOX_BATT2_LAST        = 0x0B1F;
constexpr uint16_t DATA_ID_RBOX_STATE_FIRST       = 0x0B20;
constexpr uint16_t DATA_ID_RBOX_STATE_LAST        = 0x0B2F;
constexpr uint16_t DATA_ID_RBOX_CNSP_FIRST        = 0x0B30;
constexpr uint16_t DATA_ID_RBOX_CNSP_LAST         = 0x0B3F;
constexpr uint16_t DATA_ID_SD1_FIRST              = 0x0B40;
constexpr uint16_t DATA_ID_SD1_LAST               = 0x0B4F;
constexpr uint16_t DATA_ID_ESC_POWER_FIRST        = 0x0B50;
constexpr uint16_t DATA_ID_ESC_POWER_LAST         = 0x0B5F;
constexpr uint16_t DATA_ID_ESC_RPM_CONS_FIRST     = 0x0B60;
constexpr uint16_t DATA_ID_ESC_RPM_CONS_LAST      = 0x0B6F;
constexpr uint16_t DATA_ID_ESC_TEMPERATURE_FIRST  = 0x0B70;
constexpr uint16_t DATA_ID_ESC_TEMPERATURE_LAST   = 0x0B7F;
constexpr uint16_t DATA_ID_RB3040_OUTPUT_FIRST    = 0x0B80;
constexpr uint16_t DATA_ID_RB3040_OUTPUT_LAST     = 0x0B8F;
constexpr uint16_t DATA_ID_RB3040_CH1_2_FIRST     = 0x0B90;
constexpr uint16_t DATA_ID_RB3040_CH1_2_LAST      = 0x0B9F;
constexpr uint16_t DATA_ID_RB3040_CH3_4_FIRST     = 0x0BA0;
constexpr uint16_t DATA_ID_RB3040_CH3_4_LAST      = 0x0BAF;
constexpr uint16_t DATA_ID_RB3040_CH5_6_FIRST     = 0x0BB0;
constexpr uint16_t DATA_ID_RB3040_CH5_6_LAST      = 0x0BBF;
constexpr uint16_t DATA_ID_RB3040_CH7_8_FIRST     = 0x0BC0;
constexpr uint16_t DATA_ID_RB3040_CH7_8_LAST      = 0x0BCF;
constexpr uint16_t DATA_ID_X8R_FIRST              = 0x0C20;
constexpr uint16_t DATA_ID_X8R_LAST               = 0x0C2F;
constexpr uint16_t DATA_ID_SxR_FIRST              = 0x0C30;
constexpr uint16_t DATA_ID_SxR_LAST               = 0x0C3F;
constexpr uint16_t DATA_ID_GASSUIT_TEMP1_FIRST    = 0x0D00;
constexpr uint16_t DATA_ID_GASSUIT_TEMP1_LAST     = 0x0D0F;
constexpr uint16_t DATA_ID_GASSUIT_TEMP2_FIRST    = 0x0D10;
constexpr uint16_t DATA_ID_GASSUIT_TEMP2_LAST     = 0x0D1F;
constexpr uint16_t DATA_ID_GASSUIT_SPEED_FIRST    = 0x0D20;
constexpr uint16_t DATA_ID_GASSUIT_SPEED_LAST     = 0x0D2F;
constexpr uint16_t DATA_ID_GASSUIT_RES_VOL_FIRST  = 0x0D30;
constexpr uint16_t DATA_ID_GASSUIT_RES_VOL_LAST   = 0x0D3F;
constexpr uint16_t DATA_ID_GASSUIT_RES_PERC_FIRST = 0x0D40;
constexpr uint16_t DATA_ID_GASSUIT_RES_PERC_LAST  = 0x0D4F;
constexpr uint16_t DATA_ID_GASSUIT_FLOW_FIRST     = 0x0D50;
constexpr uint16_t DATA_ID_GASSUIT_FLOW_LAST      = 0x0D5F;
constexpr uint16_t DATA_ID_GASSUIT_MAX_FLOW_FIRST = 0x0D60;
constexpr uint16_t DATA_ID_GASSUIT_MAX_FLOW_LAST  = 0x0D6F;
constexpr uint16_t DATA_ID_GASSUIT_AVG_FLOW_FIRST = 0x0D70;
constexpr uint16_t DATA_ID_GASSUIT_AVG_FLOW_LAST  = 0x0D7F;
constexpr uint16_t DATA_ID_SBEC_POWER_FIRST       = 0x0E50;
constexpr uint16_t DATA_ID_SBEC_POWER_LAST        = 0x0E5F;
constexpr uint16_t DATA_ID_DIY_STREAM_FIRST       = 0x5000;
constexpr uint16_t DATA_ID_DIY_STREAM_LAST        = 0x50FF;
constexpr uint16_t DATA_ID_DIY_FIRST              = 0x5100;
constexpr uint16_t DATA_ID_DIY_LAST               = 0x52FF;
constexpr uint16_t DATA_ID_SERVO_FIRST            = 0x6800;
constexpr uint16_t DATA_ID_SERVO_LAST             = 0x680F;
constexpr uint16_t DATA_ID_FACT_TEST              = 0xF000;
constexpr uint16_t DATA_ID_VALID_FRAME_RATE       = 0xF010;
constexpr uint16_t DATA_ID_RSSI                   = 0xF101;
constexpr uint16_t DATA_ID_ADC1                   = 0xF102;
constexpr uint16_t DATA_ID_ADC2                   = 0xF103;
constexpr uint16_t DATA_ID_BATT                   = 0xF104;
constexpr uint16_t DATA_ID_RAS                    = 0xF105;
constexpr uint16_t DATA_ID_XJT_VERSION            = 0xF106;
constexpr uint16_t DATA_ID_R9_PWR                 = 0xF107;
constexpr uint16_t DATA_ID_SP2UART_A              = 0xFD00;
constexpr uint16_t DATA_ID_SP2UART_B              = 0xFD01;

// ============================================================================

class CFrskySportDeviceTelemetry : public QObject
{
	Q_OBJECT

public:
	explicit CFrskySportDeviceTelemetry(CFrskySportIO &frskySportIO, CUICallback *pUICallback = nullptr, QObject *pParent = nullptr);
	virtual ~CFrskySportDeviceTelemetry();

	QString getLastError() const { return m_strLastError; }

	bool isOpen() const { return m_frskySportIO.isOpen(); }

	bool haveTelemetryPoll(int nPhysicalId) const
	{
		assert(nPhysicalId < TELEMETRY_PHYS_ID_COUNT);
		return m_telemetryEndpoints[nPhysicalId].m_bReceivingPolls;
	}
	bool isTelemetryPushAvailable(int nPhysicalId) const
	{
		assert(nPhysicalId < TELEMETRY_PHYS_ID_COUNT);
		return !m_telemetryEndpoints[nPhysicalId].m_bInUse;
	}
	void resetTelemetry()
	{
		for (int i = 0; i < TELEMETRY_PHYS_ID_COUNT; ++i) {
			m_telemetryEndpoints[i].m_bReceivingPolls = false;
			m_telemetryEndpoints[i].m_bInUse = false;
			m_telemetryEndpoints[i].m_strLogDetail.clear();
		}
	}

public slots:
	// Immediate Transmit:
	void txSportPacket(const CSportTelemetryPacket &packet, const QString &strLogDetail = QString(), bool bIsPushResponse = false);
	// Push Transmit:
	void pushTelemetryResponse(const CSportTelemetryPacket &packet, const QString &strLogDetail = QString());

signals:
	void rxSportPacket(const CSportTelemetryPacket &packet);		// Emitted when m_rxBuffer has a Sport Telemetry packet for consumer (like Lua) to process

	// Private:
	void dataAvailable();

protected slots:
	void en_readyRead();
	void en_receive();
	// ----
	void en_userCancel();

protected:
	struct FrameProcessResult				// Result from processFrame()
	{
//		bool m_bAdvanceState = false;		// If true, then call nextState to advance state-machine
		QString m_strLogDetail;				// Additional log file detail to add to message being processed
	};
	FrameProcessResult processFrame();		// Process the current frame in m_rxBuffer

protected:
	// -----
	CSportRxBuffer m_rxBuffer;				// Receive Sport Packet buffer from serial en_receive events
	CSportTxBuffer m_txBufferLast;			// Last Transmit Sport Packet buffer -- used to detect echos

	struct {
		bool m_bReceivingPolls = false;			// True when this telemetry physical ID is receiving polls
		CSportTelemetryPacket m_responsePacket;	// Packet to transmit in response to poll (such as Lua push)
		QString m_strLogDetail;					// Detail to log when pushing it
		bool m_bInUse = false;					// True when the response packet is in use (ready to be sent and not available to populate)
	} m_telemetryEndpoints[TELEMETRY_PHYS_ID_COUNT];

	QString m_strLastError;					// Last error to report
	CFrskySportIO &m_frskySportIO;			// Serial Port handler for Sport I/O
	CUICallback *m_pUICallback = nullptr;	// Optional user interface callback
};

// ============================================================================

#endif	// FRSKY_SPORT_TELEMETRY_H
