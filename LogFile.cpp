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

#include "LogFile.h"

// ============================================================================

CLogFile::CLogFile(QObject *pParent)
	:	QObject(pParent)
{
	m_timerLogFile.start();
}

CLogFile::~CLogFile()
{
}

bool CLogFile::openLogFile(const QString &strFilePathName, QIODevice::OpenMode nOpenMode)
{
	m_fileLogFile.setFileName(strFilePathName);
	if (!m_fileLogFile.open(nOpenMode)) return false;
	m_pLogFile.reset(new QTextStream(static_cast<QIODevice *>(&m_fileLogFile)));
	return true;
}

void CLogFile::closeLogFile()
{
	m_pLogFile.reset();
	m_fileLogFile.close();
}

void CLogFile::writeLogString(const QString &strLogString)
{
	if (!m_pLogFile.isNull() && m_pLogFile->device()->isOpen() && m_pLogFile->device()->isWritable()) {
		(*m_pLogFile) << QString("%1").arg(elapsedTime(), 0, 'f', 4) << ": " << strLogString << Qt::endl;
	}
}

// ============================================================================
