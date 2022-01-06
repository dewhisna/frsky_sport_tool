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

#include "LuaGeneral.h"

#include "frsky_sport_telemetry.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

#include "Lua_lrotable.h"

// ============================================================================

thread_local QPointer<CLuaGeneral> CLuaGeneral::g_luaGeneral;

// ============================================================================

CLuaGeneral::CLuaGeneral(CFrskySportDeviceTelemetry *pTelemetry, QObject *pParent)
	:	QObject(pParent),
		m_pTelemetry(pTelemetry)
{
	assert(!m_pTelemetry.isNull());

	connect(this, SIGNAL(sendTxSportPacket(const CSportTelemetryPacket&, const QString&)),
			m_pTelemetry, SLOT(txSportPacket(const CSportTelemetryPacket&, const QString&)));	// Outgoing immediate
	connect(this, SIGNAL(pushTxSportPacket(const CSportTelemetryPacket&, const QString&)),
			m_pTelemetry, SLOT(pushTelemetryResponse(const CSportTelemetryPacket &, const QString&)));	// Outgoing push response
	connect(m_pTelemetry, SIGNAL(rxSportPacket(const CSportTelemetryPacket&)),
			this, SLOT(en_rxSportPacket(const CSportTelemetryPacket&)));						// Incoming

	m_tmrTickTimer.start();			// Start the timer -- i.e. "turn on the radio"

	// Set the Lua General on this thread to be this one:
	g_luaGeneral = this;
}

CLuaGeneral::~CLuaGeneral()
{
}

bool CLuaGeneral::sportPortIsOpen() const
{
	return (!m_pTelemetry.isNull() && m_pTelemetry->isOpen());
}

bool CLuaGeneral::haveTelemetryPoll(int nPhysicalId) const
{
	if (m_pTelemetry.isNull()) return false;
	return m_pTelemetry->haveTelemetryPoll(nPhysicalId);
}

bool CLuaGeneral::isTelemetryPushAvailable(int nPhysicalId) const
{
	if (m_pTelemetry.isNull()) return false;
	return m_pTelemetry->isTelemetryPushAvailable(nPhysicalId);
}

uint32_t CLuaGeneral::getTimer10ms() const
{
	return m_tmrTickTimer.nsecsElapsed()/10000000;		// Convert 1nsecs to 10msec
}

void CLuaGeneral::en_rxSportPacket(const CSportTelemetryPacket &packet)
{
	// Queue a received SPORT packet from the queue. Please note that only packets
	//	using a data ID within 0x5000 to 0x50FF (frame ID == 0x10), as well as
	//	packets with a frame ID equal 0x32 (regardless of the data ID) will be
	//	passed to the LUA telemetry receive queue.
	if ((packet.getPrimId() == PRIM_ID_SERVER_RESP_CAL_FRAME) ||
		((packet.getPrimId() == PRIM_ID_DATA_FRAME) &&
		 (packet.getDataId() >= DATA_ID_DIY_STREAM_FIRST) &&
		 (packet.getDataId() <= DATA_ID_DIY_STREAM_LAST))) {
		m_queSportPackets.enqueue(packet);
	}
}

// ============================================================================

// getTime()
//
// Return the time since the radio was started in multiple of 10ms
//
// retval: number Number of 10ms ticks since the radio was started Example:
//	run time: 12.54 seconds, return value: 1254
//
//	The timer internally uses a 32-bit counter which is enough for 497 days so
//	overflows will not happen.
static int luaGetTime(lua_State * L)
{
	if (!CLuaGeneral::g_luaGeneral.isNull()) {
		lua_pushunsigned(L, CLuaGeneral::g_luaGeneral->getTimer10ms());
	} else {
		lua_pushunsigned(L, 0);
	}
	return 1;
}


// killEvents(key)
//
// Stops key state machine. See [Key Events](../key_events.md) for the detailed description.
//
//	key (number) key to be killed, can also include event type (only the key part is used)
static int luaKillEvents(lua_State * L)
{
	uint8_t key = EVT_KEY_MASK(luaL_checkinteger(L, 1));
	// prevent killing maskable keys (only in telemetry scripts)
	// TODO add which type of script is running before p_call()
	if (!CLuaGeneral::g_luaGeneral.isNull() && CLuaEvents::isMaskableKey(key)) {
		emit CLuaGeneral::g_luaGeneral->killKeyEvent(key);
	}
	return 0;
}


// ----------------------------------------------------------------------------

