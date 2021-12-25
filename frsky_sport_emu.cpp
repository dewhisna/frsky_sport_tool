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

#include "frsky_sport_emu.h"
#include "UICallback.h"

#include <QCoreApplication>

// ============================================================================

namespace {
	constexpr uint8_t PRIM_REQ_FLASHMODE =	0x00;		// Request enter firmware flash mode
	constexpr uint8_t PRIM_REQ_VERSION =	0x01;		// Request version check
	constexpr uint8_t PRIM_CMD_UPLOAD =		0x02;		// Command firmware upload ?? (not sure if this is legit or not -- we will experiment to find out)
	constexpr uint8_t PRIM_CMD_DOWNLOAD =	0x03;		// Command firmware download
	constexpr uint8_t PRIM_DATA_WORD =		0x04;		// Data Word Xfer
	constexpr uint8_t PRIM_DATA_EOF =		0x05;		// Data End-of-File

	constexpr uint8_t PRIM_ACK_FLASHMODE =	0x80;		// Confirm enter flash mode
	constexpr uint8_t PRIM_ACK_VERSION =	0x81;		// Version check response
	constexpr uint8_t PRIM_REQ_DATA_ADDR =	0x82;		// Request for specific data address (and data for upload?)
	constexpr uint8_t PRIM_END_DOWNLOAD =	0x83;		// End of download (and upload?)
	constexpr uint8_t PRIM_DATA_CRC_ERR =	0x84;		// CRC Error

	PACK(struct FrSkyFirmwareInformation {
		uint8_t fourcc[4];
		uint8_t headerVersion;
		uint8_t firmwareVersionMajor;
		uint8_t firmwareVersionMinor;
		uint8_t firmwareVersionRevision;
		uint32_t size;
		uint8_t productFamily;
		uint8_t productId;
		uint16_t crc;
	});
	static_assert((sizeof(FrSkyFirmwareInformation) == 16), "FrSkyFirmwareInformation structure sizing error");

	constexpr int FIRMWARE_BLOCK_SIZE = 1024;

	constexpr int SPORT_POLL_RATE = 12;			// Sport device poll rate in milliseconds

	// ------------------------------------------------------------------------

};

// ============================================================================

