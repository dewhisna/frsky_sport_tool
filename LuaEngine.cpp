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

#include "LuaEngine.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

#include <QMessageBox>

// ============================================================================

thread_local lua_State *CLuaEngine::g_pLSScripts = nullptr;
thread_local CLuaEngine::InterpretterState CLuaEngine::g_luaState = CLuaEngine::INTERPRETER_NOT_RUNNING;

class CLuaPanic
{
};

#define LUA_TRY try
#define LUA_CATCH(code) catch(const CLuaPanic &) { code }

// ============================================================================

int CLuaEngine::luaPanic(lua_State *pState)
{
	const char *msg = lua_tostring(pState, -1);
	if (msg == nullptr) msg = "error object is not a string";

	QMessageBox::critical(nullptr, QObject::tr("Lua Script Panic Error", "CLuaEngine"),
							QObject::tr("Lua Script Panic:", "CLuaEngine") + "\n\n" + msg);

	throw CLuaPanic();
	return 0;
}

void CLuaEngine::luaDisable()
{
	g_luaState = INTERPRETER_PANIC;
}

void CLuaEngine::luaClose(lua_State **ppState)
{
	assert(ppState != nullptr);
	if (*ppState) {
		LUA_TRY {
			lua_close(*ppState);		// This shouldn't cause a panic, but use try/catch just in case
		} LUA_CATCH(
			if (*ppState == g_pLSScripts) {
				luaDisable();
			}
		)
	}
	*ppState = nullptr;
}

void CLuaEngine::luaRegisterLibraries(lua_State *pState)
{
	luaL_openlibs(pState);
// TODO : Figure out
//	registerBitmapClass(pState);
}

CLuaEngine::ScriptState CLuaEngine::luaLoadScriptFileToState(lua_State *pState, const char *pFilename)
{
	if (g_luaState == INTERPRETER_PANIC) {
		return SCRIPT_PANIC;
	} else if (pFilename == nullptr) {
		return SCRIPT_NOFILE;
	}

	int lstatus;
	ScriptState ret = SCRIPT_NOFILE;

	// we don't pass <mode> on to loadfilex() because we want lua to load whatever file we specify, regardless of content
	lstatus = luaL_loadfilex(pState, pFilename, nullptr);
	if (lstatus == LUA_OK) {
		ret = SCRIPT_OK;
	} else {
		if (lstatus == LUA_ERRFILE) {
			ret = SCRIPT_NOFILE;
		} else if (lstatus == LUA_ERRSYNTAX) {
			ret = SCRIPT_SYNTAX_ERROR;
		} else {	//  LUA_ERRMEM or LUA_ERRGCMM
			ret = SCRIPT_PANIC;
		}
	}

  	return ret;
}

CLuaEngine::ScriptState CLuaEngine::luaLoad(lua_State *pState, const char *pFilename,
					TScriptInternalData & sid, TScriptInputsOutputs * sio)
{
	int init = 0;
	int lstatus = 0;

	sid.m_instructions = 0;
	sid.m_state = SCRIPT_OK;

	if (sio) *sio = TScriptInputsOutputs();

	if (g_luaState == INTERPRETER_PANIC) {
		return SCRIPT_PANIC;
	}

//	luaSetInstructionsLimit(L, MANUAL_SCRIPTS_MAX_INSTRUCTIONS);

	LUA_TRY {
		sid.m_state = luaLoadScriptFileToState(pState, pFilename);
		if ((sid.m_state == SCRIPT_OK) &&
			((lstatus = lua_pcall(pState, 0, 1, 0)) == LUA_OK) &&
			lua_istable(pState, -1)) {
			for (lua_pushnil(pState); lua_next(pState, -2); lua_pop(pState, 1)) {
				const char * key = lua_tostring(pState, -2);
				if (!strcmp(key, "init")) {
					init = luaL_ref(pState, LUA_REGISTRYINDEX);
					lua_pushnil(pState);
				} else if (!strcmp(key, "run")) {
					sid.m_run = luaL_ref(pState, LUA_REGISTRYINDEX);
					lua_pushnil(pState);
				} else if (!strcmp(key, "background")) {
					sid.m_background = luaL_ref(pState, LUA_REGISTRYINDEX);
					lua_pushnil(pState);
				} else if (sio && !strcmp(key, "input")) {
					luaGetInputs(pState, *sio);
				} else if (sio && !strcmp(key, "output")) {
					luaGetOutputs(pState, *sio);
				}
			}

			if (init) {
				lua_rawgeti(pState, LUA_REGISTRYINDEX, init);
				if (lua_pcall(pState, 0, 0, 0) != 0) {
					sid.m_state = SCRIPT_SYNTAX_ERROR;
				}
				luaL_unref(pState, LUA_REGISTRYINDEX, init);
				lua_gc(pState, LUA_GCCOLLECT, 0);
			}
		} else if (sid.m_state == SCRIPT_OK) {
			sid.m_state = SCRIPT_SYNTAX_ERROR;
		}
	} LUA_CATCH(
		luaDisable();
		return SCRIPT_PANIC;
	)

	if (sid.m_state != SCRIPT_OK) {
		luaFree(pState, sid);
	}

	luaDoGc(pState, true);

	return sid.m_state;
}

