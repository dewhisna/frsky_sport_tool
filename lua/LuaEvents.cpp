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

#include "LuaEvents.h"

#include "Lua_lrotable.h"

#include <QKeyEvent>

#include <assert.h>

// ============================================================================

namespace {
	static constexpr int LUA_REFRESH_RATE = 50;		// LUA Script Refresh rate in milliseconds
};

// ============================================================================

CLuaEvents::CLuaEvents(QObject *pParent)
	:	QObject(pParent)
{
	connect(&m_tmrRefresh, SIGNAL(timeout()), this, SLOT(refreshEvent()));
	m_tmrRefresh.start(LUA_REFRESH_RATE);
}

CLuaEvents::~CLuaEvents()
{
}

// ----------------------------------------------------------------------------

bool CLuaEvents::keyPressEvent(QKeyEvent *pEvent)
{
	assert(pEvent != nullptr);
	if (pEvent == nullptr) return false;

	event_t nEvent = keyToEvent(pEvent->key());
	if (nEvent == EVT_NONE) return false;			// Pass if it isn't an event we send to Lua

	// Send event if the key hasn't been killed:
	if (!m_setDeadKeyEvents.contains(nEvent)) {
		// If we haven't seen the key, raise first seen event:
		if (!m_setLiveKeyEvents.contains(nEvent)) {
			m_setLiveKeyEvents.insert(nEvent);			// Add it before we emit in case callee wants to kill it, it will be in the set
			emit luaEvent(EVT_KEY_FIRST(nEvent));
		} else if (pEvent->isAutoRepeat()) {
			// If this is an autoRepeat, the first time we will raise a long keypress:
			if (!m_setLongKeyEvents.contains(nEvent)) {
				m_setLongKeyEvents.insert(nEvent);		// Add it before we emit in case callee wants to kill it, it will be in the set
				emit luaEvent(EVT_KEY_LONG(nEvent));
			} else {
				// After that, we raise a repeat:
				emit luaEvent(EVT_KEY_REPT(nEvent));
			}
		}
	}

	return true;
}

bool CLuaEvents::keyReleaseEvent(QKeyEvent *pEvent)
{
	assert(pEvent != nullptr);
	if (pEvent == nullptr) return false;

	event_t nEvent = keyToEvent(pEvent->key());
	if (nEvent == EVT_NONE) return false;			// Pass if it isn't an event we send to Lua

	bool bRaiseBreak = !m_setDeadKeyEvents.remove(nEvent) && m_setLiveKeyEvents.remove(nEvent);
	m_setLongKeyEvents.remove(nEvent);

	// If it was a release that wasn't dead, raise a break:
	if (bRaiseBreak) emit luaEvent(EVT_KEY_BREAK(nEvent));

	return true;
}

void CLuaEvents::killKeyEvent(event_t nEvent)
{
	if (m_setLiveKeyEvents.remove(nEvent)) {
		m_setLongKeyEvents.remove(nEvent);
		m_setDeadKeyEvents.insert(nEvent);
	}
}

void CLuaEvents::refreshEvent()
{
	emit luaEvent(EVT_REFRESH);
}

// ----------------------------------------------------------------------------

bool CLuaEvents::isMaskableKey(event_t nEvent)
{
	return ((nEvent != KEY_EXIT) && (nEvent != KEY_ENTER));
}

// ----------------------------------------------------------------------------

const luaR_value_entry lua_opentx_const_events[] =
{
	{ "EVT_VIRTUAL_PREV", EVT_KEY_FIRST(KEY_UP) },
	{ "EVT_VIRTUAL_PREV_REPT", EVT_KEY_REPT(KEY_UP) },
	{ "EVT_VIRTUAL_NEXT", EVT_KEY_FIRST(KEY_DOWN) },
	{ "EVT_VIRTUAL_NEXT_REPT", EVT_KEY_REPT(KEY_DOWN) },
	{ "EVT_VIRTUAL_DEC", EVT_KEY_FIRST(KEY_DOWN) },
	{ "EVT_VIRTUAL_DEC_REPT", EVT_KEY_REPT(KEY_DOWN) },
	{ "EVT_VIRTUAL_INC", EVT_KEY_FIRST(KEY_UP) },
	{ "EVT_VIRTUAL_INC_REPT", EVT_KEY_REPT(KEY_UP) },
	// ----
	{ "EVT_VIRTUAL_PREV_PAGE", EVT_KEY_FIRST(KEY_PGUP) },
	{ "EVT_VIRTUAL_NEXT_PAGE", EVT_KEY_FIRST(KEY_PGDN) },
	{ "EVT_VIRTUAL_ENTER", EVT_KEY_BREAK(KEY_ENTER) },
	{ "EVT_VIRTUAL_ENTER_LONG", EVT_KEY_LONG(KEY_ENTER) },
	{ "EVT_VIRTUAL_EXIT", EVT_KEY_BREAK(KEY_EXIT) },
	{ "EVT_VIRTUAL_MENU", EVT_KEY_BREAK(KEY_RIGHT) },
	{ "EVT_VIRTUAL_MENU_LONG", EVT_KEY_LONG(KEY_RIGHT) },
	// ----
	{ nullptr, 0 }
};

event_t CLuaEvents::keyToEvent(int nQtKey)
{
	switch (nQtKey) {
		case Qt::Key_PageUp:
			return KEY_PGUP;
		case Qt::Key_PageDown:
			return KEY_PGDN;
		case Qt::Key_Return:		// Return is main keyboard
		case Qt::Key_Enter:			// Enter is Keypad
			return KEY_ENTER;
		case Qt::Key_Home:
			return KEY_MODEL;
		case Qt::Key_Up:
			return KEY_UP;
		case Qt::Key_Escape:
			return KEY_EXIT;
		case Qt::Key_Down:
			return KEY_DOWN;
		case Qt::Key_Tab:
			return KEY_TELEM;
		case Qt::Key_Right:
			return KEY_RIGHT;
		case Qt::Key_End:
			return KEY_RADIO;
		case Qt::Key_Left:
			return KEY_LEFT;
	}

	return EVT_NONE;
}

// ============================================================================
