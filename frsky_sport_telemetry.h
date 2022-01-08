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
