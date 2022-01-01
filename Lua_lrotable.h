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

#ifndef LUA_lrotable_h
#define LUA_lrotable_h

extern "C" {

#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>

// Read-Only Tables for Lua

// ============================================================================

// A number entry in the read only table
typedef struct luaR_value_entry
{
  const char *name;
  lua_Number value;
} luaR_value_entry;

// ============================================================================

}	// extern "C"

#endif	// LUA_lrotable_h
