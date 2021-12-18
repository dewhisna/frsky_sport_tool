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

// ============================================================================

Cfrsky_sport_io::Cfrsky_sport_io(SPORT_ID_ENUM nSport, QObject *pParent)
	:	QObject(pParent),
		m_nSportID(nSport)
{
}

Cfrsky_sport_io::~Cfrsky_sport_io()
{
}

bool Cfrsky_sport_io::openPort()
{
	closePort();

	if (CPersistentSettings::instance()->getDeviceSerialPort(m_nSportID).isEmpty()) {
		m_strLastError = tr("S.port #%1 not selected.  Set configuration first!").arg(m_nSportID+1);
		return false;
	}

	m_serialPort.setPortName(CPersistentSettings::instance()->getDeviceSerialPort(m_nSportID));
	m_serialPort.setQueryMode(QextSerialPort::EventDriven);
	m_serialPort.setBaudRate(static_cast<BaudRateType>(CPersistentSettings::instance()->getDeviceBaudRate(m_nSportID)));
	m_serialPort.setFlowControl(FLOW_OFF);
	char chParity = CPersistentSettings::instance()->getDeviceParity(m_nSportID);
	switch (chParity) {
		case 'N':
		case 'n':
			m_serialPort.setParity(PAR_NONE);
			break;
		case 'O':
		case 'o':
			m_serialPort.setParity(PAR_ODD);
			break;
		case 'E':
		case 'e':
			m_serialPort.setParity(PAR_EVEN);
			break;
	}
	m_serialPort.setDataBits(static_cast<DataBitsType>(CPersistentSettings::instance()->getDeviceDataBits(m_nSportID)));
	int nStopBits = CPersistentSettings::instance()->getDeviceStopBits(m_nSportID);
	switch (nStopBits) {
		case 1:
			m_serialPort.setStopBits(STOP_1);
			break;
		case 2:
			m_serialPort.setStopBits(STOP_2);
			break;
	}

	if (!m_serialPort.open(QIODevice::ReadWrite)) {
		m_strLastError = m_serialPort.errorString();
		return false;
	}

	connect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(en_receive()));
	return true;
}

void Cfrsky_sport_io::closePort()
{
	if (!m_serialPort.isOpen()) return;

	m_serialPort.close();
	disconnect(&m_serialPort, SIGNAL(readyRead()), this, SLOT(en_receive()));
}

// ----------------------------------------------------------------------------

void Cfrsky_sport_io::en_receive()
{
}

// ============================================================================