int CLuaEngine::luaGetInputs(lua_State * pState, TScriptInputsOutputs & sio)
{
	if (!lua_istable(pState, -1))
		return -1;

	sio.m_inputsCount = 0;
	for (lua_pushnil(pState); lua_next(pState, -2); lua_pop(pState, 1)) {
		luaL_checktype(pState, -2, LUA_TNUMBER);	// key is number
		luaL_checktype(pState, -1, LUA_TTABLE);		// value is table
		if (sio.m_inputsCount < MAX_SCRIPT_INPUTS) {
			uint8_t field = 0;
			int type = 0;
			TScriptInput * si = &sio.m_inputs[sio.m_inputsCount];
			for (lua_pushnil(pState); (lua_next(pState, -2) && (field<5)); lua_pop(pState, 1), field++) {
				switch (field) {
					case 0:
						luaL_checktype(pState, -2, LUA_TNUMBER);	// key is number
						luaL_checktype(pState, -1, LUA_TSTRING);	// value is string
						si->name = lua_tostring(pState, -1);
						break;
					case 1:
						luaL_checktype(pState, -2, LUA_TNUMBER);	// key is number
						luaL_checktype(pState, -1, LUA_TNUMBER);	// value is number
						type = lua_tointeger(pState, -1);
						if ((type >= INPUT_TYPE_FIRST) && (type <= INPUT_TYPE_LAST)) {
							si->type = type;
						}
						if (si->type == INPUT_TYPE_VALUE) {
							si->min = -100;
							si->max = 100;
						} else {
							si->max = 60;	// MIXSRC_LAST_TELEM;			// TODO : Figure out the correct value to use here with no radio
						}
						break;
					case 2:
						luaL_checktype(pState, -2, LUA_TNUMBER);	// key is number
						luaL_checktype(pState, -1, LUA_TNUMBER);	// value is number
						if (si->type == INPUT_TYPE_VALUE) {
							si->min = lua_tointeger(pState, -1);
						}
						break;
					case 3:
						luaL_checktype(pState, -2, LUA_TNUMBER);	// key is number
						luaL_checktype(pState, -1, LUA_TNUMBER);	// value is number
						if (si->type == INPUT_TYPE_VALUE) {
							si->max = lua_tointeger(pState, -1);
						}
						break;
					case 4:
						luaL_checktype(pState, -2, LUA_TNUMBER);	// key is number
						luaL_checktype(pState, -1, LUA_TNUMBER);	// value is number
						if (si->type == INPUT_TYPE_VALUE) {
							si->def = lua_tointeger(pState, -1);
						}
						break;
				}
			}
			sio.m_inputsCount++;
		}
	}

	return 0;
}


int CLuaEngine::luaGetOutputs(lua_State * pState, TScriptInputsOutputs & sio)
{
	if (!lua_istable(pState, -1))
		return -1;

	sio.m_outputsCount = 0;
	for (lua_pushnil(pState); lua_next(pState, -2); lua_pop(pState, 1)) {
		luaL_checktype(pState, -2, LUA_TNUMBER);	// key is number
		luaL_checktype(pState, -1, LUA_TSTRING);	// value is string
		if (sio.m_outputsCount < MAX_SCRIPT_OUTPUTS) {
			sio.m_outputs[sio.m_outputsCount++].name = lua_tostring(pState, -1);
		}
	}

	return 0;
}

