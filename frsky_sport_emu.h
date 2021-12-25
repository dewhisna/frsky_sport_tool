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

#ifndef FRSKY_SPORT_EMU_H
#define FRSKY_SPORT_EMU_H

#include "frsky_sport_io.h"

#include <QObject>
#include <QString>
#include <QPointer>
#include <QIODevice>
#include <QByteArray>
#include <QFlags>
#include <QTimer>

// Forward Declarations
class CUICallback;

// ============================================================================

class CFrskySportDeviceEmu : public QObject
{
	Q_OBJECT

protected:
	static constexpr int PHYS_ID_POLL_COUNT = 28;		// Number of devices in poll list

	// State-Machine:
	enum State {
		SPORT_IDLE,					// In idle condition, doing nothing yet
		SPORT_POLL_DISC_MODE,		// Running Poll Discover Mode loop (finding new devices) -- first state from startDeviceEmulation when polling is on
		SPORT_POLL_SERV_MODE,		// Running Poll Service Mode loop (servicing known devices)
		SPORT_FLASHMODE_REQ,		// Waiting FlashMode request -- first state from startDeviceEmulation when polling is off
		SPORT_FLASHMODE_ACK,		// Send FlashMode ACK
		SPORT_VERSION_REQ,			// Waiting Version request
		SPORT_VERSION_ACK,			// Send Version ACK
		SPORT_USER_ABORT,			// User aborted emulation
		SPORT_CMD_DOWNLOAD,			// Received Command Download
		SPORT_CMD_UPLOAD,			// Received Command Upload
		SPORT_DATA_REQ,				// Send Data Req (programming-only), either after PRIM_CMD_DOWNLOAD or DATA_TRANSFER
		SPORT_DATA_AVAIL,			// Send Data Avail (reading-only), either after PRIM_CMD_UPLOAD or DATA_TRANSFER
		SPORT_DATA_TRANSFER,		// Received Data Transfer (i.e. after SPORT_DATA_REQ or SPORT_DATA_AVAIL is processed)
		SPORT_END_TRANSFER,			// Received End of transfer, no more packets to receive, signaling we're done (end of firmware file)
		SPORT_CRC_FAILURE,			// Reported CRC Failure
		SPORT_END_EMULATION,		// Emulation is complete
	};

public:
	// Bitflags for devices to emulate:
	enum FrskyDevices {
		FRSKDEV_NONE = 0,			// Default for no devices (serial port monitor only)
		FRSKDEV_RX = 1,				// Receiver Device
	};
	Q_DECLARE_FLAGS(FrskyDeviceFlags, FrskyDevices)

	static bool deviceIsReceiver(FrskyDeviceFlags nDevice) { return (nDevice & FRSKDEV_RX); }

	explicit CFrskySportDeviceEmu(CFrskySportIO &frskySportIO, CUICallback *pUICallback = nullptr, QObject *pParent = nullptr);
	virtual ~CFrskySportDeviceEmu();

	bool emulatorRunning() const
	{
		return ((m_state != SPORT_IDLE) && (m_state != SPORT_END_EMULATION));
	}

	bool inPollingMode() const
	{
		return ((m_state == SPORT_POLL_DISC_MODE) || (m_state == SPORT_POLL_SERV_MODE));
	}

	// startDeviceEmulation function:
	//		nDevices : Devices to emulate with this instance
	//		bBlocking : If true, this function won't return until emulation
	//					is complete and will return completion status bool.
	//					Else, if non-blocking, will return immediately with
	//					'true' and will emit a deviceEmulationComplete signal with
	//					the status.  Note: if there's an immediate error in
	//					non-blocking mode, the return value will be 'false'
	//					in addition to emitting the signal.  And, the signal
	//					is also emitted even on blocking mode (for consistency).
	//
	//		Note: If a firmware is set via setFirmware, that firmware will
	//					be compared if the tool under test sends a firmware.
	//					If not, firmware file comparison will be skipped.
	bool startDeviceEmulation(FrskyDeviceFlags nDevices, bool bBlocking);

	QString getLastError() const { return m_strLastError; }

	void setReceiverPolling(bool bPoll);
	bool getReceiverPolling() const { return m_bRxPoll; }

	uint32_t getVersionInfo() const { return m_nVersionInfo; }
	void setVersionInfo(uint32_t nVersionInfo) { m_nVersionInfo = nVersionInfo; }	// Sets version info to report during emulation

