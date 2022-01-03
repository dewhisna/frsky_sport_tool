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

#ifndef LUASCRIPTDLG_H
#define LUASCRIPTDLG_H

#include <QPointer>
#include <QDialog>

// ============================================================================

// Forware Declarations
class QKeyEvent;
class CLuaEvents;
class CLuaEngine;
class CLuaGeneral;

// ----------------------------------------------------------------------------

namespace Ui {
	class CLuaScriptDlg;
}

class CLuaScriptDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CLuaScriptDlg(const QString &strFilename = QString(), QWidget *parent = nullptr);
	virtual ~CLuaScriptDlg();

protected:
	virtual void keyPressEvent(QKeyEvent *pEvent) override;
	virtual void keyReleaseEvent(QKeyEvent *pEvent) override;

private:
	QPointer<CLuaEvents> m_pLuaEvents;
	QPointer<CLuaEngine> m_pLuaEngine;
	QPointer<CLuaGeneral> m_pLuaGeneral;
	Ui::CLuaScriptDlg *ui;
};

// ============================================================================

#endif // LUASCRIPTDLG_H