void CFrskySportDeviceEmu::nextState()
{
	static thread_local uint8_t arrFirmwareBlock[FIRMWARE_BLOCK_SIZE];		// Current firmware data block being sent

	switch (m_state) {
		case SPORT_IDLE:
			break;

		case SPORT_POLL_DISC_MODE:
			// TODO : Poll next device in discovery list
			break;

		case SPORT_POLL_SERV_MODE:
			// TODO : Poll next device in service list
			break;

		case SPORT_FLASHMODE_REQ:
			// Here, we are waiting for an incoming flash
			//	mode request -- this represents the device
			//	bootloader phase, only we aren't currently
			//	time limited in this state on the emulator
			//	so here's nothing to do here.  If we ever
			//	wanted to emulate the startup timing of a
			//	device, we could limit the time in this state
			//	with a timeout that transitions us back to
			//	idle or poll mode (if we are a receiver)
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("Waiting for FlashMode Request..."));
			}
			break;

		case SPORT_FLASHMODE_ACK:
			// Send response for FlashMode Request
			m_state = SPORT_VERSION_REQ;
			sendFirmwareFrame(CSportFirmwarePacket(PRIM_ACK_FLASHMODE, (uint32_t)0, 0, true), tr("Flash Mode ACK"));
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("Received FlashMode Request, Sending Acknowledge..."));
			}
			break;

		case SPORT_VERSION_REQ:
			// Here, we are waiting for an incoming version
			//	request after acknowledging flash mode.
			//	There's nothing to do here.
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("Waiting for Version Request or Download/Upload Start..."));
			}
			break;

		case SPORT_VERSION_ACK:
			// Send response for Version Request, but halt
			//	in this state until we get a command to
			//	upload or download
			sendFirmwareFrame(CSportFirmwarePacket(PRIM_ACK_VERSION, m_nVersionInfo, 0, true), tr("Version ACK : Version=0x%1").arg(m_nVersionInfo, 8, 16, QChar('0')));
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("Received Version Request, Sending Version and Waiting for Download/Upload Start..."));
			}
			break;

		case SPORT_USER_ABORT:
			emuError(tr("User aborted device emulation"));
			m_state = SPORT_END_EMULATION;
			emit deviceEmulationComplete(false);
			break;

		case SPORT_CMD_DOWNLOAD:
			// Here when we received the command to enter download
			//	mode and start receiving data.  We must setup addressing
			//	and send a PRIM_REQ_DATA_ADDR to request data for it:
			m_nReqAddress = 0;
			m_nFileAddress = 0;
			m_bFirmwareRxMode = true;				// Download/Flashing mode
			m_baRxFirmware.clear();
			m_state = SPORT_DATA_TRANSFER;
			sendFirmwareFrame(CSportFirmwarePacket(PRIM_REQ_DATA_ADDR, m_nReqAddress, 0, true), tr("Req Data, Addr=0x%1").arg(m_nReqAddress, 8, 16, QChar('0')));
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("Received Download Command, Sending Data Request..."));
			}
			break;

		case SPORT_CMD_UPLOAD:
		case SPORT_DATA_AVAIL:
			// Here on request to upload.  This looks a bit like the
			//	tool's sending of data on a download process.  It is
			//	congruous between initial CMD_UPLOAD and DATA_AVAIL:
			assert(!m_pFirmware.isNull());			// processFrame should not have advanced this state if we don't have firmware
			if (m_nReqAddress == m_nFileAddress) {		// See if we need to read more data from firmware file
				if (m_pFirmware->atEnd()) {
					assert(m_nFileAddress == m_nFirmwareSize);
					m_state = SPORT_END_EMULATION;
					sendFirmwareFrame(CSportFirmwarePacket(PRIM_END_DOWNLOAD, (uint32_t)0, 0, true), tr("End Download"));
					break;
				}
				int nBlockSize = m_pFirmware->read((char*)arrFirmwareBlock, sizeof(arrFirmwareBlock));
				if (nBlockSize < 0) {
					emuError(tr("Error reading firmware file"));
					m_state = SPORT_END_EMULATION;
					emit deviceEmulationComplete(false);
					return;
				}
				m_nFileAddress += nBlockSize;
			}
			m_state = SPORT_DATA_TRANSFER;
			sendFirmwareFrame(CSportFirmwarePacket(PRIM_REQ_DATA_ADDR, &arrFirmwareBlock[m_nReqAddress & 0x3FF],
						m_nReqAddress & 0xFF, true), tr("Data Xfer: %1.%2.%3.%4")
						.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+0], 2, 16, QChar('0'))
						.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+1], 2, 16, QChar('0'))
						.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+2], 2, 16, QChar('0'))
						.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+3], 2, 16, QChar('0')));
			break;

		case SPORT_DATA_REQ:
			// Here when we receive a data packet from tool to store
			m_baRxFirmware.append((char*)m_arrDataRead, sizeof(m_arrDataRead));
			m_nReqAddress += 4;
			m_nFileAddress += 4;
			m_state = SPORT_DATA_TRANSFER;
			sendFirmwareFrame(CSportFirmwarePacket(PRIM_REQ_DATA_ADDR, m_nReqAddress, 0, true), tr("Req Data, Addr=0x%1").arg(m_nReqAddress, 8, 16, QChar('0')));
			break;

		case SPORT_DATA_TRANSFER:
			// Here when we are in our receive loop.  This
			//	event should never happen as it will get promoted
			//	to either SPORT_DATA_REQ or SPORT_DATA_AVAIL
			assert(false);
			break;

		case SPORT_END_TRANSFER:
			// Here when tool has finished sending (or receiving?) data
			//	and we must send our end download primitive, and we're done
			sendFirmwareFrame(CSportFirmwarePacket(PRIM_END_DOWNLOAD, (uint32_t)0, 0, true), tr("End Download"));
			m_state = SPORT_END_EMULATION;
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("Received Data EOF Message with Good Data..."));
			}
			break;

		case SPORT_CRC_FAILURE:
			// Here if receiving firmware and it was invalid.  Report
			//	CRC Error primitive and we're done:
			sendFirmwareFrame(CSportFirmwarePacket(PRIM_DATA_CRC_ERR, (uint32_t)0, 0, true), tr("Data Error Response"));
			m_state = SPORT_END_EMULATION;
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("Received Data EOF Message with Bad Data..."));
			}
			break;

		case SPORT_END_EMULATION:
			// What does the real device do when firmware transfer has
			//	finished??  Do we enter poll mode?  Or just sit here?  Or what?
			//	TODO : we need to observe real devices and see if they enter
			//	poll mode.  If so, we can do that here.  Otherwise, this will
			//	automatically exit emulation mode and return blocking emulation
			//	back to the caller to decide...
			if (m_pUICallback) {
				m_pUICallback->setProgressText(tr("End of Emulation..."));
			}
			break;
	}
}

