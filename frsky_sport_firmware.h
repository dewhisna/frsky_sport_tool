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

#ifndef FRSKY_SPORT_FIRMWARE_H
#define FRSKY_SPORT_FIRMWARE_H

#include "defs.h"
#include "frsky_sport_io.h"
#include "UICallback.h"

#include <QObject>
#include <QString>
#include <QPointer>
#include <QIODevice>
#include <QTimer>
#include <QElapsedTimer>

// ============================================================================

class CFrskyDeviceFirmwareUpdate : public QObject
{
	Q_OBJECT

protected:
	// State-Machine:
	enum State {
		SPORT_IDLE,					// In idle condition, doing nothing yet
		SPORT_START,				// Start flashing sequence -- first state from flashDeviceFirmware
		SPORT_FLASHMODE_REQ,		// Send FlashMode request to device
		SPORT_FLASHMODE_ACK,		// Have received FlashMode ACK from device
		SPORT_VERSION_REQ,			// Send Version request to device
		SPORT_VERSION_ACK,			// Have received Version ACK from device
		SPORT_CMD_DOWNLOAD,			// Send Command Download to device
		SPORT_DATA_REQ,				// Have received Data Req from device
		SPORT_DATA_TRANSFER,		// Send Data Transfer to device
		SPORT_END_TRANSFER,			// End of transfer, no more packets to send, signaling device we're done (end of firmware file)
		SPORT_CRC_FAILURE,			// Device reported CRC Failure
		SPORT_COMPLETE,				// Final State for programming successful, if device accepts the end of transfer
		SPORT_FAIL					// Final State for programming failed, for any failure
	};

public:
	explicit CFrskyDeviceFirmwareUpdate(CFrskySportIO &frskySportIO, CUICallback *pUICallback = nullptr, QObject *pParent = nullptr);
	virtual ~CFrskyDeviceFirmwareUpdate();

	// Main: flashDeviceFirmware function:
	//		firmware = QIODevice to filestream of firmware file content
	//		bIsFRSKFile = If true, this is a ".frsk" file with the "frsk"
	//					file header that we need to skip.  If false, it's
	//					a "frk" that has no header.
	//		bBlocking : If true, this function won't return until flashing
	//					is complete and will return completion status bool.
	//					Else, if non-blocking, will return immediately with
	//					'true' and will emit a flashComplete signal with
	//					the status.  Note: if there's an immediate error in
	//					non-blocking mode, the return value will be 'false'
	//					in addition to emitting the signal.  And, the signal
	//					is also emitted even on blocking mode (for consistency).
	bool flashDeviceFirmware(QIODevice &firmware, bool bIsFRSKFile, bool bBlocking);

	QString getLastError() const { return m_strLastError; }

signals:
	void flashComplete(bool bSuccess);		// bSuccess True if completed successfully, else getLastError will have error message
	void writeLogString(const QString &strLogString);

	// Private:
	void dataAvailable();

protected slots:
	void en_timeout();
	void en_readyRead();
	void en_receive();

protected slots:
	void nextState();						// State machine drive logic

protected:
	void processFrame();					// Process the current frame in m_rxBuffer
	void waitState(State nNextState, uint32_t nTimeout, int nRetries);	// wait for specified state for nRetries, with nTimeout time between tries
	void sendFrame(const CSportFirmwarePacket &packet);	// Transmit frame with specified packet on bus

protected:
	State m_state = SPORT_IDLE;				// Current StateMachine state
	State m_nextState = SPORT_START;		// Next State Expected by StateMachine (initially, the next state expected is the start of flash programming)
	int m_nRetryCount = 0;					// Number of retries remaining for current state
	uint32_t m_nReqAddress = 0;				// Address in firmware file being requested by device
	uint32_t m_nFileAddress = 0;			// Address in firmware file being read from file
	uint32_t m_nVersionInfo = 0;			// Version information read from device
	QPointer<QIODevice> m_pFirmware;		// Current firmware file
	qint64 m_nFirmwareSize = 0;				// Size of firmware, used for size checking and for progress callbacks
	CSportRxBuffer m_rxBuffer;				// Receive Sport Packet buffer from serial en_receive events
	QTimer m_tmrEventTimeout;				// Current Event Timeout Timer, triggers for doing retries and state machine driving
	QElapsedTimer m_tmrLogFile;				// Log File event timer

	QString m_strLastError;					// Last error to report
	CFrskySportIO &m_frskySportIO;			// Serial Port handler for Sport I/O
	CUICallback *m_pUICallback = nullptr;	// Optional user interface callback
};

// ============================================================================

#endif	// FRSKY_SPORT_FIRMWARE_H
