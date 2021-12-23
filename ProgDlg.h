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

#ifndef PROG_DLG_H
#define PROG_DLG_H

#include "UICallback.h"

#include <QString>
#include <QWidget>
#include <QProgressDialog>

// ============================================================================

class CProgDlg : public CUICallback
{
public:
	CProgDlg(const QString &strTitle, QWidget *pParent = nullptr);

	QString title() const { return m_strTitle; }
	QWidget *parent() { return &m_dlgProgress; }

	virtual void setProgressRange(int nMin, int nMax) override;
	virtual void setProgressPos(int nValue) override;
	virtual void setProgressText(const QString &strStatus) override;
	virtual void enableCancel(bool bEnable) override;
	virtual void hookCancel(QObject *pObject, const char *member) override;
	// ----
	virtual BTN_TYPE promptUser(PROMPT_TYPE nPromptType,
						const QString &strText,
						BTN_TYPE nButtons,
						BTN_TYPE nDefaultButton) override;
	// ----
	virtual void setRxDataStatusLED(bool bStatus) override;		// Rx Data Status LED, On/Off or Color change
	virtual void setTxDataStatusLED(bool bStatus) override;		// Tx Data Status LED, On/Off or Color change

private:
	QString m_strTitle;
	QProgressDialog m_dlgProgress;
};



// ============================================================================

#endif	// PROG_DLG_H

