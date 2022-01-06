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

#ifndef LUA_GENERAL_H
#define LUA_GENERAL_H

#include <QObject>
#include <QPointer>
#include <QElapsedTimer>
#include <QQueue>

#include "LuaEvents.h"

#include "frsky_sport_io.h"

// Forward Declarations
extern "C" struct luaR_value_entry;
extern "C" struct luaL_Reg;

class CFrskySportDeviceTelemetry;

// ============================================================================

constexpr int TELEM_LABEL_LEN = 4;

enum TelemetryUnit {
	UNIT_RAW,
	UNIT_VOLTS,
	UNIT_AMPS,
	UNIT_MILLIAMPS,
	UNIT_KTS,
	UNIT_METERS_PER_SECOND,
	UNIT_FEET_PER_SECOND,
	UNIT_KMH,
	UNIT_SPEED = UNIT_KMH,
	UNIT_MPH,
	UNIT_METERS,
	UNIT_DIST = UNIT_METERS,
	UNIT_FEET,
	UNIT_CELSIUS,
	UNIT_TEMPERATURE = UNIT_CELSIUS,
	UNIT_FAHRENHEIT,
	UNIT_PERCENT,
	UNIT_MAH,
	UNIT_WATTS,
	UNIT_MILLIWATTS,
	UNIT_DB,
	UNIT_RPMS,
	UNIT_G,
	UNIT_DEGREE,
	UNIT_RADIANS,
	UNIT_MILLILITERS,
	UNIT_FLOZ,
	UNIT_MILLILITERS_PER_MINUTE,
	UNIT_HERTZ,
	UNIT_MS,
	UNIT_US,
	UNIT_KM,
	UNIT_DBM,
	UNIT_MAX = UNIT_DBM,
	UNIT_SPARE6,
	UNIT_SPARE7,
	UNIT_SPARE8,
	UNIT_SPARE9,
	UNIT_SPARE10,
	UNIT_HOURS,
	UNIT_MINUTES,
	UNIT_SECONDS,
	// FrSky format used for these fields, could be another format in the future
	UNIT_FIRST_VIRTUAL,
	UNIT_CELLS = UNIT_FIRST_VIRTUAL,
	UNIT_DATETIME,
	UNIT_GPS,
	UNIT_BITFIELD,
	UNIT_TEXT,
	// Internal units (not stored in sensor unit)
	UNIT_GPS_LONGITUDE,
	UNIT_GPS_LATITUDE,
	UNIT_DATETIME_YEAR,
	UNIT_DATETIME_DAY_MONTH,
	UNIT_DATETIME_HOUR_MIN,
	UNIT_DATETIME_SEC
};

// ============================================================================

class CLuaGeneral : public QObject
{
	Q_OBJECT

public:
	explicit CLuaGeneral(CFrskySportDeviceTelemetry *pTelemetry, QObject *pParent = nullptr);
	virtual ~CLuaGeneral();

	uint32_t getTimer10ms() const;

	bool sportPortIsOpen() const;
	bool haveRxSportPacket() const { return !m_queSportPackets.isEmpty(); }
	CSportTelemetryPacket popRxSportPacket() { return m_queSportPackets.dequeue(); }
	// ----
	bool haveTelemetryPoll(int nPhysicalId) const;
	bool isTelemetryPushAvailable(int nPhysicalId) const;

signals:
	void killKeyEvent(event_t nEvent);					// Signal for parent CLuaEvents::killKeyEvent
	// ----
	void sendTxSportPacket(const CSportTelemetryPacket &packet, const QString &strLogDetail = QString());
	void pushTxSportPacket(const CSportTelemetryPacket &packet, const QString &strLogDetail = QString());

private slots:
	void en_rxSportPacket(const CSportTelemetryPacket &packet);

private:
	QElapsedTimer m_tmrTickTimer;						// Used to provide the number of 10ms Ticks since "radio was started" for Lua getTime() function
	QQueue<CSportTelemetryPacket> m_queSportPackets;	// Queue of received telemetry packets (filtered for luaSportTelemetryPop)
	// ----
	QPointer<CFrskySportDeviceTelemetry> m_pTelemetry;	// Sport Telemetry Serial Handler

public:
	// Per thread Lua General -- one General on each thread running Lua:
	static thread_local QPointer<CLuaGeneral> g_luaGeneral;
};

// ----------------------------------------------------------------------------

extern "C" const luaR_value_entry lua_opentx_const_general[];			// Lua OpenTx General Constants
extern "C" const luaL_Reg lua_opentx_generalLib[];						// Lua OpenTx General functions

// ============================================================================

#endif	// LUA_GENERAL_H