	// setFirmware:
	//		firmware = QIODevice of filestream to read firmware file content
	//					for comparison with external flashing operation
	//		bIsFRSKFile = If true, this is a ".frsk" file with the "frsk"
	//					file header that we need to skip.  If false, it's
	//					a "frk" that has no header.
	bool setFirmware(QIODevice &firmware, bool bIsFRSKFile);

	// getFirmware:
	//		returns only the content from the emulation process.  That is
	//			the incoming firmware from the tool is recorded and stored
	//			and made available here.  It does not reflect the firmware
	//			received unless we received the same firmware during
	//			device emulation:
	const QByteArray &getFirmware() const { return m_baRxFirmware; }

signals:
	void deviceEmulationComplete(bool bSuccess);		// bSuccess True if completed successfully, else getLastError will have error message
	void writeLogString(const QString &strLogString);					// Used for logging of received comm messages
	void emulationErrorEncountered(const QString &strErrorMessage);		// Used for logging/reporting emulation issues (such as requesting device sending wrong data or bad message)

	// Private:
	void dataAvailable();

public slots:
	void endEmulation();

protected slots:
	void en_readyRead();
	void en_receive();
	// ----
	void en_pollEvent();
	// ----
	void en_userCancel();

protected slots:
	void nextState();						// State machine drive logic

protected:
	struct FrameProcessResult				// Result from processFrame()
	{
		bool m_bAdvanceState = false;		// If true, then call nextState to advance state-machine
		QString m_strLogDetail;				// Additional log file detail to add to message being processed
	};
	FrameProcessResult processFrame();		// Process the current frame in m_rxBuffer
	void sendFirmwareFrame(const CSportFirmwarePacket &packet, const QString &strLogDetail = QString());	// Transmit frame with specified packet on bus
	void sendTelemetryFrame(const CSportTelemetryPacket &packet, const QString &strLogDetail = QString());	// Transmit frame with specified packet on bus
	bool compareFirmware() const;			// Compare received firmware against original firmware file expected

	void resetPollList();

	void emuError(const QString &strError)
	{
		m_strLastError = strError;
		emit emulationErrorEncountered(strError);
	}

protected:
	FrskyDeviceFlags m_nDevices = FRSKDEV_NONE;	// Devices being emulated
	// ----
	State m_state = SPORT_IDLE;				// Current StateMachine state
	bool m_bRxPoll = false;					// Receiver device Sport polling (if 'True', receiver emulate will send out Sport device polls)
	bool m_arrDeviceFound[PHYS_ID_POLL_COUNT] = {};		// Devices found during poll
	int m_nPollDiscDeviceIndex = -1;		// Index of device to discover poll (-1 if not started)
	int m_nPollServDeviceIndex = -1;		// Index of device to service poll (-1 if not started)
	// -----
	uint32_t m_nReqAddress = 0;				// Address in firmware file being requested/sent by device
	uint8_t m_arrDataRead[4];				// Data sent by device during flash read
	uint32_t m_nFileAddress = 0;			// Address in firmware file being read from file
	uint32_t m_nVersionInfo = 0;			// Version information read from device
	QPointer<QIODevice> m_pFirmware;		// Current firmware file
	qint64 m_nFirmwareSize = 0;				// Size of firmware, used for size checking and for progress callbacks
	bool m_bFirmwareRxMode = true;			// True if receiving firmware (flashing), False if sending firmware (reading)
	QByteArray m_baRxFirmware;				// Firmware received from bus
	bool m_bRxFirmwareError = false;		// Set to 'True' if there was a CRC error or size error in receiving the firmware -- used for final reponse to tool (complete or fail)
	CSportRxBuffer m_rxBuffer;				// Receive Sport Packet buffer from serial en_receive events
	QTimer m_tmrPollEvent;					// Receiver poll event timer

	QString m_strLastError;					// Last error to report
	CFrskySportIO &m_frskySportIO;			// Serial Port handler for Sport I/O
	CUICallback *m_pUICallback = nullptr;	// Optional user interface callback
};
Q_DECLARE_OPERATORS_FOR_FLAGS(CFrskySportDeviceEmu::FrskyDeviceFlags)

// ============================================================================

#endif	// FRSKY_SPORT_EMU_H