CFrskySportDeviceEmu::FrameProcessResult CFrskySportDeviceEmu::processFrame()
{
	assert(m_rxBuffer.haveCompletePacket());
	FrameProcessResult results;
	results.m_bAdvanceState = false;

	if (m_rxBuffer.isFirmwarePacket()) {
		if ((m_rxBuffer.firmwarePacket().m_physicalId == PHYS_ID_FIRMCMD) &&
			(m_rxBuffer.firmwarePacket().m_primId == PRIM_ID_FIRMWARE_FRAME) &&
			(!getReceiverPolling())) {
			switch (m_rxBuffer.firmwarePacket().m_cmd) {
				case PRIM_REQ_FLASHMODE:			// Request to start flash mode
					results.m_strLogDetail = tr("Request Flash Mode");
					if (m_state == SPORT_FLASHMODE_REQ) {	// Can only be requested immediately at startup
						m_state = SPORT_FLASHMODE_ACK;
						results.m_bAdvanceState = true;
					} else {
						emuError(tr("FlashMode can only be requested at Rx Device Startup"));
						results.m_strLogDetail += tr("  *** Ignoring: Can only be requested at Rx Device Startup");
					}
					break;

				case PRIM_REQ_VERSION:				// Request to send Version Info
					results.m_strLogDetail = tr("Request Version");
					if (m_state == SPORT_VERSION_REQ) {		// Can only be requested after entering flash mode
						m_state = SPORT_VERSION_ACK;
						results.m_bAdvanceState = true;
					} else {
						results.m_strLogDetail += tr("  *** Ignoring");
						// For some reason, the real Rx will only respond once to VersionInfo.
						//	Ignore duplicate requests and don't resend it:
						if (m_state != SPORT_VERSION_ACK) {
							emuError(tr("VersionInfo can only be requested after entering flash mode"));
							results.m_strLogDetail += tr(": Can only be requested after entering flash mode");
						}
					}
					break;

				case PRIM_CMD_UPLOAD:				// Command upload mode ??
					results.m_strLogDetail = tr("Command Upload??");
					if ((m_state == SPORT_VERSION_REQ) ||		// Can this be requested after FlashMode only? without VersionInfo Req?
						(m_state == SPORT_VERSION_ACK) ||
						((m_state == SPORT_DATA_TRANSFER) && (!m_bFirmwareRxMode))) {	// TODO : Determine if PRIM_CMD_UPLOAD is used for Reading or if PRIM_DATA_ADDR is used
						m_nReqAddress = m_rxBuffer.firmwarePacket().dataValue();		// Is this really the address?
						m_bFirmwareRxMode = false;				// Upload/Reading mode
						results.m_strLogDetail += QString("  Addr: 0x%1 ?").arg(m_nReqAddress, 8, 16, QChar('0'));
						if (m_pFirmware.isNull()) {
							emuError(tr("Upload Command without emulator firmware file content"));
							results.m_strLogDetail += tr("  *** Fail: Don't have firmware file content");
							m_state = SPORT_CRC_FAILURE;		// If upload is requested without a firmware, report error.  Is this the best error mechanism?
						} else {
							m_state = SPORT_CMD_UPLOAD;
						}
						results.m_bAdvanceState = true;
					} else {
						emuError(tr("Upload Command can only be accepted after entering flash mode"));
						results.m_strLogDetail += tr("  *** Ignoring: Can only be requested after entering flash mode");
					}
					break;

				case PRIM_CMD_DOWNLOAD:				// Command download mode
					results.m_strLogDetail = tr("Command Download");
					if ((m_state == SPORT_VERSION_REQ) ||		// Can this be requested after FlashMode only? without VersionInfo Req?
						(m_state == SPORT_VERSION_ACK)) {
						m_state = SPORT_CMD_DOWNLOAD;
						results.m_bAdvanceState = true;
					} else {
						emuError(tr("Download Command can only be accepted after entering flash mode"));
						results.m_strLogDetail += tr("  *** Ignoring: Can only be requested after entering flash mode");
					}
					break;

				case PRIM_DATA_WORD:				// Receive Data Word Xfer
					results.m_strLogDetail = tr("Data Xfer");
					if (m_state == SPORT_DATA_TRANSFER) {		// Data Transfer can only happen after Cmd Upload or Cmd Download has completed
						if (m_bFirmwareRxMode) {
							m_arrDataRead[0] = m_rxBuffer.firmwarePacket().m_data[0];
							m_arrDataRead[1] = m_rxBuffer.firmwarePacket().m_data[1];
							m_arrDataRead[2] = m_rxBuffer.firmwarePacket().m_data[2];
							m_arrDataRead[3] = m_rxBuffer.firmwarePacket().m_data[3];
							results.m_strLogDetail += QString(": %1.%2.%3.%4")
									.arg(m_arrDataRead[0], 2, 16, QChar('0'))
									.arg(m_arrDataRead[1], 2, 16, QChar('0'))
									.arg(m_arrDataRead[2], 2, 16, QChar('0'))
									.arg(m_arrDataRead[3], 2, 16, QChar('0'));
							m_state = SPORT_DATA_REQ;
						} else {
							m_nReqAddress = m_rxBuffer.firmwarePacket().dataValue();
							results.m_strLogDetail += QString(" Addr: 0x%1 ?").arg(m_nReqAddress, 8, 16, QChar('0'));
							m_state = SPORT_DATA_AVAIL;
						}
						results.m_bAdvanceState = true;
					} else {
						emuError(tr("Data Transfer can only happen after CMD Upload or CMD Download"));
						results.m_strLogDetail += tr("   *** Ignoring: Can only happen after CMD Upload/Download");
					}
					break;

				case PRIM_DATA_EOF:					// Data End-of-File
					results.m_strLogDetail = tr("Data EOF");
					if (m_state == SPORT_DATA_TRANSFER) {		// Data EOF can only happen during data xfer
						if (m_bFirmwareRxMode) {
							// If receiving the firmware, compare against real firmware
							m_state = compareFirmware() ? SPORT_END_TRANSFER : SPORT_CRC_FAILURE;
							results.m_bAdvanceState = true;
						} else {
							m_state = SPORT_END_TRANSFER;		// Will this mode happen on upload?
							results.m_bAdvanceState = true;
						}
					} else {
						emuError(tr("Data EOF received without Data Transfer"));
						results.m_strLogDetail += tr("  *** Ignoring: Data EOF received without Data Transfer");
					}
					break;

				default:
					results.m_strLogDetail = tr("*** Unexpected/Unknown Firmware Packet");
					break;
			}
		} else {
			if (deviceIsReceiver(m_nDevices) && getReceiverPolling() &&
				(m_rxBuffer.firmwarePacket().m_physicalId == PHYS_ID_FIRMCMD)) {
				emuError(tr("Firmware packet received when Rx in polling mode"));
				results.m_strLogDetail = tr("Firmware packet received in polling mode, Ignoring");
			}
			// Ignore others, as they are probably just our echos
		}
	} else if (m_rxBuffer.isTelemetryPacket()) {
		// TODO : Handle Telemetry Packet I/O emulation
	} else {
		results.m_strLogDetail = tr("*** Unexpected/Unknown Firmware Packet");
	}

	return results;
}

