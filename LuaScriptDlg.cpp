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

#include "LuaScriptDlg.h"
#include "ui_LuaScriptDlg.h"

#include "LuaEvents.h"
#include "LuaEngine.h"
#include "LuaGeneral.h"

#include <QTimer>
#include <QKeyEvent>

#include <assert.h>

// ============================================================================

CLuaScriptDlg::CLuaScriptDlg(const QString &strFilename, QWidget *parent) :
	QDialog(parent),
	m_pLuaEvents(new CLuaEvents(this)),
	m_pLuaEngine(new CLuaEngine(this)),
	m_pLuaGeneral(new CLuaGeneral(this)),
	ui(new Ui::CLuaScriptDlg)
{
	ui->setupUi(this);

	connect(m_pLuaEngine, SIGNAL(killKeyEvent(event_t)), m_pLuaEvents, SLOT(killKeyEvent(event_t)));
	connect(m_pLuaGeneral, SIGNAL(killKeyEvent(event_t)), m_pLuaEvents, SLOT(killKeyEvent(event_t)));
	connect(m_pLuaEvents, SIGNAL(luaEvent(event_t)), m_pLuaEngine, SLOT(runLuaScript(event_t)));

	if (!strFilename.isEmpty()) QTimer::singleShot(1, m_pLuaEngine, [=]() { m_pLuaEngine->execLuaScript(strFilename); });
}

CLuaScriptDlg::~CLuaScriptDlg()
{
	delete ui;
	ui = nullptr;
}

// ----------------------------------------------------------------------------

void CLuaScriptDlg::keyPressEvent(QKeyEvent *pEvent)
{
	assert(!m_pLuaEvents.isNull());
	if (!m_pLuaEvents->keyPressEvent(pEvent)) QDialog::keyPressEvent(pEvent);
}

void CLuaScriptDlg::keyReleaseEvent(QKeyEvent *pEvent)
{
	assert(!m_pLuaEvents.isNull());
	if (!m_pLuaEvents->keyReleaseEvent(pEvent)) QDialog::keyReleaseEvent(pEvent);
}

// ----------------------------------------------------------------------------


// ============================================================================
