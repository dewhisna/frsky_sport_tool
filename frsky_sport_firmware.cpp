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

#include "frsky_sport_firmware.h"
#include "UICallback.h"

#include "crc.h"

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
		uint16_t crc;							// This CRC is CRC-1021 format
	});
	static_assert((sizeof(FrSkyFirmwareInformation) == 16), "FrSkyFirmwareInformation structure sizing error");

	constexpr int FIRMWARE_BLOCK_SIZE = 1024;

	// ------------------------------------------------------------------------

	struct FirmwareDescriptor {
		int m_nID;
		QString m_strDescription;
	};

	static const FirmwareDescriptor conarrProductFamily[] =
	{
		{ FIRMWARE_FAMILY_INTERNAL_MODULE, "Internal Module" },
		{ FIRMWARE_FAMILY_EXTERNAL_MODULE, "External Module" },
		{ FIRMWARE_FAMILY_RECEIVER, "Receiver" },
		{ FIRMWARE_FAMILY_SENSOR, "Sensor" },
		{ FIRMWARE_FAMILY_BLUETOOTH_CHIP, "Bluetooth Chip" },
		{ FIRMWARE_FAMILY_POWER_MANAGEMENT_UNIT, "Power Management Unit" },
		{ FIRMWARE_FAMILY_FLIGHT_CONTROLLER, "Flight Controller" },
		{ -1, "Unknown" }
	};

	static const FirmwareDescriptor conarrModuleProductID[] =
	{
		{ FIRMWARE_ID_MODULE_NONE, "None" },
		{ FIRMWARE_ID_MODULE_XJT, "XJT" },
		{ FIRMWARE_ID_MODULE_ISRM, "ISRM" },
		{ -1, "Unknown" }
	};

	static const FirmwareDescriptor conarrReceiverProductID[] =
	{
		{ FIRMWARE_ID_RECEIVER_NONE, "None" },
		{ FIRMWARE_ID_RECEIVER_X8R, "X8R" },
		{ FIRMWARE_ID_RECEIVER_RX8R, "RX8R" },
		{ FIRMWARE_ID_RECEIVER_RX8R_PRO, "RX8R-Pro" },
		{ FIRMWARE_ID_RECEIVER_RX6R, "RX6R" },
		{ FIRMWARE_ID_RECEIVER_RX4R, "RX4R" },
		{ FIRMWARE_ID_RECEIVER_G_RX8, "G-RX8" },
		{ FIRMWARE_ID_RECEIVER_G_RX6, "G-RX6" },
		{ FIRMWARE_ID_RECEIVER_X6R, "X6R" },
		{ FIRMWARE_ID_RECEIVER_X4R, "X4R" },
		{ FIRMWARE_ID_RECEIVER_X4R_SB, "X4R-SB" },
		{ FIRMWARE_ID_RECEIVER_XSR, "XSR" },
		{ FIRMWARE_ID_RECEIVER_XSR_M, "XSR-M" },
		{ FIRMWARE_ID_RECEIVER_RXSR, "RXSR" },
		{ FIRMWARE_ID_RECEIVER_S6R, "S6R" },
		{ FIRMWARE_ID_RECEIVER_S8R, "S8R" },
		{ FIRMWARE_ID_RECEIVER_XM, "XM" },
		{ FIRMWARE_ID_RECEIVER_XMP, "XM+" },
		{ FIRMWARE_ID_RECEIVER_XMR, "XMR" },
		{ FIRMWARE_ID_RECEIVER_R9, "R9" },
		{ FIRMWARE_ID_RECEIVER_R9_SLIM, "R9-Slim" },
		{ FIRMWARE_ID_RECEIVER_R9_SLIMP, "R9-Slim+" },
		{ FIRMWARE_ID_RECEIVER_R9_MINI, "R9-Mini" },
		{ FIRMWARE_ID_RECEIVER_R9_MM, "R9-MM" },
		{ FIRMWARE_ID_RECEIVER_R9_STAB, "R9-STAB OTA" },
		{ FIRMWARE_ID_RECEIVER_R9_MINI_OTA, "R9-Mini OTA" },
		{ FIRMWARE_ID_RECEIVER_R9_MM_OTA, "R9-MM OTA" },
		{ FIRMWARE_ID_RECEIVER_R9_SLIMP_OTA, "R9-Slim+ OTA" },
		{ FIRMWARE_ID_RECEIVER_ARCHER_X, "Archer X OTA" },
		{ FIRMWARE_ID_RECEIVER_R9MX, "R9MX OTA" },
		{ FIRMWARE_ID_RECEIVER_R9SX, "R9SX OTA" },
		{ -1, "Unknown" }
	};
};


// ============================================================================

