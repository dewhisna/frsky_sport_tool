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

#ifndef LOG_FILE_H
#define LOG_FILE_H

#include <QString>
#include <QScopedPointer>
#include <QIODevice>
#include <QFile>
#include <QElapsedTimer>
#include <QTextStream>

// ============================================================================

class CLogFile : public QObject
{
	Q_OBJECT
public:
	CLogFile(QObject *pParent = nullptr);
	virtual ~CLogFile();

	bool openLogFile(const QString &strFilePathName, QIODevice::OpenMode nOpenMode);
	void closeLogFile();

	QString getLastError() const { return m_fileLogFile.errorString(); }

	bool isOpen() const { return (!m_pLogFile.isNull() && (m_pLogFile->device() != nullptr) && m_pLogFile->device()->isOpen()); }
	bool isWritable() const { return (!m_pLogFile.isNull() && (m_pLogFile->device() != nullptr) && m_pLogFile->device()->isOpen() && m_pLogFile->device()->isWritable()); }
	bool isReadable() const { return (!m_pLogFile.isNull() && (m_pLogFile->device() != nullptr) && m_pLogFile->device()->isOpen() && m_pLogFile->device()->isReadable()); }

	double elapsedTime() const { return static_cast<double>(m_timerLogFile.nsecsElapsed())/1000000.0; }		// in msecs

public slots:
	void writeLogString(const QString &strLogString);
	void resetTimer() { m_timerLogFile.restart(); }

protected:
	QFile m_fileLogFile;
	QScopedPointer<QTextStream> m_pLogFile;			// Currently open log file
	QElapsedTimer m_timerLogFile;					// LogFile timestamp keeper
};

// ============================================================================

#endif	// LOG_FILE_H

