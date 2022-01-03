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

#include "PersistentSettings.h"

#include <assert.h>

// ============================================================================

namespace {
	const QString conarrstrSportSettingsGroup[SPIDE_COUNT] = { "Sport1", "Sport2", };
	// ----
	const QString constrSerialPortKey("SerialPort");
	const QString constrBaudRateKey("BaudRate");
	const QString constrDataBitsKey("DataBits");
	const QString constrParityKey("Parity");
	const QString constrStopBitsKey("StopBits");
	// ----
	const QString constrLogFileSettingsGroup("LogFile");
	const QString constrLogFileLastPathKey("LastPath");
	// ----
	const QString constrFirmwareCommGroup("FirmwareComm");
	const QString constrDataConfigCommGroup("DataConfigComm");
	// ----
	const QString constrSportPortKey("SportPort");
	const QString constrLogTxEchosKey("LogTxEchos");
	// ----
	const QString constrFirmwareGroup("Firmware");
	const QString constrLastReadPathKey("LastReadPath");
	const QString constrLastWritePathKey("LastWritePath");
	// ----
	const QString constrLuaScriptGroup("LuaScript");
	const QString constrLuaScriptLastPathKey("LastPath");
	const QString constrLuaScreenThemeKey("ScreenTheme");
	// ----

	// ------------------------------------------------------------------------

	const DEVICE_SETTINGS conarrDefaultDeviceSettings[SPIDE_COUNT] =
	{
		// Sport1:
		{
			QString(),					// SerialPort
			57600,						// BaudRate
			'N',						// Parity
			8,							// DataBits
			1,							// StopBits
		},

		// Sport2:
		{
			QString(),					// SerialPort
			57600,						// BaudRate
			'N',						// Parity
			8,							// DataBits
			1,							// StopBits
		},
	};

};

// ============================================================================

CPersistentSettings::CPersistentSettings(QObject *parent)
	:	QSettings(parent),
		m_nFirmwareSportPort(SPIDE_SPORT1),
		m_bFirmwareLogTxEchos(false),
		m_nDataConfigSportPort(SPIDE_SPORT2),
		m_bDataConfigLogTxEchos(false),
		m_nLuaScreenTheme(0)
{
	for (int nSport = 0; nSport < SPIDE_COUNT; ++nSport) {
		m_deviceSettings[nSport] = conarrDefaultDeviceSettings[nSport];
	}
}

CPersistentSettings::~CPersistentSettings()
{
}

CPersistentSettings *CPersistentSettings::instance()
{
	static CPersistentSettings thePersistentSettings;
	return &thePersistentSettings;
}

void CPersistentSettings::saveSettings()
{
	// Device Settings:
	// ----------------
	for (int nSport = 0; nSport < SPIDE_COUNT; ++nSport) {
		beginGroup(conarrstrSportSettingsGroup[nSport]);
		setValue(constrSerialPortKey, m_deviceSettings[nSport].m_strSerialPort);
		setValue(constrBaudRateKey, m_deviceSettings[nSport].m_nBaudRate);
		setValue(constrDataBitsKey, m_deviceSettings[nSport].m_nDataBits);
		setValue(constrParityKey, m_deviceSettings[nSport].m_chrParity);
		setValue(constrStopBitsKey, m_deviceSettings[nSport].m_nStopBits);
		endGroup();
	}

	// LogFile Settings:
	// -----------------
	beginGroup(constrLogFileSettingsGroup);
	setValue(constrLogFileLastPathKey, m_strLogFileLastPath);
	endGroup();

	// Comm Settings:
	// --------------
	beginGroup(constrFirmwareCommGroup);
	setValue(constrSportPortKey, m_nFirmwareSportPort);
	setValue(constrLogTxEchosKey, m_bFirmwareLogTxEchos);
	endGroup();
	// ----
	beginGroup(constrDataConfigCommGroup);
	setValue(constrSportPortKey, m_nDataConfigSportPort);
	setValue(constrLogTxEchosKey, m_bDataConfigLogTxEchos);
	endGroup();

	// Firmware Settings:
	// ------------------
	beginGroup(constrFirmwareGroup);
	setValue(constrLastReadPathKey, m_strFirmwareLastReadPath);
	setValue(constrLastWritePathKey, m_strFirmwareLastWritePath);
	endGroup();

	// Lua Script Settings:
	// --------------------
	beginGroup(constrLuaScriptGroup);
	setValue(constrLuaScriptLastPathKey, m_strLuaScriptLastPath);
	setValue(constrLuaScreenThemeKey, m_nLuaScreenTheme);
	endGroup();
}

void CPersistentSettings::loadSettings()
{
	// Device Settings:
	// ----------------
	for (int nSport = 0; nSport < SPIDE_COUNT; ++nSport) {
		beginGroup(conarrstrSportSettingsGroup[nSport]);
		m_deviceSettings[nSport].m_strSerialPort = value(constrSerialPortKey, m_deviceSettings[nSport].m_strSerialPort).toString();
		m_deviceSettings[nSport].m_nBaudRate = value(constrBaudRateKey, m_deviceSettings[nSport].m_nBaudRate).toInt();
		m_deviceSettings[nSport].m_nDataBits = value(constrDataBitsKey, m_deviceSettings[nSport].m_nDataBits).toInt();
		m_deviceSettings[nSport].m_chrParity = value(constrParityKey, m_deviceSettings[nSport].m_chrParity).toInt();
		m_deviceSettings[nSport].m_nStopBits = value(constrStopBitsKey, m_deviceSettings[nSport].m_nStopBits).toInt();
		endGroup();
	}

	// LogFile Settings:
	// -----------------
	beginGroup(constrLogFileSettingsGroup);
	m_strLogFileLastPath = value(constrLogFileLastPathKey, m_strLogFileLastPath).toString();
	endGroup();

	// Comm Settings:
	// --------------
	beginGroup(constrFirmwareCommGroup);
	m_nFirmwareSportPort = static_cast<SPORT_ID_ENUM>(value(constrSportPortKey, m_nFirmwareSportPort).toInt());
	m_bFirmwareLogTxEchos = value(constrLogTxEchosKey, m_bFirmwareLogTxEchos).toBool();
	endGroup();
	// ----
	beginGroup(constrDataConfigCommGroup);
	m_nDataConfigSportPort = static_cast<SPORT_ID_ENUM>(value(constrSportPortKey, m_nDataConfigSportPort).toInt());
	m_bDataConfigLogTxEchos = value(constrLogTxEchosKey, m_bDataConfigLogTxEchos).toBool();
	endGroup();

	// Firmware Settings:
	// ------------------
	beginGroup(constrFirmwareGroup);
	m_strFirmwareLastReadPath = value(constrLastReadPathKey, m_strFirmwareLastReadPath).toString();
	m_strFirmwareLastWritePath = value(constrLastWritePathKey, m_strFirmwareLastWritePath).toString();
	endGroup();

	// Lua Script Settings:
	// --------------------
	beginGroup(constrLuaScriptGroup);
	m_strLuaScriptLastPath = value(constrLuaScriptLastPathKey, m_strLuaScriptLastPath).toString();
	m_nLuaScreenTheme = value(constrLuaScreenThemeKey, m_nLuaScreenTheme).toInt();
	endGroup();
}

// ----------------------------------------------------------------------------

// ============================================================================