void CFrskyDeviceFirmwareUpdate::nextState()
{
	static thread_local uint8_t arrFirmwareBlock[FIRMWARE_BLOCK_SIZE];		// Current firmware data block being sent

	m_tmrEventTimeout.stop();		// Halt our retry timer until we determine we are in a state that needs retry processing
	bool bIsWaitState = (m_nextState == m_state);	// True if this was the state we were waiting for, False if it's a retry on this same state

	if (bIsWaitState || (m_nRetryCount != 0)) {
		switch (m_state) {
			case SPORT_IDLE:
				break;

			case SPORT_START:
				assert(bIsWaitState);			// This should never be a retry
				if (m_pUICallback) {
					m_pUICallback->setProgressRange(0, 0);	// Non-deterministic mode
					m_pUICallback->enableCancel(true);		// Let user halt the device finding mode
				}
				if ((m_runmode == FSM_RM_DEVICE_ID) ||
					((m_runmode == FSM_RM_FLASH_PROGRAM) && !m_pFirmware.isNull() && m_pFirmware->isOpen() && m_pFirmware->isReadable()) ||
					((m_runmode == FSM_RM_FLASH_READ) && !m_pFirmware.isNull() && m_pFirmware->isOpen() && m_pFirmware->isWritable())) {
					if (m_pUICallback) {
						m_pUICallback->setProgressText(tr("Finding Device..."));
					}
					m_state = SPORT_FLASHMODE_REQ;
					m_nextState = SPORT_FLASHMODE_REQ;
					QTimer::singleShot(50, this, SLOT(nextState()));	// 50ms delay then start with sendReqFlashMode
				} else {
					m_strLastError = tr("No firmware file");
					m_state = SPORT_FAIL;
					emit flashComplete(false);
					return;
				}
				break;

			case SPORT_FLASHMODE_REQ:			// This starts the flashing logic and is initiated by flashDeviceFirmware()
				if (bIsWaitState) {
					waitState(SPORT_FLASHMODE_ACK, 100, 300);		// Send up to 300 times, waiting 100msec each
				}
				sendFrame(CSportFirmwarePacket(PRIM_REQ_FLASHMODE), tr("Request Flash Mode"));
				break;

			case SPORT_FLASHMODE_ACK:
				assert(bIsWaitState);			// This should never be a retry
				if (m_pUICallback) {
					m_pUICallback->setProgressText(tr("Get Version Info"));
				}
				m_state = SPORT_VERSION_REQ;
				m_nextState = SPORT_VERSION_REQ;
				QTimer::singleShot(20, this, SLOT(nextState()));	// 20ms delay then start with sendReqVersion
				break;

			case SPORT_VERSION_REQ:
				if (bIsWaitState) {
					waitState(SPORT_VERSION_ACK, 100, 10);			// Send up to 10 times, waiting 100msec each
				}
				sendFrame(CSportFirmwarePacket(PRIM_REQ_VERSION), tr("Request Version"));
				break;

			case SPORT_VERSION_ACK:
				assert(bIsWaitState);			// This should never be a retry
				if (m_pUICallback) {
					// Display Version Information to user and
					//	prompt for continuation:
					QString strPrompt = QString::asprintf("Firmware V%2.2x.%2.2x, Hardware V%2.2x.%2.2x",
										((m_nVersionInfo >> 24) & 0xFF),
										((m_nVersionInfo >> 16) & 0xFF),
										((m_nVersionInfo >> 8) & 0xFF),
										(m_nVersionInfo & 0xFF));
					if (!m_pUICallback->isInteractive()) {
						m_pUICallback->setProgressText(strPrompt);
					}
					if ((m_runmode == FSM_RM_FLASH_PROGRAM) ||
						(m_runmode == FSM_RM_FLASH_READ)) {
						if (m_pUICallback->isInteractive()) {
							if (m_runmode == FSM_RM_FLASH_PROGRAM) {
								strPrompt += "\nContinue with flash programming?";
							} else {
								strPrompt += "\nContinue with flash reading?";
							}
							int nResp = m_pUICallback->promptUser(CUICallback::PT_QUESTION, strPrompt,
												CUICallback::Yes | CUICallback::No, CUICallback::Yes);
							if (nResp == CUICallback::No) {
								if (m_runmode == FSM_RM_FLASH_PROGRAM) {
									m_strLastError = tr("User aborted programming");
								} else {
									m_strLastError = tr("User aborted reading");
								}
								m_state = SPORT_FAIL;
								emit flashComplete(false);
								return;
							}
						}

						if (m_runmode == FSM_RM_FLASH_PROGRAM) {
							m_pUICallback->setProgressText(tr("Programming in progress..."));
						} else {
							m_pUICallback->setProgressText(tr("Reading in progress..."));
						}
					} else if (m_runmode == FSM_RM_DEVICE_ID) {
						if (m_pUICallback->isInteractive()) {
							m_pUICallback->promptUser(CUICallback::PT_INFORMATION, strPrompt,
												CUICallback::Ok, CUICallback::Ok);
						}
					}
				}

				switch (m_runmode) {
					case FSM_RM_DEVICE_ID:
						m_state = SPORT_COMPLETE;			// Done if we are only doing ID
						m_nextState = SPORT_COMPLETE;
						break;
					case FSM_RM_FLASH_PROGRAM:
						m_state = SPORT_CMD_DOWNLOAD;		// Start Download if we are programming
						m_nextState = SPORT_CMD_DOWNLOAD;
						break;
					case FSM_RM_FLASH_READ:
						m_state = SPORT_CMD_UPLOAD;			// Start Upload if we are reading
						m_nextState = SPORT_CMD_UPLOAD;
						break;
				}

				QTimer::singleShot(200, this, SLOT(nextState()));	// 200ms delay then command download
				break;

			case SPORT_USER_ABORT:
				assert(bIsWaitState);			// This should never be a retry and is a forced condition
				// Note: This should only ever happen on device search and/or version reading
				m_strLastError = tr("User aborted device search/version read");
				m_state = SPORT_FAIL;
				emit flashComplete(false);
				break;

			case SPORT_CMD_DOWNLOAD:
				assert(bIsWaitState);			// This should never be a retry since we are only sending it once.  A retry will trigger an error already
				if (m_pUICallback) {
					m_pUICallback->enableCancel(false);		// Once we start programming, don't let user break things by interrupting us

					// Switch to deterministic mode (if we have m_nFirmwareSize):
					m_pUICallback->setProgressRange(0, (m_nFirmwareSize/FIRMWARE_BLOCK_SIZE) +
													((m_nFirmwareSize % FIRMWARE_BLOCK_SIZE) ?  1 : 0));
					m_pUICallback->setProgressPos(0);
				}
				waitState(SPORT_DATA_REQ, 2000, 1);		// Send only once, waiting up to 2sec
				sendFrame(CSportFirmwarePacket(PRIM_CMD_DOWNLOAD), tr("Command Download"));
				break;

			case SPORT_CMD_UPLOAD:
				assert(bIsWaitState);			// This should never be a retry since we are only sending it once.  A retry will trigger an error already
				// Note: Leave cancel enabled here, since halting an
				//	upload shouldn't break the device (assuming the
				//	device actually supports upload).  Also, upload
				//	mode is completely experimental and we may need
				//	to abort if something goes wrong.
				waitState(SPORT_DATA_REQ, 2000, 1);		// Send only once, waiting up to 2sec
				sendFrame(CSportFirmwarePacket(PRIM_CMD_UPLOAD, m_nReqAddress), tr("Command Upload??,  Addr: 0x%1 ?").arg(m_nReqAddress, 8, 16, QChar('0')));		// Should this include the address or not??
				break;

			case SPORT_DATA_REQ:
				// I don't like this part of the algorithm from the opentx code.  Though,
				//	it looks like the opentx code mirrors that of the "official" FrSky
				//	code and is general weirdness of the FrSky protocol itself.
				//	First, it makes the very bad assumption that the addresses requested
				//	are always within the read data block and that they are always
				//	linearly increasing and that no value will be re-requested.  They
				//	start their loop through the block and never check to make sure it
				//	matches the address being requested and never check to make sure a
				//	packet isn't being rerequested.  Are they just relying on blind luck?
				//	Or is the protocol really that bad?
				//
				//	I even disassembled the "official" FrSky tool to compare and it mostly
				//	matches with the opentx code logic [with only a few slight differences
				//	to work with QIODevice instead of RTOS functions].  The pseudocode of
				//	the FrSky tool looks something like this:
				//		uint32_t nFileAddress = 0;		the relative address in the firmware file
				//		uint32_t nReqAddress = 0;		the address requested in the PRIM_REQ_DATA_ADDR from device
				//		uint32_t data[1024/sizeof(uint32_t)]);		data buffer to read file
				//		bool transmit_loop = true;
				//		while (transmit_loop) {
				//			if (nFileAddress == nReqAddress) {
				//				if (firmware.atEOF) {
				//					transmit_loop = false;
				//					continue;
				//				}
				//
				//				nFileAddress += firmware.read(data, 1024, ...);		Read 1K from file and bump address by bytes read
				//			}
				//			offset = (nReqAddress & 0x3FF) >> 2;
				//			send_data(data[offset]);		Send the 4-bytes indexed by address requested
				//		}
				//		send_End_of_Data();
				//
				//	That is horrible and so Minnie Mouse!  This means that the device
				//	can't just rerequest anything outside of the current 1K block and
				//	means it must always start at 0 and increase linearly without any
				//	jumps, which pigeonholes them considerably.  And to make matters
				//	worse, they don't even check the CRC of the inbound address requests
				//	from the device.  And the opentx code is a bit worse -- due to
				//	their using a for-loop for the count of 32-bit words in the current
				//	block, it means the device can't rerequest anything at all, whereas
				//	the FrSky code could at least rerequest within the current 1K block,
				//	up until the point where (nFileAddress == nReqAddress).
				//	The other goofiness is in the FrSky protocol is that you'd think
				//	the extra byte in the data packet would be the packet counter within
				//	the current block -- i.e. that it would be (offset & 0xFF).  But no,
				//	they use (nReqAddress & 0xFF), which is not unique and wastes the
				//	last 2-bits, which will always be zero.  WTF!?
				//
				//	I'm going to at least log cases where the address isn't what is
				//	expected, either by the device requesting them out-of-order or with
				//	some predisposed offset or from a retry.  At present, this code still
				//	reflects the Minnie Mouse behavior of the FrSky/opentx code, but we'll
				//	at least log cases where Minnie and Mickey end up in divorce court...
				//	[because Minnie's F'ing Goofy]
				//
				assert(!m_pFirmware.isNull());
				assert(m_runmode == FSM_RM_FLASH_PROGRAM);		// This is the program-only transfer
				assert(bIsWaitState);			// This should never be a retry
				if (m_nReqAddress == m_nFileAddress) {		// See if we need to read more data from firmware file
					int nBlockSize = m_pFirmware->read((char*)arrFirmwareBlock, sizeof(arrFirmwareBlock));
					// Handle both sequential devices and random devices and allow for
					//		random access devices/files containing other data after the
					//		firmware block.  Note that according to Qt docs, atEnd()
					//		may return true on certain special sequential devices when
					//		they aren't really atEnd().  So, we do the read above, which
					//		should return 0 if it really is atEnd.  Or, if we know the
					//		size of the firmware, either because of it being a random
					//		access device or a FRSK file which has a header with the
					//		size, then we'll use that to trigger the end for special
					//		cases when there is data beyond the firmware data block:
					if (((nBlockSize == 0) && m_pFirmware->atEnd()) ||
						((m_nFirmwareSize != 0) && (m_nFileAddress == m_nFirmwareSize))) {
						// Update progress to end:
						if (m_pUICallback && m_nFirmwareSize) {
							m_pUICallback->setProgressPos((m_nFirmwareSize/FIRMWARE_BLOCK_SIZE) +
															((m_nFirmwareSize % FIRMWARE_BLOCK_SIZE) ?  1 : 0));
						}

						assert((m_nFirmwareSize == 0) || (m_nFileAddress == m_nFirmwareSize));
						m_state = SPORT_END_TRANSFER;
						waitState(SPORT_COMPLETE, 2000, 1);		// Send only once, waiting up to 2sec
						sendFrame(CSportFirmwarePacket(PRIM_DATA_EOF), tr("Data EOF"));
						break;
					}

					if (nBlockSize < 0) {
						m_strLastError = tr("Error reading firmware file");
						m_state = SPORT_FAIL;
						emit flashComplete(false);
						return;
					}

					m_nFileAddress += nBlockSize;
					if ((m_nFirmwareSize != 0) && (m_nFileAddress > m_nFirmwareSize)) {
						// Truncate reads at the known firmware file size, if we know
						//	it, such as from the FRSK header.  This allows for random
						//	access files to have data after the firmware data block
						//	and works equally as well for sequential streams:
						m_nFileAddress = m_nFirmwareSize;
					}

					// Update progress:
					if (m_pUICallback && m_nFirmwareSize) {
						m_pUICallback->setProgressPos(m_nFileAddress/FIRMWARE_BLOCK_SIZE);
					}
				}
				m_state = SPORT_DATA_TRANSFER;
				waitState(SPORT_DATA_REQ, 2000, 1);		// Send only once, waiting up to 2sec
				sendFrame(CSportFirmwarePacket(PRIM_DATA_WORD, &arrFirmwareBlock[m_nReqAddress & 0x3FF],
							m_nReqAddress & 0xFF), tr("Data Xfer: %1.%2.%3.%4")
							.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+0], 2, 16, QChar('0'))
							.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+1], 2, 16, QChar('0'))
							.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+2], 2, 16, QChar('0'))
							.arg(arrFirmwareBlock[(m_nReqAddress & 0x3FF)+3], 2, 16, QChar('0')));
				break;

			case SPORT_DATA_AVAIL:
			{
				assert(!m_pFirmware.isNull());
				assert(m_runmode == FSM_RM_FLASH_READ);		// This is the read-only transfer
				assert(bIsWaitState);			// This should never be a retry
				int nWritten = m_pFirmware->write((char*)m_arrDataRead, sizeof(m_arrDataRead));
				if (nWritten != sizeof(m_arrDataRead)) {		// Note: this includes negative numbers (like -1) for error states
					m_strLastError = tr("Error writing firmware file");
					m_state = SPORT_FAIL;
					emit flashComplete(false);
					return;
				}
				m_nReqAddress += sizeof(m_arrDataRead);
				m_nFileAddress += sizeof(m_arrDataRead);
				m_state = SPORT_DATA_TRANSFER;
				waitState(SPORT_DATA_AVAIL, 2000, 1);	// Send only once, waiting up to 2sec
				sendFrame(CSportFirmwarePacket(PRIM_CMD_UPLOAD, m_nReqAddress), tr("Req Data, Addr=0x%1").arg(m_nReqAddress, 8, 16, QChar('0')));		// ??? Do we use PRIM_CMD_UPLOAD here or PRIM_DATA_WORD ???
				// Add CRC retry logic here so that we aren't as Minnie Mouse as FrSky's download mode?
			}
				break;

			case SPORT_COMPLETE:
				if (m_pUICallback) {
					m_pUICallback->setProgressText(tr("Complete"));
				}
				m_strLastError.clear();
				emit flashComplete(true);
				break;

			case SPORT_CRC_FAILURE:
				// This one is a "special case" in that it's never an
				//	"expected" state and so needs to be processed
				//	regardless of whether it's a "retry" or not:
				if (m_pUICallback) {
					m_pUICallback->setProgressText(tr("Device cancelled, please retry"));
				}
				m_strLastError = tr("Device reports CRC error");
				m_state = SPORT_FAIL;
				emit flashComplete(false);
				break;

			case SPORT_FAIL:
				// SPORT_FAIL is always an output/terminal state from
				//	the state-machine processing, never a "next state".
				assert(false);
				break;

			case SPORT_DATA_TRANSFER:
			case SPORT_END_TRANSFER:
				// Since we aren't doing retries for data transfers or
				//	end transfers (see comments above), these cases
				//	should never happen.  If ever the protocol changes
				//	to allow retries, these can be lumped into the
				//	SPORT_DATA_REQ logic above...
				assert(false);
				break;
		}
		if (!bIsWaitState) {
			--m_nRetryCount;				// Handle retry count if this was a retry
			m_tmrEventTimeout.start();		// And restart the timer
		}
	} else {
		// If this was a retry and we are out of retries,
		//	process error message:
		switch (m_state) {
			case SPORT_FLASHMODE_REQ:
				if (m_pUICallback) {
					m_pUICallback->setProgressText(tr("Timed out, please retry"));
				}
				m_strLastError = tr("Device not responding");
				break;

			case SPORT_VERSION_REQ:
				if (m_pUICallback) {
					m_pUICallback->setProgressText(tr("Get device version failed"));
				}
				m_strLastError = tr("Version request failed");
				break;

			case SPORT_CMD_DOWNLOAD:
			case SPORT_CMD_UPLOAD:
			case SPORT_DATA_TRANSFER:
				// If we get here on a SPORT_DATA_TRANSFER state, it means
				//	we timed out sending the last block without getting a
				//	response.  Ideally, we'd ask the user if they'd like to
				//	try and resend it in case the serial cable came loose or
				//	something.  But we can't because the Minnie Mouse Frsky
				//	protocol (as noted above) can't resync addresses.  There's
				//	no way for us to know if the device received our last
				//	frame and it was its data request for the next address
				//	that got lost or if it never saw our last data transmission.
				//	Therefore, if we resend the data, it could think that it's
				//	for the next transfer that it just requested and not the
				//	previous and put the data in the wrong memory location.
				//	The only recourse is to do here the same thing that the
				//	OpenTx code does, which is to just abort with a "Data
				//	transfer refused" error and let the user start all over.
				//	This could have been solved if they would have transferred
				//	the address more clearly from the tool in the transfer
				//	data.  There is at least the opportunity for the device
				//	to rerequest some data, but it can't do out-of-band
				//	rerequests on the same block if it doesn't get a response
				//	from us because we timed out and didn't see a response from
				//	it -- if it did, it would likely trample on top of our
				//	next data transfer, since it's a single-wire half-duplex
				//	bus topology.  So again, very Minnie Mouse.  Maybe this
				//	is one of their reasons for moving toward F.port as a
				//	replacement for S.port?  That is -- go full-duplex and
				//	get rid of the signal level inversion and be a normal
				//	serial port??  <sigh>
				m_strLastError = tr("Data transfer refused");
				break;

			case SPORT_END_TRANSFER:
				m_strLastError = tr("Firmware rejected");
				break;

			case SPORT_CRC_FAILURE:
				// This one is a "special case" in that it's never an
				//	"expected" state and so needs to be processed
				//	regardless of whether it's a "retry" or not:
				if (m_pUICallback) {
					m_pUICallback->setProgressText(tr("Device cancelled, please retry"));
				}
				m_strLastError = tr("Device reports CRC error");
				break;

			case SPORT_IDLE:
			case SPORT_START:
			case SPORT_FLASHMODE_ACK:
			case SPORT_VERSION_ACK:
			case SPORT_USER_ABORT:
			case SPORT_DATA_REQ:
			case SPORT_DATA_AVAIL:
			case SPORT_COMPLETE:
			case SPORT_FAIL:
				// We can't be out of retries on states we never
				//	retry.  Or more specifically, we can't trigger
				//	these on input-only states.
				assert(false);
				break;
		}
		m_state = SPORT_FAIL;
		m_tmrEventTimeout.stop();
		emit flashComplete(false);
	}
}

