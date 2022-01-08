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

#ifndef LUA_ENGINE_H
#define LUA_ENGINE_H

#include <QObject>
#include <QString>
#include <QPointer>

#include "LuaEvents.h"

// Forware Declarations
struct lua_State;
class QWidget;

// ============================================================================

class CLuaEngine : public QObject
{
	Q_OBJECT

protected:
	enum InterpretterState {
		INTERPRETER_NOT_RUNNING = 0,					// Interpretter not initialized
		INTERPRETER_RUNNING_STANDALONE_SCRIPT = 1,		// Interpretter initialized and m_standaloneScript loaded
		INTERPRETER_RELOAD_PERMANENT_SCRIPTS = 2,		// Interpretter initialized, but m_standaloneScript defunct (such as script load error)
		INTERPRETER_PANIC = 255,						// Interpretter panicked or we caught a Lua Exception and disabled it
	};

public:
	enum ScriptState {
		SCRIPT_OK,				// SCRIPT_OK on success (LUA_OK)
		SCRIPT_NOFILE,			// SCRIPT_NOFILE if file wasn't found for specified mode or Lua could not open file (LUA_ERRFILE)
		SCRIPT_SYNTAX_ERROR,	// SCRIPT_SYNTAX_ERROR if Lua returned a syntax error during pre/de-compilation (LUA_ERRSYNTAX)
		SCRIPT_PANIC,			// SCRIPT_PANIC for Lua memory errors (LUA_ERRMEM or LUA_ERRGCMM)
		SCRIPT_KILLED
	};

//	static constexpr int MAX_SCRIPTS = 9;
//	static constexpr int MAX_SPECIAL_FUNCTIONS = 64;

//	enum ScriptReference {
//		SCRIPT_REF_NIL = -1,
//		SCRIPT_MIX_FIRST = 0,
//		SCRIPT_MIX_LAST=SCRIPT_MIX_FIRST+MAX_SCRIPTS-1,
//		SCRIPT_FUNC_FIRST,
//		SCRIPT_FUNC_LAST=SCRIPT_FUNC_FIRST+MAX_SPECIAL_FUNCTIONS-1,    // model functions
//		SCRIPT_GFUNC_FIRST,
//		SCRIPT_GFUNC_LAST=SCRIPT_GFUNC_FIRST+MAX_SPECIAL_FUNCTIONS-1,  // global functions
//		SCRIPT_TELEMETRY_FIRST,
//		SCRIPT_TELEMETRY_LAST=SCRIPT_TELEMETRY_FIRST+MAX_SCRIPTS, // telem0 and telem1 .. telem7
//	};

	struct TScriptInternalData {
//		ScriptReference m_reference = SCRIPT_REF_NIL;
		ScriptState m_state = SCRIPT_OK;
		int m_run = 0;						// Run function in script
		int m_background = 0;				// Background function in script
		uint8_t m_instructions = 0;
	};

	enum luaScriptInputType {
		INPUT_TYPE_FIRST = 0,
		INPUT_TYPE_VALUE = INPUT_TYPE_FIRST,
		INPUT_TYPE_SOURCE,
		INPUT_TYPE_LAST = INPUT_TYPE_SOURCE
	};

	struct TScriptInput {
		const char *name;
		uint8_t type;
		int16_t min;
		int16_t max;
		int16_t def;
	};

	struct TScriptOutput {
		const char *name;
		int16_t value;
	};

	static constexpr int MAX_SCRIPT_INPUTS = 6;
	static constexpr int MAX_SCRIPT_OUTPUTS = 6;

	struct TScriptInputsOutputs {
		uint8_t m_inputsCount = 0;
		TScriptInput m_inputs[MAX_SCRIPT_INPUTS] = {};
		uint8_t m_outputsCount = 0;
		TScriptOutput m_outputs[MAX_SCRIPT_OUTPUTS] = {};
	};

public:
	explicit CLuaEngine(QWidget *pParent = nullptr);
	virtual ~CLuaEngine();

	static QString currentStandaloneScriptPath()
	{
		return g_strLuaStandaloneScriptPath;
	}

public slots:
	virtual void execLuaScript(const QString &strFilename = QString());		// Loads and inits the Lua Script file
	virtual void runLuaScript(event_t nEvt);			// Does one run of the Lua Script's 'run' function

signals:
	void killKeyEvent(event_t nEvent);					// Signal for parent CLuaEvents::killKeyEvent
	void scriptFinished(int nStatus);					// Signal for when the script has finished execution (nStatus == ScriptState at completion)

protected:
	static int luaPanic(lua_State *pState);

	static void luaDisable();
	static void luaClose(lua_State **ppState);
	static void luaRegisterLibraries(lua_State *pState);

	static ScriptState luaLoadScriptFileToState(lua_State *pState, const char *pFilename);
	static ScriptState luaLoad(lua_State *pState, const char *pFilename,
						TScriptInternalData & sid, TScriptInputsOutputs * sio=nullptr);
	static int luaGetInputs(lua_State * pState, TScriptInputsOutputs & sio);
	static int luaGetOutputs(lua_State * pState, TScriptInputsOutputs & sio);
	static void luaFree(lua_State * pState, TScriptInternalData & sid);
	static void luaDoGc(lua_State * pState, bool bFull);

	static void luaInit();
	bool luaExec(const char *pFilename, TScriptInternalData & sid);
	void luaError(lua_State * pState, ScriptState nError, const QString &strExtraMsg = QString(), bool bAcknowledge = true);

	virtual void init();
	virtual bool exec(const char *pFilename);
	virtual void error(const QString &strTitle, const QString &strMessage, bool bAcknowledge = true);

protected:	// Data:
	static thread_local lua_State *g_pLSScripts;
	static thread_local InterpretterState g_luaState;
	static thread_local QString g_strLuaStandaloneScriptPath;
	TScriptInternalData m_standaloneScript;

	QPointer<QWidget> m_pParentWidget;
};


// ============================================================================

#endif	// LUA_ENGINE_H

