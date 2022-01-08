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

#ifndef LUA_EVENTS_H
#define LUA_EVENTS_H

#include <inttypes.h>

#include <QObject>
#include <QSet>
#include <QTimer>

// Forward Declarations
extern "C" struct luaR_value_entry;
class QKeyEvent;

// ============================================================================

typedef uint16_t event_t;
Q_DECLARE_METATYPE(event_t)

enum EnumLuaKeys
{
	KEY_PGUP,
	KEY_PGDN,
	KEY_ENTER,
	KEY_MODEL,
	KEY_UP = KEY_MODEL,
	KEY_EXIT,
	KEY_DOWN = KEY_EXIT,
	KEY_TELEM,
	KEY_RIGHT = KEY_TELEM,
	KEY_RADIO,
	KEY_LEFT = KEY_RADIO,

	TRM_BASE,
	TRM_LH_DWN = TRM_BASE,
	TRM_LH_UP,
	TRM_LV_DWN,
	TRM_LV_UP,
	TRM_RV_DWN,
	TRM_RV_UP,
	TRM_RH_DWN,
	TRM_RH_UP,
	TRM_LS_DWN,
	TRM_LS_UP,
	TRM_RS_DWN,
	TRM_RS_UP,
	TRM_LAST = TRM_RS_UP,

	NUM_KEYS
};

constexpr event_t EVT_KEY_MASK(event_t e) { return ((e) & 0x1f); }

constexpr event_t _MSK_KEY_BREAK =		0x0200;
constexpr event_t _MSK_KEY_REPT =		0x0400;
constexpr event_t _MSK_KEY_FIRST =		0x0600;
constexpr event_t _MSK_KEY_LONG =		0x0800;
constexpr event_t _MSK_KEY_FLAGS =		0x0e00;
constexpr event_t EVT_ENTRY =			0x1000;
constexpr event_t EVT_ENTRY_UP =		0x2000;

// normal order of events is: FIRST, LONG, REPEAT, REPEAT, ..., BREAK
constexpr event_t EVT_KEY_FIRST(event_t key) { return ((key)|_MSK_KEY_FIRST); }  // fired when key is pressed
constexpr event_t EVT_KEY_REPT(event_t key)  { return ((key)|_MSK_KEY_REPT); }   // fired when key is held pressed long enough, fires multiple times with increasing speed
constexpr event_t EVT_KEY_LONG(event_t key)  { return ((key)|_MSK_KEY_LONG); }   // fired when key is held pressed for a while
constexpr event_t EVT_KEY_BREAK(event_t key) { return ((key)|_MSK_KEY_BREAK); }  // fired when key is released (short or long), but only if the event was not killed

constexpr bool IS_KEY_FIRST(event_t evt) { return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_FIRST; }
constexpr bool IS_KEY_REPT(event_t evt)  { return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_REPT; }
constexpr bool IS_KEY_LONG(event_t evt)  { return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_LONG; }
constexpr bool IS_KEY_BREAK(event_t evt) { return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_BREAK; }
constexpr bool IS_KEY_EVT(event_t evt, uint8_t key) { return (evt & _MSK_KEY_FLAGS) && (EVT_KEY_MASK(evt) == key); }

constexpr event_t EVT_ROTARY_BREAK = EVT_KEY_BREAK(KEY_ENTER);
constexpr event_t EVT_ROTARY_LONG = EVT_KEY_LONG(KEY_ENTER);
constexpr event_t EVT_ROTARY_LEFT = 0xDF00;
constexpr event_t EVT_ROTARY_RIGHT = 0xDE00;
inline bool IS_NEXT_EVENT(event_t event) { return (event==EVT_ROTARY_RIGHT); }
inline bool IS_PREVIOUS_EVENT(event_t event) { return (event==EVT_ROTARY_LEFT); }

constexpr event_t EVT_REFRESH = 0xDD00;

constexpr event_t EVT_NONE = 0xFFFF;

// ============================================================================

class CLuaEvents : public QObject
{
	Q_OBJECT

public:
	explicit CLuaEvents(QObject *pParent = nullptr);
	virtual ~CLuaEvents();

	virtual bool keyPressEvent(QKeyEvent *pEvent);		// Inbound events from keyboard presses to trigger luaEvent emission, returns 'true' if the key was processed and shouldn't be sent to parent widget
	virtual bool keyReleaseEvent(QKeyEvent *pEvent);	// Inbound events from keyboard releases to trigger luaEvent emission, returns 'true' if the key was processed and shouldn't be sent to parent widget

public slots:
	virtual void killKeyEvent(event_t nEvent);			// Removes nEvent from m_setLiveKeyEvents, causing any pending keyReleaseEvent to be suppressed, adds it to m_setDeadKeyEvents to suppress mismatched press/release
	virtual void refreshEvent();						// Refresh Timer trigger slot that fires luaEvent with EVT_REFRESH

signals:
	void luaEvent(event_t nEvent);

public:
	static bool isMaskableKey(event_t nEvent);

protected:
	event_t keyToEvent(int nQtKey);

private:
	QSet<event_t> m_setLiveKeyEvents;		// Set of Key events firing (triggered by keyPressEvent, removed via keyReleaseEvent or killKeyEvent)
	QSet<event_t> m_setDeadKeyEvents;		// Set of Key events that have been killed -- used to prevent imbalance of press/release events to parent widgets
	QSet<event_t> m_setLongKeyEvents;		// Set of Key events that have triggered a Long from a repeat to prevent multiple long events when it's repeat events
	// ----
	QTimer m_tmrRefresh;
};

// ----------------------------------------------------------------------------

extern "C" const luaR_value_entry lua_opentx_const_events[];		// Lua OpenTx Constants for Events

// ============================================================================

#endif	// LUA_EVENTS_H

