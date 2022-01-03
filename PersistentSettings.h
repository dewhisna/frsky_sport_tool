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
	SPIDE_SPORT2 = 1,
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

	// ----

	SPORT_ID_ENUM getFirmwareSportPort() const { return m_nFirmwareSportPort; }
	bool getFirmwareLogTxEchos() const { return m_bFirmwareLogTxEchos; }
	// ----
	SPORT_ID_ENUM getDataConfigSportPort() const { return m_nDataConfigSportPort; }
	bool getDataConfigLogTxEchos() const { return m_bDataConfigLogTxEchos; }

	// ----

	QString getFirmwareLastReadPath() const { return m_strFirmwareLastReadPath; }
	QString getFirmwareLastWritePath() const { return m_strFirmwareLastWritePath; }

	// ----

	QString getLuaScriptLastPath() const { return m_strLuaScriptLastPath; }
	int getLuaScreenTheme() const { return m_nLuaScreenTheme; }

	// ----

	// --------------------------------

public slots:
	void setDeviceSerialPort(SPORT_ID_ENUM nSport, const QString &strSerialPort) { m_deviceSettings[nSport].m_strSerialPort = strSerialPort; }
	void setDeviceBaudRate(SPORT_ID_ENUM nSport, int nBaudRate) { m_deviceSettings[nSport].m_nBaudRate = nBaudRate; }
	void setDeviceParity(SPORT_ID_ENUM nSport, char chrParity) { m_deviceSettings[nSport].m_chrParity = chrParity; }
	void setDeviceDataBits(SPORT_ID_ENUM nSport, int nDataBits) { m_deviceSettings[nSport].m_nDataBits = nDataBits; }
	void setDeviceStopBits(SPORT_ID_ENUM nSport, int nStopBits) { m_deviceSettings[nSport].m_nStopBits = nStopBits; }

	// ----

	void setLogFileLastPath(const QString &strLastPath) { m_strLogFileLastPath = strLastPath; }

	// ----

	void setFirmwareSportPort(SPORT_ID_ENUM nPort) { m_nFirmwareSportPort = nPort; }
	void setFirmwareLogTxEchos(bool bLogEchos) { m_bFirmwareLogTxEchos = bLogEchos; }
	// ----
	void setDataConfigSportPort(SPORT_ID_ENUM nPort) { m_nDataConfigSportPort = nPort; }
	void setDataConfigLogTxEchos(bool bLogEchos) { m_bDataConfigLogTxEchos = bLogEchos; }

	// ----

	void setFirmwareLastReadPath(const QString &strLastPath) { m_strFirmwareLastReadPath = strLastPath; }
	void setFirmwareLastWritePath(const QString &strLastPath) { m_strFirmwareLastWritePath = strLastPath; }

	// ----

	void setLuaScriptLastPath(const QString &strLastPath) { m_strLuaScriptLastPath = strLastPath; }
	void setLuaScreenTheme(int nTheme) { m_nLuaScreenTheme = nTheme; }

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

	// Comm Settings:
	// --------------
	SPORT_ID_ENUM m_nFirmwareSportPort;
	bool m_bFirmwareLogTxEchos;
	// ----
	SPORT_ID_ENUM m_nDataConfigSportPort;
	bool m_bDataConfigLogTxEchos;
	// ----

	// Firmware Settings:
	// ------------------
	QString m_strFirmwareLastReadPath;
	QString m_strFirmwareLastWritePath;
	// ----

	// Lua Script Settings:
	// --------------------
	QString m_strLuaScriptLastPath;
	int m_nLuaScreenTheme;

private:
};

// ============================================================================

#endif // CPERSISTENTSETTINGS_H