CFrskyDeviceFirmwareUpdate::FrameProcessResult CFrskyDeviceFirmwareUpdate::processFrame()
{
	assert(m_rxBuffer.haveCompletePacket() && m_rxBuffer.isFirmwarePacket());
	FrameProcessResult results;
	results.m_bAdvanceState = false;

	if ((m_rxBuffer.firmwarePacket().m_physicalId == PHYS_ID_FIRMRSP) &&
		(m_rxBuffer.firmwarePacket().m_primId == PRIM_ID_FIRMWARE_FRAME)) {
		switch (m_rxBuffer.firmwarePacket().m_cmd) {
			case PRIM_ACK_FLASHMODE:		// Device ACK Flash Mode and is present
				results.m_strLogDetail = tr("Flash Mode ACK");
				if (m_state == SPORT_FLASHMODE_REQ) {
					m_state = SPORT_FLASHMODE_ACK;
					results.m_bAdvanceState = true;
				}
				break;

			case PRIM_ACK_VERSION:			// Device ACK Version Request
				results.m_strLogDetail = tr("Version ACK");
				if (m_state == SPORT_VERSION_REQ) {
					m_nVersionInfo = m_rxBuffer.firmwarePacket().dataValue();
					m_state = SPORT_VERSION_ACK;
					results.m_strLogDetail += tr(" : Version=0x%1").arg(m_nVersionInfo, 8, 16, QChar('0'));
					results.m_bAdvanceState = true;
				}
				break;

			case PRIM_REQ_DATA_ADDR:		// Device requests specific file address from firmware image
				results.m_strLogDetail = tr("Req Data");
				if ((m_state == SPORT_CMD_DOWNLOAD) ||			// Either initial command download
					(m_state == SPORT_CMD_UPLOAD) ||			//	or initial command upload
					(m_state == SPORT_DATA_TRANSFER)) {			//	or ongoing data transfer
					switch (m_runmode) {
						case FSM_RM_FLASH_PROGRAM:
							m_nReqAddress = m_rxBuffer.firmwarePacket().dataValue();
							results.m_strLogDetail += tr(", Addr=0x%1").arg(m_nReqAddress, 8, 16, QChar('0'));
							m_state = SPORT_DATA_REQ;
							break;
						case FSM_RM_FLASH_READ:
							m_arrDataRead[0] = m_rxBuffer.firmwarePacket().m_data[0];
							m_arrDataRead[1] = m_rxBuffer.firmwarePacket().m_data[1];
							m_arrDataRead[2] = m_rxBuffer.firmwarePacket().m_data[2];
							m_arrDataRead[3] = m_rxBuffer.firmwarePacket().m_data[3];
							results.m_strLogDetail += tr(", Data Xfer: %1.%2.%3.%4")
									.arg(m_arrDataRead[0], 2, 16, QChar('0'))
									.arg(m_arrDataRead[1], 2, 16, QChar('0'))
									.arg(m_arrDataRead[2], 2, 16, QChar('0'))
									.arg(m_arrDataRead[3], 2, 16, QChar('0'));
							m_state = SPORT_DATA_AVAIL;
							break;
						default:
							assert(false);
							break;
					}
					results.m_bAdvanceState = true;
				}
				break;

			case PRIM_END_DOWNLOAD:			// Device reports end-of-download (complete)
				results.m_strLogDetail = tr("End Download");
				m_state = SPORT_COMPLETE;
				results.m_bAdvanceState = true;
				break;

			case PRIM_DATA_CRC_ERR:			// Device reports CRC failure
				results.m_strLogDetail = tr("Data Error Response (CRC?)");
				m_state = SPORT_CRC_FAILURE;
				results.m_bAdvanceState = true;
				break;

			default:
				results.m_strLogDetail = tr("*** Unexpected/Unknown Firmware Packet");
				break;

			// What about flash erase or write failures?  There seems
			//	to be no way for the device to notify us of that...
			//	Unless the "CRC ERR" is really any error and not CRC-only...
		}
	}	// Ignore others, as they are probably just our echos

	return results;
}