void CFrskySportDeviceEmu::waitState(State nNextState, uint32_t nTimeout, int nRetries)
{
}

void CFrskySportDeviceEmu::sendFirmwareFrame(const CSportFirmwarePacket &packet, const QString &strLogDetail)
{
	CSportTxBuffer frameReqFlashMode;
	frameReqFlashMode.pushFirmwarePacketWithByteStuffing(packet);

	QByteArray arrBytes;
	arrBytes.append(0x7E);			// Start of Frame
	arrBytes.append(frameReqFlashMode.data());
	m_frskySportIO.logMessage(CFrskySportIO::LT_TX, arrBytes, strLogDetail);
	m_frskySportIO.port().write(arrBytes);
	m_frskySportIO.port().flush();
}

void CFrskySportDeviceEmu::sendTelemetryFrame(const CSportTelemetryPacket &packet, const QString &strLogDetail)
{
	CSportTxBuffer frameReqFlashMode;
	frameReqFlashMode.pushTelemetryPacketWithByteStuffing(packet);

	QByteArray arrBytes;
	arrBytes.append(0x7E);			// Start of Frame
	arrBytes.append(frameReqFlashMode.data());
	m_frskySportIO.logMessage(CFrskySportIO::LT_TX, arrBytes, strLogDetail);
	m_frskySportIO.port().write(arrBytes);
	m_frskySportIO.port().flush();
}