void CLuaEngine::luaFree(lua_State * pState, TScriptInternalData & sid)
{
	LUA_TRY {
		if (sid.m_run) {
			luaL_unref(pState, LUA_REGISTRYINDEX, sid.m_run);
			sid.m_run = 0;
		}
		if (sid.m_background) {
			luaL_unref(pState, LUA_REGISTRYINDEX, sid.m_background);
			sid.m_background = 0;
		}
	} LUA_CATCH(
		luaDisable();
	)

	luaDoGc(pState, true);
}

void CLuaEngine::luaDoGc(lua_State * pState, bool bFull)
{
	if (pState) {
		LUA_TRY {
			if (bFull) {
				lua_gc(pState, LUA_GCCOLLECT, 0);
			} else {
				lua_gc(pState, LUA_GCSTEP, 10);
			}
		} LUA_CATCH(
			// we disable Lua for the rest of the session
			if (pState == g_pLSScripts) luaDisable();
//			if (pState == g_pLSWidgets) g_pLSWidgets = nullptr;		// TODO : Figure this out
		)
	}
}

// ----------------------------------------------------------------------------

void CLuaEngine::luaInit()
{
//	std::function<void (lua_State *pState)> f_panic = [this](lua_State *pState) { luaPanic(pState); };

	luaClose(&g_pLSScripts);

	if (g_luaState != INTERPRETER_PANIC) {
		g_pLSScripts = luaL_newstate();

		if (g_pLSScripts) {
			// install our panic handler
//			Callback<int(lua_State*)>::func = std::bind(&CLuaEngine::luaPanic, this, std::placeholders::_1);
//			lua_CFunction func = static_cast<lua_CFunction>(Callback<int(lua_State*)>::callback);
//			lua_atpanic(g_pLSScripts, func);

			lua_atpanic(g_pLSScripts, luaPanic);

			LUA_TRY {
				luaRegisterLibraries(g_pLSScripts);
			} LUA_CATCH(
				luaDisable();
			)
		} else {
			luaDisable();
		}
	}
}

bool CLuaEngine::luaExec(const char *pFilename, TScriptInternalData &sid)
{
	init();

	if (g_luaState != INTERPRETER_PANIC) {
		sid.m_state = SCRIPT_NOFILE;
		ScriptState result = luaLoad(g_pLSScripts, pFilename, sid);
		// TODO the same with run ...
		if (result == SCRIPT_OK) {
			g_luaState = INTERPRETER_RUNNING_STANDALONE_SCRIPT;
		} else {
			luaError(g_pLSScripts, result);
			g_luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
		}
	}

	return (g_luaState == INTERPRETER_RUNNING_STANDALONE_SCRIPT);
}

void CLuaEngine::luaError(lua_State * pState, ScriptState nError, const QString &strExtraMsg, bool bAcknowledge)
{
	QString strErrorTitle;

	switch (nError) {
		case SCRIPT_SYNTAX_ERROR:
			strErrorTitle = tr("Script syntax error");
			break;
		case SCRIPT_KILLED:
			strErrorTitle = tr("Script killed");
			break;
		case SCRIPT_PANIC:
			strErrorTitle = tr("Script panic");
			break;
		default:
			strErrorTitle = tr("Unknown error");
			break;
	}

	const char *pMsg = lua_tostring(pState, -1);
	QString strMsg = strExtraMsg;
	if (pMsg) {
		QString strTemp = QString::fromUtf8(pMsg);		// TODO : UTF8 or Latin1 here?
		if (!strMsg.isEmpty() && !strTemp.isEmpty()) strMsg += "\n\n";
		strMsg += strTemp;
	}

	if (strMsg.isEmpty()) strMsg = strErrorTitle;

	error(strErrorTitle, strMsg, bAcknowledge);
}

// ----------------------------------------------------------------------------

void CLuaEngine::init()
{
	luaInit();
}