void CFrskyDeviceFirmwareUpdate::waitState(State nNextState, uint32_t nTimeout, int nRetries)
{
	m_nextState = nNextState;
	m_nRetryCount = nRetries ? (nRetries-1) : nRetries;		// If not retrying, set to zero.  Otherwise, retries remaining is one less than the number of tries
	if (nTimeout > 0) {
		m_tmrEventTimeout.start(nTimeout);
	} else {
		m_tmrEventTimeout.stop();		// Make sure timer is off if not doing retries (it probably is anyway)
	}
}

void CFrskyDeviceFirmwareUpdate::sendFrame(const CSportFirmwarePacket &packet, const QString &strLogDetail)
{
	CSportTxBuffer frameFirmware;
	frameFirmware.pushFirmwarePacketWithByteStuffing(packet);

	QByteArray arrBytes(1, 0x7E);	// Start of Frame
	arrBytes.append(frameFirmware.data());
	m_frskySportIO.logMessage(CFrskySportIO::LT_TX, arrBytes, strLogDetail);
	m_frskySportIO.port().write(arrBytes);
	m_frskySportIO.port().flush();
}

void CFrskyDeviceFirmwareUpdate::en_timeout()
{
	if (m_state != m_nextState) {
		QTimer::singleShot(1, this, SLOT(nextState()));			// Trigger retry (do this with a oneshot event so we are outside of timeout timer's handler)
	}
	// Otherwise, if we've already hit the next (expected)
	//	state, then this isn't a timeout but a race-condition
	//	between the processing loop and the timer, so ignore.
}