// sportTelemetryPop()
//
// Pops a received SPORT packet from the queue. Please note that only packets
//	using a data ID within 0x5000 to 0x50FF (frame ID == 0x10), as well as
//	packets with a frame ID equal 0x32 (regardless of the data ID) will be
//	passed to the LUA telemetry receive queue.
//
// retval: nil queue does not contain any (or enough) bytes to form a whole packet
//
// retval: multiple returns 4 values:
//			* sensor ID (number)
//			* frame ID (number)
//			* data ID (number)
//			* value (number)
static int luaSportTelemetryPop(lua_State * L)
{
	if (CLuaGeneral::g_luaGeneral.isNull()) return 0;

	if (CLuaGeneral::g_luaGeneral->haveRxSportPacket()) {
		CSportTelemetryPacket packet = CLuaGeneral::g_luaGeneral->popRxSportPacket();
		lua_pushnumber(L, packet.getPhysicalId());
		lua_pushnumber(L, packet.getPrimId());
		lua_pushnumber(L, packet.getDataId());
		lua_pushunsigned(L, packet.getValue());
		return 4;
	}

	return 0;
}


// sportTelemetryPush([sensorId, frameId, dataId, value])
//
// This functions allows for sending SPORT telemetry data toward the receiver,
//	and more generally, to anything connected SPORT bus on the receiver or transmitter.
//
// When called without parameters, it will only return the status of the output
//	buffer without sending anything.
//
//	sensorId  physical sensor ID
//	frameId   frame ID
//	dataId    data ID
//	value     value
//
// retval: boolean  data queued in output buffer or not.
// retval: nil      incorrect telemetry protocol.
static int luaSportTelemetryPush(lua_State * L)
{
	// This is a Sport Tool, so we will always be on the
	//	correct telemetry protocol ... so this retval
	//	will be whether or not we have a LuaGeneral:
	//	if (!IS_FRSKY_SPORT_PROTOCOL()) {
	if (CLuaGeneral::g_luaGeneral.isNull()) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_gettop(L) == 0) {
		lua_pushboolean(L, CLuaGeneral::g_luaGeneral->sportPortIsOpen());
		return 1;
	} else if (lua_gettop(L) > int(sizeof(CSportTelemetryPacket))) {
		lua_pushboolean(L, false);
		return 1;
	}

	uint8_t nPhysicalId = luaL_checkunsigned(L, 1);
	if ((nPhysicalId < TELEMETRY_PHYS_ID_COUNT) &&
		(CLuaGeneral::g_luaGeneral->sportPortIsOpen())) {
		CSportTelemetryPacket packet(	nPhysicalId,	// PhysId
										luaL_checkunsigned(L, 2),	// PrimId
										luaL_checkunsigned(L, 3),	// DataId
										luaL_checkunsigned(L, 4));	// Value

		if (CLuaGeneral::g_luaGeneral->haveTelemetryPoll(nPhysicalId) &&
			CLuaGeneral::g_luaGeneral->isTelemetryPushAvailable(nPhysicalId)) {
			// sensor is found, we queue it to transmit on push
			emit CLuaGeneral::g_luaGeneral->pushTxSportPacket(packet, "From Lua Script");
		} else {
			// sensor not found, we send the frame to the SPORT line
			emit CLuaGeneral::g_luaGeneral->sendTxSportPacket(packet, "From Lua Script");
		}

		lua_pushboolean(L, true);
		return 1;
	}

	lua_pushboolean(L, false);
	return 1;
}


// ============================================================================

