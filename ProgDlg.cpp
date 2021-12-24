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

#include "ProgDlg.h"

#include <QPushButton>
#include <QMessageBox>

// ============================================================================

CProgDlg::CProgDlg(const QString &strTitle, QWidget *pParent)
	:	m_strTitle(strTitle),
		m_dlgProgress(pParent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	m_dlgProgress.setWindowTitle(strTitle);
	m_dlgProgress.setWindowModality(Qt::WindowModal);
	m_dlgProgress.setAutoClose(false);
	m_dlgProgress.setAutoReset(false);
	m_dlgProgress.show();
}

void CProgDlg::setProgressRange(int nMin, int nMax)
{
	m_dlgProgress.setRange(nMin, nMax);
}

void CProgDlg::setProgressPos(int nValue)
{
	m_dlgProgress.setValue(nValue);
}

void CProgDlg::setProgressText(const QString &strStatus)
{
	m_dlgProgress.setLabelText(strStatus);
}

void CProgDlg::enableCancel(bool bEnable)
{
	if (bEnable) {
		m_dlgProgress.setCancelButton(new QPushButton(QObject::tr("Cancel", "CProgDlg")));
	} else {
		m_dlgProgress.setCancelButton(nullptr);
	}
}

void CProgDlg::hookCancel(QObject *pObject, const char *member)
{
	pObject->connect(&m_dlgProgress, SIGNAL(canceled()), pObject, member);
}

void CProgDlg::unhookCancel(QObject *pObject, const char *member)
{
	pObject->disconnect(&m_dlgProgress, SIGNAL(canceled()), pObject, member);
}

BTN_TYPE CProgDlg::promptUser(PROMPT_TYPE nPromptType,
					const QString &strText,
					BTN_TYPE nButtons,
					BTN_TYPE nDefaultButton)
{
	switch (nPromptType) {
		case PT_CRITICAL:
			return QMessageBox::critical(&m_dlgProgress, m_strTitle, strText, nButtons, nDefaultButton);

		case PT_INFORMATION:
			return QMessageBox::information(&m_dlgProgress, m_strTitle, strText, nButtons, nDefaultButton);

		case PT_QUESTION:
			return QMessageBox::question(&m_dlgProgress, m_strTitle, strText, nButtons, nDefaultButton);

		case PT_WARNING:
			return QMessageBox::warning(&m_dlgProgress, m_strTitle, strText, nButtons, nDefaultButton);
	}

	return nDefaultButton;
}

void CProgDlg::setRxDataStatusLED(bool bStatus)
{
	Q_UNUSED(bStatus);
}

void CProgDlg::setTxDataStatusLED(bool bStatus)
{
	Q_UNUSED(bStatus);
}

// ============================================================================