void CFrskyDeviceFirmwareUpdate::en_userCancel()
{
	m_state = SPORT_USER_ABORT;
	m_nextState = SPORT_USER_ABORT;
	nextState();
}

// ----------------------------------------------------------------------------

// This function gets triggered whenever the serial port
//	has data.  Here, we emit a queued connection function
//	that handles the read.  We do that rather than read
//	the data here to avoid a race condition between reading
//	the data and getting another event from the serial device.
void CFrskyDeviceFirmwareUpdate::en_readyRead()
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
void CFrskyDeviceFirmwareUpdate::en_receive()
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
					bool bIsEcho = m_rxBuffer.isFirmwarePacket() && (m_rxBuffer.firmwarePacket().m_physicalId == PHYS_ID_FIRMCMD);

					FrameProcessResult procResults;
					if (m_rxBuffer.isFirmwarePacket()) procResults = processFrame();		// Process only firmware packets

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


// ============================================================================


CFrskyDeviceFirmwareUpdate::CFrskyDeviceFirmwareUpdate(CFrskySportIO &frskySportIO, CUICallback *pUICallback, QObject *pParent)
	:	QObject(pParent),
		m_frskySportIO(frskySportIO),
		m_pUICallback(pUICallback)
{
	connect(&m_frskySportIO.port(), SIGNAL(readyRead()), this, SLOT(en_readyRead()));
	connect(this, SIGNAL(dataAvailable()), this, SLOT(en_receive()), Qt::QueuedConnection);

	connect(&m_tmrEventTimeout, SIGNAL(timeout()), this, SLOT(en_timeout()), Qt::DirectConnection);
	if (pUICallback) {
		pUICallback->hookCancel(this, SLOT(en_userCancel()));
	}
}