const luaL_Reg lua_opentx_generalLib[] = {
	{ "getTime", luaGetTime },
//	{ "getDateTime", luaGetDateTime },
#if defined(RTCLOCK)
//	{ "getRtcTime", luaGetRtcTime },
#endif
//	{ "getVersion", luaGetVersion },
//	{ "getGeneralSettings", luaGetGeneralSettings },
//	{ "getGlobalTimer", luaGetGlobalTimer },
//	{ "getRotEncSpeed", luaGetRotEncSpeed },
//	{ "getValue", luaGetValue },
//	{ "getRAS", luaGetRAS },
//	{ "getTxGPS", luaGetTxGPS },
//	{ "getFieldInfo", luaGetFieldInfo },
//	{ "getFlightMode", luaGetFlightMode },
//	{ "playFile", luaPlayFile },
//	{ "playNumber", luaPlayNumber },
//	{ "playDuration", luaPlayDuration },
//	{ "playTone", luaPlayTone },
//	{ "playHaptic", luaPlayHaptic },
//	{ "flushAudio", luaFlushAudio },
//	// { "popupInput", luaPopupInput },
//	{ "popupWarning", luaPopupWarning },
//	{ "popupConfirmation", luaPopupConfirmation },
//	{ "defaultStick", luaDefaultStick },
//	{ "defaultChannel", luaDefaultChannel },
//	{ "getRSSI", luaGetRSSI },
	{ "killEvents", luaKillEvents },
//	{ "chdir", luaChdir },
//	{ "loadScript", luaLoadScript },
//	{ "getUsage", luaGetUsage },
//	{ "getAvailableMemory", luaGetAvailableMemory },
//	{ "resetGlobalTimer", luaResetGlobalTimer },
#if defined(PXX2)
//	{ "accessTelemetryPush", luaAccessTelemetryPush },
#endif
	{ "sportTelemetryPop", luaSportTelemetryPop },
	{ "sportTelemetryPush", luaSportTelemetryPush },
//	{ "setTelemetryValue", luaSetTelemetryValue },
#if defined(CROSSFIRE)
//	{ "crossfireTelemetryPop", luaCrossfireTelemetryPop },
//	{ "crossfireTelemetryPush", luaCrossfireTelemetryPush },
#endif
#if defined(RADIO_FAMILY_TBS)
//	{ "SetDevId", luaSetDevId },
//	{ "GetDevId", luaGetDevId },
#endif
#if defined(MULTIMODULE)
//	{ "multiBuffer", luaMultiBuffer },
#endif
//	{ "setSerialBaudrate", luaSetSerialBaudrate },
//	{ "serialWrite", luaSerialWrite },
//	{ "serialRead", luaSerialRead },

	// ----
	{ nullptr, nullptr }	// sentinel
};


// ----------------------------------------------------------------------------

const luaR_value_entry lua_opentx_const_general[] =
{
	{"UNIT_RAW", UNIT_RAW },
	{"UNIT_VOLTS", UNIT_VOLTS },
	{"UNIT_AMPS", UNIT_AMPS },
	{"UNIT_MILLIAMPS", UNIT_MILLIAMPS },
	{"UNIT_KTS", UNIT_KTS },
	{"UNIT_METERS_PER_SECOND", UNIT_METERS_PER_SECOND },
	{"UNIT_FEET_PER_SECOND", UNIT_FEET_PER_SECOND },
	{"UNIT_KMH", UNIT_KMH },
	{"UNIT_MPH", UNIT_MPH },
	{"UNIT_METERS", UNIT_METERS },
	{"UNIT_KM", UNIT_KM },
	{"UNIT_FEET", UNIT_FEET },
	{"UNIT_CELSIUS", UNIT_CELSIUS },
	{"UNIT_FAHRENHEIT", UNIT_FAHRENHEIT },
	{"UNIT_PERCENT", UNIT_PERCENT },
	{"UNIT_MAH", UNIT_MAH },
	{"UNIT_WATTS", UNIT_WATTS },
	{"UNIT_MILLIWATTS", UNIT_MILLIWATTS },
	{"UNIT_DB", UNIT_DB },
	{"UNIT_RPMS", UNIT_RPMS },
	{"UNIT_G", UNIT_G },
	{"UNIT_DEGREE", UNIT_DEGREE },
	{"UNIT_RADIANS", UNIT_RADIANS },
	{"UNIT_MILLILITERS", UNIT_MILLILITERS },
	{"UNIT_FLOZ", UNIT_FLOZ },
	{"UNIT_MILLILITERS_PER_MINUTE", UNIT_MILLILITERS_PER_MINUTE },
	{"UNIT_HERTZ", UNIT_HERTZ },
	{"UNIT_MS", UNIT_MS },
	{"UNIT_US", UNIT_US },
	{"UNIT_HOURS", UNIT_HOURS },
	{"UNIT_MINUTES", UNIT_MINUTES },
	{"UNIT_SECONDS", UNIT_SECONDS },
	{"UNIT_CELLS", UNIT_CELLS},
	{"UNIT_DATETIME", UNIT_DATETIME},
	{"UNIT_GPS", UNIT_GPS},
	{"UNIT_BITFIELD", UNIT_BITFIELD},
	{"UNIT_TEXT", UNIT_TEXT},
	// ----
	{ nullptr, 0 }
};

// ============================================================================