bool CLuaEngine::exec(const char *pFilename)
{
	return luaExec(pFilename, m_standaloneScript);
}

void CLuaEngine::error(const QString &strTitle, const QString &strMessage, bool bAcknowledge)
{
	if (bAcknowledge) {
		QMessageBox::warning(m_pParentWidget, strTitle, strMessage);
	} else {
		QMessageBox::warning(m_pParentWidget, strTitle, strMessage);		// TODO : Differentiate between Acknowledge true/false
	}
}

// ============================================================================


CLuaEngine::CLuaEngine(QWidget *pParent)
	:	QObject(pParent),
		m_pParentWidget(pParent)
{
}

CLuaEngine::~CLuaEngine()
{
	if (g_pLSScripts) {
		if (m_standaloneScript.m_state == SCRIPT_OK) {
			luaFree(g_pLSScripts, m_standaloneScript);
		}

		luaDoGc(g_pLSScripts, true);

		luaClose(&g_pLSScripts);
	}
}

// ============================================================================

void CLuaEngine::execLuaScript(const QString &strFilename)
{
	if (strFilename.isEmpty()) return;

	exec(strFilename.toUtf8().data());
}

void CLuaEngine::runLuaScript(event_t nEvt)
{
	QString strErrorMsg;

	if (m_standaloneScript.m_state != SCRIPT_OK) return;

	LUA_TRY {
		if (m_standaloneScript.m_run) {
//			luaSetInstructionsLimit(lsScripts, MANUAL_SCRIPTS_MAX_INSTRUCTIONS);
			lua_rawgeti(g_pLSScripts, LUA_REGISTRYINDEX, m_standaloneScript.m_run);
			lua_pushunsigned(g_pLSScripts, nEvt);
			if (lua_pcall(g_pLSScripts, 1, 1, 0) == 0) {
				if (!lua_isnumber(g_pLSScripts, -1)) {
//					if (instructionsPercent > 100) {
//						TRACE("Script killed");
//						m_standaloneScript.m_state = SCRIPT_KILLED;
//					} else
					if (lua_isstring(g_pLSScripts, -1)) {
						// TODO : Should this be UTF8 or Latin1?
						QString strNextScript = QString::fromUtf8(lua_tostring(g_pLSScripts, -1));
						exec(strNextScript.toUtf8().data());
					} else {
						strErrorMsg = tr("Script run function returned unexpected value");
						m_standaloneScript.m_state = SCRIPT_SYNTAX_ERROR;
					}
				} else {
					int nScriptResult = lua_tointeger(g_pLSScripts, -1);
					lua_pop(g_pLSScripts, 1);	//  pop returned value
					if (nScriptResult != 0) {
						strErrorMsg = tr("Script finished with status %1").arg(nScriptResult);
						m_standaloneScript.m_state = SCRIPT_NOFILE;
					}
				}
			} else {
				strErrorMsg = tr("Script error: %1").arg(lua_tostring(g_pLSScripts, -1));
//				m_standaloneScript.m_state = (instructionsPercent > 100 ? SCRIPT_KILLED : SCRIPT_SYNTAX_ERROR);
				m_standaloneScript.m_state = SCRIPT_SYNTAX_ERROR;
			}

			if (nEvt == EVT_KEY_LONG(KEY_EXIT)) {
				strErrorMsg = tr("Script force exit");
				emit killKeyEvent(nEvt);
				m_standaloneScript.m_state = SCRIPT_NOFILE;
			}
		} else {
			strErrorMsg = tr("Script run method missing");
			m_standaloneScript.m_state = SCRIPT_SYNTAX_ERROR;
		}
	} LUA_CATCH(
		luaDisable();
	)

	if (m_standaloneScript.m_state != SCRIPT_OK) {
		g_luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;		// ??? Really want to do this?  Or only if m_state==SCRIPT_NOFILE ?
		if (!strErrorMsg.isEmpty()) {
			luaError(g_pLSScripts, m_standaloneScript.m_state, strErrorMsg);
		}
		// TODO : Free/Release script??
	}

	luaDoGc(g_pLSScripts, false);
//	luaDoGc(g_pLSWidgets, false);		// TODO : Figure this out

}


// ============================================================================