bool CFrskySportDeviceEmu::compareFirmware() const
{
	// TODO : Complete

	return true;
}

void CFrskySportDeviceEmu::resetPollList()
{
	for (int i = 0; i < PHYS_ID_POLL_COUNT; ++i) {
		m_arrDeviceFound[i] = false;
	}
	m_nPollDiscDeviceIndex = -1;
	m_nPollServDeviceIndex = -1;
}

void CFrskySportDeviceEmu::en_userCancel()
{
	m_state = SPORT_USER_ABORT;
	nextState();
}

void CFrskySportDeviceEmu::endEmulation()
{
	m_state = SPORT_END_EMULATION;
	nextState();
}

// ----------------------------------------------------------------------------

// This function gets triggered whenever the serial port
//	has data.  Here, we emit a queued connection function
//	that handles the read.  We do that rather than read
//	the data here to avoid a race condition between reading
//	the data and getting another event from the serial device.
void CFrskySportDeviceEmu::en_readyRead()
{
	emit dataAvailable();
}

// This function gets triggered by the readyRead queued
//	connection with dataAvailable.  Here, we check to see if we
//	have incoming bytes from the serial connection, which might
//	be bytes from the device or bytes we are sending being
//	echoed since it's a half-duplex, single-wire connection,
//	and then if it's a message from the device, we post an
//	event to process them:
void CFrskySportDeviceEmu::en_receive()
{
	QByteArray arrBytes = m_frskySportIO.port().readAll();
	if (!arrBytes.isEmpty()) {
		for (int ndx = 0; ndx < arrBytes.size(); ++ndx) {
			QByteArray baExtraneous = m_rxBuffer.pushByte(arrBytes.at(ndx));
			if (!baExtraneous.isEmpty()) {
				m_frskySportIO.logMessage(CFrskySportIO::LT_RX, baExtraneous, "*** Extraneous Bytes");
			}
			if (m_rxBuffer.haveCompletePacket()) {
				if (!m_rxBuffer.isFirmwarePacket() && !m_rxBuffer.isTelemetryPacket()) {
					QByteArray baUnexpected(1, 0x7E);		// Add the 0x7E since it's eaten by the RxBuffer
					baUnexpected.append(m_rxBuffer.rawData());
					m_frskySportIO.logMessage(CFrskySportIO::LT_RX, baUnexpected, "*** Unexpected/Unknown packet");
				} else {
					uint8_t nExpectedCRC = m_rxBuffer.isFirmwarePacket() ? m_rxBuffer.firmwarePacket().crc() :
															m_rxBuffer.telemetryPacket().crc();
					bool bIsEcho = m_rxBuffer.isFirmwarePacket() && (m_rxBuffer.firmwarePacket().m_physicalId == PHYS_ID_FIRMRSP);

					FrameProcessResult procResults = processFrame();		// Process all packets

					if (CPersistentSettings::instance()->getFirmwareLogTxEchos() ||
						(!CPersistentSettings::instance()->getFirmwareLogTxEchos() && !bIsEcho) ||
						(nExpectedCRC != m_rxBuffer.crc()) ||
						!procResults.m_strLogDetail.isEmpty()) {
						QByteArray baMessage(1, 0x7E);		// Add the 0x7E since it's eaten by the RxBuffer
						baMessage.append(m_rxBuffer.rawData());
						QString strExtraMessage = procResults.m_strLogDetail;
						if (nExpectedCRC != m_rxBuffer.crc()) {
							QString strCRCError = QString("*** Expected CRC of 0x%1, Received CRC of 0x%2")
										.arg(QString("%1").arg(nExpectedCRC, 2, 16, QChar('0')).toUpper(),
											QString("%1").arg(m_rxBuffer.crc(), 2, 16, QChar('0')).toUpper());
							if (!strExtraMessage.isEmpty()) strExtraMessage += "  ";
							strExtraMessage += strCRCError;
						}
						m_frskySportIO.logMessage(bIsEcho ? CFrskySportIO::LT_TXECHO : CFrskySportIO::LT_RX, baMessage, strExtraMessage);
					}

					if (procResults.m_bAdvanceState) nextState();
				}

				m_rxBuffer.reset();
			}
		}
	}
}

