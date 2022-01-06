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

#include "frsky_sport_telemetry.h"
#include "UICallback.h"

// ============================================================================

CFrskySportDeviceTelemetry::FrameProcessResult CFrskySportDeviceTelemetry::processFrame()
{
	FrameProcessResult results;

	if (m_rxBuffer.haveTelemetryPoll()) {
		uint8_t nPhysId = m_rxBuffer.telemetryPollPacket().getPhysicalId();
		if (nPhysId < TELEMETRY_PHYS_ID_COUNT) {
			m_telemetryEndpoints[nPhysId].m_bReceivingPolls = true;
			if (m_telemetryEndpoints[nPhysId].m_bInUse) {
				txSportPacket(m_telemetryEndpoints[nPhysId].m_responsePacket, m_telemetryEndpoints[nPhysId].m_strLogDetail, true);
				m_telemetryEndpoints[nPhysId].m_bInUse = false;
			}
		}
	} else if (m_rxBuffer.isTelemetryPacket()) {
		emit rxSportPacket(m_rxBuffer.telemetryPacket());
	}

	return results;
}

void CFrskySportDeviceTelemetry::txSportPacket(const CSportTelemetryPacket &packet, const QString &strLogDetail, bool bIsPushResponse)
{
	m_txBufferLast.reset();
	m_txBufferLast.pushPacketWithByteStuffing(packet);

	QByteArray arrBytes(1, 0x7E);	// Start of Frame
	arrBytes.append(m_txBufferLast.data());
	m_frskySportIO.logMessage(bIsPushResponse ? CFrskySportIO::LT_TXPUSH : CFrskySportIO::LT_TX, arrBytes, strLogDetail);
	m_frskySportIO.port().write(bIsPushResponse ? arrBytes.mid(2) : arrBytes);	// For push, drop the SOF and PhysicalId
	m_frskySportIO.port().flush();
}

void CFrskySportDeviceTelemetry::pushTelemetryResponse(const CSportTelemetryPacket &packet, const QString &strLogDetail)
{
	uint8_t nPhysicalId = packet.getPhysicalId();
	assert(nPhysicalId < TELEMETRY_PHYS_ID_COUNT);
	m_telemetryEndpoints[nPhysicalId].m_responsePacket = packet;
	m_telemetryEndpoints[nPhysicalId].m_bInUse = true;
	m_telemetryEndpoints[nPhysicalId].m_strLogDetail = strLogDetail;
}

void CFrskySportDeviceTelemetry::en_userCancel()
{
	// TODO
}


// ----------------------------------------------------------------------------

// This function gets triggered whenever the serial port
//	has data.  Here, we emit a queued connection function
//	that handles the read.  We do that rather than read
//	the data here to avoid a race condition between reading
//	the data and getting another event from the serial device.
void CFrskySportDeviceTelemetry::en_readyRead()
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
void CFrskySportDeviceTelemetry::en_receive()
{
	QByteArray arrBytes = m_frskySportIO.port().readAll();
	if (!arrBytes.isEmpty()) {
		for (int ndx = 0; ndx < arrBytes.size(); ++ndx) {
			QByteArray baExtraneous = m_rxBuffer.pushByte(arrBytes.at(ndx));
			if (!baExtraneous.isEmpty()) {
				m_frskySportIO.logMessage(CFrskySportIO::LT_RX, baExtraneous, "*** Extraneous Bytes");
			}
			if ((ndx == arrBytes.size()-1) &&
				m_rxBuffer.haveTelemetryPoll() && m_rxBuffer.telemetryPollPacket().physicalIdValid()) {
				QByteArray baMessage(1, 0x7E);		// Add the 0x7E since it's eaten by the RxBuffer
				baMessage.append(m_rxBuffer.rawData());
				m_frskySportIO.logMessage(CFrskySportIO::LT_TELEPOLL, baMessage, QString("Poll for PhysID: %1").arg(m_rxBuffer.telemetryPollPacket().getPhysicalId()));
				processFrame();
			} else if (m_rxBuffer.haveCompletePacket()) {
				bool bIsEcho = (QByteArray((char*)m_rxBuffer.data(), m_rxBuffer.size()) == m_txBufferLast.data());
				m_txBufferLast.reset();

				if (!m_rxBuffer.isFirmwarePacket() && !m_rxBuffer.isTelemetryPacket()) {
					QByteArray baUnexpected(1, 0x7E);		// Add the 0x7E since it's eaten by the RxBuffer
					baUnexpected.append(m_rxBuffer.rawData());
					m_frskySportIO.logMessage(bIsEcho ? CFrskySportIO::LT_TXECHO : CFrskySportIO::LT_RX, baUnexpected, "*** Unexpected/Unknown packet");
				} else {
					uint8_t nExpectedCRC = m_rxBuffer.isFirmwarePacket() ? m_rxBuffer.firmwarePacket().crc() :
															m_rxBuffer.telemetryPacket().crc();
					FrameProcessResult procResults = processFrame();		// Process all packets

					if (CPersistentSettings::instance()->getDataConfigLogTxEchos() ||
						(!CPersistentSettings::instance()->getDataConfigLogTxEchos() && !bIsEcho) ||
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

//					if (procResults.m_bAdvanceState) nextState();
				}

				m_rxBuffer.reset();
			}
		}
	}
}

// ============================================================================


CFrskySportDeviceTelemetry::CFrskySportDeviceTelemetry(CFrskySportIO &frskySportIO, CUICallback *pUICallback, QObject *pParent)
	:	QObject(pParent),
		m_frskySportIO(frskySportIO),
		m_pUICallback(pUICallback)
{
	connect(&m_frskySportIO.port(), SIGNAL(readyRead()), this, SLOT(en_readyRead()));
	connect(this, SIGNAL(dataAvailable()), this, SLOT(en_receive()), Qt::QueuedConnection);

	if (pUICallback) {
		pUICallback->hookCancel(this, SLOT(en_userCancel()));
	}
}

CFrskySportDeviceTelemetry::~CFrskySportDeviceTelemetry()
{
}

// ----------------------------------------------------------------------------


// ============================================================================

