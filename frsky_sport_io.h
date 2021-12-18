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

#include <QObject>
#include <QString>

#include <qextserialport.h>

// ============================================================================

class Cfrsky_sport_io : public QObject
{
	Q_OBJECT
public:
	Cfrsky_sport_io(SPORT_ID_ENUM nSport, QObject *pParent = nullptr);
	virtual ~Cfrsky_sport_io();

	SPORT_ID_ENUM getSportID() const { return m_nSportID; }

	bool openPort();
	void closePort();

	QString getLastError() const { return m_strLastError; }

protected slots:
	void en_receive();

protected:
	QString m_strLastError;
	SPORT_ID_ENUM m_nSportID;
	QextSerialPort m_serialPort;
};

// ============================================================================

#endif	// FRSKY_SPORT_IO_H