void CFrskySportDeviceEmu::en_pollEvent()
{
	if (!emulatorRunning()) return;

	// TODO : Finish
}

void CFrskySportDeviceEmu::setReceiverPolling(bool bPoll)
{
	resetPollList();		// Changing poll mode always clears lists of devices found in previous polls

	if (bPoll) {
		if (m_bRxPoll && emulatorRunning() && !inPollingMode()) {
			emuError(tr("Polling started -- Flash state machine aborted"));
			m_state = SPORT_POLL_DISC_MODE;
			nextState();
		}
		m_tmrPollEvent.start(SPORT_POLL_RATE);
	} else {
		m_tmrPollEvent.stop();
		if (m_bRxPoll && inPollingMode()) {
			// Stopping polling when in poll mode, switches us to idle mode
			//	and stops the emulator, but doesn't affect flash mode, where
			//	it's a no-op.  This is different than SPORT_END_EMULATION.
			m_state = SPORT_IDLE;
			nextState();
		}
	}
	m_bRxPoll = bPoll;
}

// ============================================================================


CFrskySportDeviceEmu::CFrskySportDeviceEmu(CFrskySportIO &frskySportIO, CUICallback *pUICallback, QObject *pParent)
	:	QObject(pParent),
		m_frskySportIO(frskySportIO),
		m_pUICallback(pUICallback)
{
	connect(&m_frskySportIO.port(), SIGNAL(readyRead()), this, SLOT(en_readyRead()));
	connect(this, SIGNAL(dataAvailable()), this, SLOT(en_receive()), Qt::QueuedConnection);

	connect(&m_tmrPollEvent, SIGNAL(timeout()), this, SLOT(en_pollEvent()));
	if (pUICallback) {
		pUICallback->hookCancel(this, SLOT(endEmulation()));
	}

	if (m_bRxPoll) {
		setReceiverPolling(true);
	}
}

