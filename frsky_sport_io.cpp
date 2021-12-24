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

#include "frsky_sport_io.h"

#include "crc.h"

// ============================================================================

uint8_t CSportTelemetryPacket::crc() const
{
	uint16_t crc = 0;
	for (size_t i=1; i<sizeof(m_raw); ++i) {	// no CRC on 1st byte (physicalId)
		uint8_t byte = m_raw[i];
		crc += byte; // 0-1FF
		crc += crc >> 8; // 0-100
		crc &= 0x00ff;
	}
	return (0xFF - crc);
}

uint8_t CSportFirmwarePacket::crc() const
{
	// Yes, they are only sending the low-byte of the 16-bit CRC
	return crc16(CRC_1021, &m_raw[1], sizeof(m_raw)-1);		// no CRC on 1st byte (physicalId)
}

// ============================================================================

void CSportTxBuffer::pushByte(uint8_t byte)
{
	m_data.append(byte);
}

void CSportTxBuffer::pushByteWithByteStuffing(uint8_t byte)
{
	if (byte == 0x7E || byte == 0x7D) {
		pushByte(0x7D);
		pushByte(0x20 ^ byte);
	} else {
		pushByte(byte);
	}
}

void CSportTxBuffer::pushTelemetryPacketWithByteStuffing(const CSportTelemetryPacket & packet)
{
	reset();
	pushByte(packet.m_physicalId);		// no bytestuffing
	for (size_t i=1; i<sizeof(CSportTelemetryPacket); ++i) {
		pushByteWithByteStuffing(packet.m_raw[i]);
	}
	pushByteWithByteStuffing(packet.crc());
}

void CSportTxBuffer::pushFirmwarePacketWithByteStuffing(const CSportFirmwarePacket & packet)
{
	reset();
	pushByte(packet.m_physicalId);		// no bytestuffing
	for (size_t i=1; i<sizeof(CSportFirmwarePacket); ++i) {
		pushByteWithByteStuffing(packet.m_raw[i]);
	}
	pushByteWithByteStuffing(packet.crc());
}

// ----------------------------------------------------------------------------

QByteArray CSportRxBuffer::pushByte(uint8_t byte)
{
	QByteArray baExtraneous;

	if (byte == 0x7E) {			// Is this the start frame marker?
		// Note: since 0x7E is escaped and stuffed, there's no need
		//	to verify that we aren't in escapement or that we have data
		//	since we can only ever receive a 0x7E as the start frame byte
		baExtraneous = m_baExtraneous;
		reset();				// If so, reset our buffer for a new frame
		m_bHaveFrameStart = true;
	} else if (m_bHaveFrameStart) {
		if (m_size == 0) {
			m_data[m_size++] = byte;		// The physical ID is never byte stuffed
			m_bInEscape = false;			// m_bInEscape should already be false, but just in case
		} else {
			if (m_size < SPORT_BUFFER_SIZE) {
				if (m_bInEscape) {
					m_data[m_size++] = byte ^ 0x20;		// Unescape
					m_bInEscape = false;
				} else {
					if (byte == 0x7D) {					// Is this an escapement stuffing?
						m_bInEscape = true;
					} else {
						m_data[m_size++] = byte;
					}
				}
				if (haveCompletePacket()) {
					// Once we receive a complete packet, exit
					//	the frame to ignore extra bytes (which there
					//	shouldn't be any of) before next frame:
					m_bHaveFrameStart = false;
				}
			}
		}
	} else {
		// Note: ignore bytes we receive before a frame start, but
		//	keep them for logging so we know we had extraneous bytes:
		m_baExtraneous.append(byte);
	}

	return baExtraneous;
}


// ============================================================================

CFrskySportIO::CFrskySportIO(SPORT_ID_ENUM nSport, QObject *pParent)
	:	QObject(pParent),
		m_nSportID(nSport)
{
}

CFrskySportIO::~CFrskySportIO()
{
}

bool CFrskySportIO::openPort(const QString &strSerialPort)
{
	closePort();

	QString strPortName = strSerialPort;
	if (strPortName.isEmpty()) strPortName = CPersistentSettings::instance()->getDeviceSerialPort(m_nSportID);

	if (strPortName.isEmpty()) {
		m_strLastError = tr("S.port #%1 device not selected.  Set configuration first!").arg(m_nSportID+1);
		return false;
	}

	m_serialPort.setPortName(strPortName);
	m_serialPort.setBaudRate(CPersistentSettings::instance()->getDeviceBaudRate(m_nSportID));
	m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
	char chParity = CPersistentSettings::instance()->getDeviceParity(m_nSportID);
	switch (chParity) {
		case 'N':
		case 'n':
			m_serialPort.setParity(QSerialPort::NoParity);
			break;
		case 'O':
		case 'o':
			m_serialPort.setParity(QSerialPort::OddParity);
			break;
		case 'E':
		case 'e':
			m_serialPort.setParity(QSerialPort::EvenParity);
			break;
	}
	m_serialPort.setDataBits(static_cast<QSerialPort::DataBits>(CPersistentSettings::instance()->getDeviceDataBits(m_nSportID)));
	int nStopBits = CPersistentSettings::instance()->getDeviceStopBits(m_nSportID);
	switch (nStopBits) {
		case 1:
			m_serialPort.setStopBits(QSerialPort::OneStop);
			break;
		case 2:
			m_serialPort.setStopBits(QSerialPort::TwoStop);
			break;
	}

	if (!m_serialPort.open(QIODevice::ReadWrite)) {
		m_strLastError = m_serialPort.errorString();
		return false;
	}

	return true;
}

void CFrskySportIO::closePort()
{
	if (!m_serialPort.isOpen()) return;

	m_serialPort.close();
}

// ----------------------------------------------------------------------------

void CFrskySportIO::logMessage(LOG_TYPE nLT, const QByteArray &baMsg, const QString &strExtraMsg)
{
	QString strLogMsg;

	switch (nLT) {
		case LT_RX:
			strLogMsg += "Recv: ";
			break;
		case LT_TX:
			strLogMsg += "Send: ";
			break;
		case LT_TXECHO:
			strLogMsg += "Echo: ";
			break;
	}

	for (int i = 0; i < baMsg.size(); ++i) {
		if (i) strLogMsg += QChar('.');
		strLogMsg += QString("%1").arg((uint8_t)(baMsg.at(i)), 2, 16, QChar('0')).toUpper();
	}

	if (!strExtraMsg.isEmpty()) strLogMsg += "  " + strExtraMsg;

	emit writeLogString(m_nSportID, strLogMsg);
}

// ============================================================================

