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

#ifndef CPERSISTENTSETTINGS_H
#define CPERSISTENTSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QString>

// ============================================================================

enum SPORT_ID_ENUM {				// DO NOT CHANGE ORDER
	SPIDE_SPORT1 = 0,
	// ----
	SPIDE_COUNT
};
Q_DECLARE_METATYPE(SPORT_ID_ENUM)

// ----------------------------------------------------------------------------

struct DEVICE_SETTINGS {
	QString m_strSerialPort;
	int m_nBaudRate;
	char m_chrParity;
	int m_nDataBits;
	int m_nStopBits;
};

// ----------------------------------------------------------------------------

class CPersistentSettings : public QSettings
{
	Q_OBJECT
private:				// Enforce Singleton:
	CPersistentSettings(QObject *parent = nullptr);

public:
	~CPersistentSettings();
	static CPersistentSettings *instance();

	void saveSettings();
	void loadSettings();

	// --------------------------------

	QString getDeviceSerialPort(SPORT_ID_ENUM nSport) const { return m_deviceSettings[nSport].m_strSerialPort; }
	int getDeviceBaudRate(SPORT_ID_ENUM nSport) const { return m_deviceSettings[nSport].m_nBaudRate; }
	char getDeviceParity(SPORT_ID_ENUM nSport) const { return m_deviceSettings[nSport].m_chrParity; }
	int getDeviceDataBits(SPORT_ID_ENUM nSport) const { return m_deviceSettings[nSport].m_nDataBits; }
	int getDeviceStopBits(SPORT_ID_ENUM nSport) const { return m_deviceSettings[nSport].m_nStopBits; }

	// ----

	QString getLogFileLastPath() const { return m_strLogFileLastPath; }

	// --------------------------------

public slots:
	void setDeviceSerialPort(SPORT_ID_ENUM nSport, const QString &strSerialPort) { m_deviceSettings[nSport].m_strSerialPort = strSerialPort; }
	void setDeviceBaudRate(SPORT_ID_ENUM nSport, int nBaudRate) { m_deviceSettings[nSport].m_nBaudRate = nBaudRate; }
	void setDeviceParity(SPORT_ID_ENUM nSport, char chrParity) { m_deviceSettings[nSport].m_chrParity = chrParity; }
	void setDeviceDataBits(SPORT_ID_ENUM nSport, int nDataBits) { m_deviceSettings[nSport].m_nDataBits = nDataBits; }
	void setDeviceStopBits(SPORT_ID_ENUM nSport, int nStopBits) { m_deviceSettings[nSport].m_nStopBits = nStopBits; }

	// ----

	void setLogFileLastPath(const QString &strLastPath) { m_strLogFileLastPath = strLastPath; }

	// --------------------------------

signals:

protected:
	// Device Settings:
	// ----------------
	DEVICE_SETTINGS m_deviceSettings[SPIDE_COUNT];
	// ----

	// LogFile Settings:
	// -----------------
	QString m_strLogFileLastPath;
	// ----

private:
};

// ============================================================================

#endif // CPERSISTENTSETTINGS_H