CFrskyDeviceFirmwareUpdate::~CFrskyDeviceFirmwareUpdate()
{

}

// ----------------------------------------------------------------------------

CFrskyDeviceFirmwareUpdate::TFirmwareFileContent CFrskyDeviceFirmwareUpdate::verifyFRSKFirmwareFileContent(QIODevice &firmware)
{
	TFirmwareFileContent ffc;
	ffc.m_bValid = true;			// default to good

	if (!firmware.isOpen() || !firmware.isReadable()) {
		ffc.m_strLastError = tr("Firmware file not open and readable");
		ffc.m_bValid = false;
		return ffc;
	}

	if (firmware.isSequential()) {
		ffc.m_strLastError = tr("Firmware check on works on random-access streams");
		ffc.m_bValid = false;
		return ffc;
	}

	qint64 nFileSize = firmware.size();
	qint64 nFilePos = firmware.pos();

	FrSkyFirmwareInformation header;
	int nReadSize = firmware.read((char *)&header, sizeof(header));
	if ((nReadSize < 0) ||
		(static_cast<size_t>(nReadSize) != sizeof(FrSkyFirmwareInformation))) {
		firmware.seek(nFilePos);
		ffc.m_strLastError = tr("Failed to Read Firmware Header from File");
		ffc.m_bValid = false;
		return ffc;
	}
	if ((header.headerVersion != 1) ||
		(header.fourcc[0] != 'F') ||
		(header.fourcc[1] != 'R') ||
		(header.fourcc[2] != 'S') ||
		(header.fourcc[3] != 'K')) {
		firmware.seek(nFilePos);
		ffc.m_strLastError = tr("Wrong/unknown .frsk file format");
		ffc.m_bValid = false;
		return ffc;
	}

	if ((nFileSize != 0) &&
		(nFileSize < static_cast<qint64>(sizeof(FrSkyFirmwareInformation) + header.size))) {
		firmware.seek(nFilePos);
		ffc.m_strLastError = tr("Wrong firmware file size (file doesn't match FRSK header)");
		ffc.m_bValid = false;
		return ffc;
	}

	uint16_t nCRC = 0;
	QByteArray baFirmwareBlock;
	qint64 nRemaining = header.size;
	while (nRemaining) {
		int nReadSize = std::min(static_cast<qint64>(FIRMWARE_BLOCK_SIZE), nRemaining);
		baFirmwareBlock = firmware.read(nReadSize);
		if (baFirmwareBlock.size() != nReadSize) {
			firmware.seek(nFilePos);
			ffc.m_strLastError = tr("Error reading firmware file");
			ffc.m_bValid = false;
			return ffc;
		}
		nCRC = crc16(CRC_1021, (const uint8_t *)baFirmwareBlock.data(), baFirmwareBlock.size(), nCRC);
		nRemaining -= nReadSize;
	}
	if (nCRC != header.crc) {
		firmware.seek(nFilePos);
		ffc.m_strLastError = tr("Firmware file CRC doesn't match data.  File is corrupt.");
		ffc.m_bValid = false;
		return ffc;
	}

	firmware.seek(nFilePos);

	QString strFirmwareID;
	strFirmwareID += tr("Product Family: ");
	const FirmwareDescriptor *pDescriptor = &conarrProductFamily[0];
	while ((header.productFamily != pDescriptor->m_nID) && (pDescriptor->m_nID != -1)) ++pDescriptor;
	strFirmwareID += pDescriptor->m_strDescription + "\n";
	strFirmwareID += tr("Product ID: ");
	switch (header.productFamily) {
		case FIRMWARE_FAMILY_INTERNAL_MODULE:
		case FIRMWARE_FAMILY_EXTERNAL_MODULE:
			pDescriptor = &conarrModuleProductID[0];
			while ((header.productId != pDescriptor->m_nID) && (pDescriptor->m_nID != -1)) ++pDescriptor;
			strFirmwareID += pDescriptor->m_strDescription + "\n";
			break;

		case FIRMWARE_FAMILY_RECEIVER:
			pDescriptor = &conarrReceiverProductID[0];
			while ((header.productId != pDescriptor->m_nID) && (pDescriptor->m_nID != -1)) ++pDescriptor;
			strFirmwareID += pDescriptor->m_strDescription + "\n";
			break;

		default:
			strFirmwareID += "Unknown\n";
			break;
	}
	strFirmwareID += tr("Version: %1.%2.%3\n").arg(header.firmwareVersionMajor).arg(header.firmwareVersionMinor).arg(header.firmwareVersionRevision);
	strFirmwareID += tr("Size: %1 (bytes)\n").arg(header.size);
	strFirmwareID += tr("CRC: 0x%1\n").arg(QString("%1").arg(header.crc, 4, 16, QChar('0')).toUpper());

	ffc.m_strFirmwareDetail = strFirmwareID;

	return ffc;
}