CFrskySportDeviceEmu::~CFrskySportDeviceEmu()
{
}

// ----------------------------------------------------------------------------

bool CFrskySportDeviceEmu::setFirmware(QIODevice &firmware, bool bIsFRSKFile)
{
	if (emulatorRunning()) {
		emuError(tr("Can't change firmware file while emulator is running"));
		return false;
	}

	m_pFirmware = &firmware;
	m_nFirmwareSize = 0;
	m_nFileAddress = 0;

	if (firmware.isOpen() && firmware.isReadable()) {
		m_nFirmwareSize = firmware.size();		// So that this will work with random-access files and serial streams too, read size once, since streams is bytesAvailable, not overall size

		if ((m_nFirmwareSize == 0) ||
			(bIsFRSKFile && (static_cast<size_t>(m_nFirmwareSize) <= sizeof(FrSkyFirmwareInformation)))) {
			emuError(tr("File is Empty"));
			return false;
		}

		// FRSK file will have FrSkyFirmwareInformation header:
		if (bIsFRSKFile) {
			FrSkyFirmwareInformation header;
			int nReadSize = firmware.read((char *)&header, sizeof(header));
			if ((nReadSize < 0) ||
				(static_cast<size_t>(nReadSize) != sizeof(FrSkyFirmwareInformation))) {
				emuError(tr("Failed to Read Firmware Header from File"));
				return false;
			}
			if ((header.headerVersion != 1) ||
				(header.fourcc[0] != 'F') ||
				(header.fourcc[1] != 'R') ||
				(header.fourcc[2] != 'S') ||
				(header.fourcc[3] != 'K')) {
				emuError(tr("Wrong .frsk file format"));
				return false;
			}

			if (m_nFirmwareSize != (sizeof(FrSkyFirmwareInformation) + header.size)) {
				emuError(tr("Wrong firmware file size"));
				return false;
			}

			m_nFirmwareSize -= sizeof(FrSkyFirmwareInformation);
		}
	}

	return true;
}

bool CFrskySportDeviceEmu::startDeviceEmulation(FrskyDeviceFlags nDevices, bool bBlocking)
{
	assert(!emulatorRunning());
	if (emulatorRunning()) {
		emuError(tr("Can't start emulator while emulator is already running"));
		return false;
	}

	assert(nDevices != FRSKDEV_NONE);
	if (nDevices == FRSKDEV_NONE) {
		emuError(tr("Must specify one or more devices to emulate"));
		return false;
	}

	// Reset the the state-machine, in case this function gets
	//	called again without creating a new object:
	m_nDevices = nDevices;
	if (deviceIsReceiver(nDevices)) resetPollList();
	m_state = SPORT_IDLE;
	m_nReqAddress = 0;
	m_nFileAddress = 0;
	m_nVersionInfo = 0;
	m_bFirmwareRxMode = true;

	m_baRxFirmware.clear();
	m_bRxFirmwareError = false;
	m_strLastError.clear();

	m_state = (getReceiverPolling() && deviceIsReceiver(nDevices)) ? SPORT_POLL_DISC_MODE : SPORT_FLASHMODE_REQ;
	nextState();		// Start emulation state-machine

	if (bBlocking) {
		while (m_state != SPORT_END_EMULATION) {
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
			QCoreApplication::sendPostedEvents();
		}
	}

	return true;
}

// ============================================================================