// ----------------------------------------------------------------------------

bool CFrskyDeviceFirmwareUpdate::idDevice(bool bBlocking)
{
	// Reset the the state-machine, in case this function gets
	//	called again without creating a new object:
	m_runmode = FSM_RM_DEVICE_ID;
	m_state = SPORT_IDLE;
	m_nextState = SPORT_START;
	m_nReqAddress = 0;
	m_nFileAddress = 0;
	m_nVersionInfo = 0;
	m_pFirmware.clear();
	m_nFirmwareSize = 0;
	m_strLastError.clear();

	m_state = SPORT_START;
	nextState();		// Start device id state-machine

	if (bBlocking) {
		while ((m_state != SPORT_COMPLETE) &&
			   (m_state != SPORT_FAIL)) {
			QCoreApplication::processEvents();
			QCoreApplication::sendPostedEvents();
		}
		return (m_state == SPORT_COMPLETE);
	}

	return true;
}

bool CFrskyDeviceFirmwareUpdate::flashDeviceFirmware(QIODevice &firmware, bool bIsFRSKFile, bool bBlocking)
{
	// Reset the the state-machine, in case this function gets
	//	called again without creating a new object:
	m_runmode = FSM_RM_FLASH_PROGRAM;
	m_state = SPORT_IDLE;
	m_nextState = SPORT_START;
	m_nReqAddress = 0;
	m_nFileAddress = 0;
	m_nVersionInfo = 0;
	m_pFirmware = &firmware;
	m_nFirmwareSize = (firmware.isSequential() ? 0 : firmware.size());		// Since sequential streams is bytesAvailable and not overall size, use zero for them and special case it, but read full size for random access
	m_strLastError.clear();

	if (!firmware.isOpen() || !firmware.isReadable()) {
		m_strLastError = tr("Firmware file not open and readable");
		emit flashComplete(false);
		return false;
	}

	// This empty-file check works for random-access files, but not
	//	sequential.  The best we can do with sequential is see if
	//	there's a FRSK header and make sure we can at least read that
	//	and hope we don't run out of data during data transfer:
	if (!firmware.isSequential() && ((m_nFirmwareSize == 0) ||
		(bIsFRSKFile && (static_cast<size_t>(m_nFirmwareSize) <= sizeof(FrSkyFirmwareInformation))))) {
		m_strLastError = tr("File is Empty");
		emit flashComplete(false);
		return false;
	}

	// FRSK file will have FrSkyFirmwareInformation header:
	if (bIsFRSKFile) {
		// Do the verification on random-access files first, since
		//	it will leave the file back at the start:
		if (!firmware.isSequential()) {
			qint64 nFilePos = firmware.pos();
			TFirmwareFileContent ffc = verifyFRSKFirmwareFileContent(firmware);
			assert(nFilePos == firmware.pos());		// Check verifyFRSKFirmwareFileContent().  It should return file to start position!
			if (!ffc.m_bValid) {
				m_strLastError = ffc.m_strLastError;
				emit flashComplete(false);
				return false;
			} else if (m_pUICallback && m_pUICallback->isInteractive()) {
				ffc.m_strFirmwareDetail.prepend(tr("FRSK Firmware File Identity:\n\n"));
				BTN_TYPE nResult = m_pUICallback->promptUser(CUICallback::PT_QUESTION, ffc.m_strFirmwareDetail,
																CUICallback::Ok | CUICallback::Cancel, CUICallback::Ok);
				if (nResult != CUICallback::Ok) {
					m_strLastError = tr("User aborted programming");
					emit flashComplete(false);
					return false;
				}
			}
		}

		// Must read/check the FRSK header itself here, even though
		//	we just called verifyFRSKFirmwareFileContent() in
		//	order to handle the sequential stream case correctly.
		//	Plus we have to advance beyond the header:
		FrSkyFirmwareInformation header;
		int nReadSize = firmware.read((char *)&header, sizeof(header));
		if ((nReadSize < 0) ||
			(static_cast<size_t>(nReadSize) != sizeof(FrSkyFirmwareInformation))) {
			m_strLastError = tr("Failed to Read Firmware Header from File");
			emit flashComplete(false);
			return false;
		}
		if ((header.headerVersion != 1) ||
			(header.fourcc[0] != 'F') ||
			(header.fourcc[1] != 'R') ||
			(header.fourcc[2] != 'S') ||
			(header.fourcc[3] != 'K')) {
			m_strLastError = tr("Wrong/unknown .frsk file format");
			emit flashComplete(false);
			return false;
		}

		// Use the header size for the firmware size so that we have
		//	the correct firmware size -- works for both the sequential
		//	case (where the size is 0 to this point) or random access
		//	case where there's data beyond the firmware data block
		//	(which isn't currently the case for type-1 FRSK files, but
		//	may be a future case):
		m_nFirmwareSize = header.size;
	}

	m_state = SPORT_START;
	nextState();		// Start programming state-machine

	if (bBlocking) {
		while ((m_state != SPORT_COMPLETE) &&
			   (m_state != SPORT_FAIL)) {
			QCoreApplication::processEvents();
			QCoreApplication::sendPostedEvents();
		}
		return (m_state == SPORT_COMPLETE);
	}

	return true;
}

bool CFrskyDeviceFirmwareUpdate::readDeviceFirmware(QIODevice &firmware, bool bBlocking)
{
	// Reset the the state-machine, in case this function gets
	//	called again without creating a new object:
	m_runmode = FSM_RM_FLASH_READ;
	m_state = SPORT_IDLE;
	m_nextState = SPORT_START;
	m_nReqAddress = 0;
	m_nFileAddress = 0;
	m_nVersionInfo = 0;
	m_pFirmware = &firmware;		// Note: we intentionally don't reset/clear device stream -- we'll only append to the end, or overwrite if that's how it was opened, it's up to the caller
	m_nFirmwareSize = 0;
	m_strLastError.clear();

	if (!firmware.isOpen() || !firmware.isWritable()) {
		m_strLastError = tr("Firmware file not open and writable");
		emit flashComplete(false);
		return false;
	}

	m_state = SPORT_START;
	nextState();		// Start reading state-machine

	if (bBlocking) {
		while ((m_state != SPORT_COMPLETE) &&
			   (m_state != SPORT_FAIL)) {
			QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
			QCoreApplication::sendPostedEvents();
		}
		return (m_state == SPORT_COMPLETE);
	}

	return true;
}

// ============================================================================
