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

#include "CLIProgDlg.h"

#include "myio.h"

#include <iostream>
#include <QStringList>
#include <QCoreApplication>

namespace {
	struct BTN_MAP {
		BTN_TYPE m_nBT;
		char m_cKey;
		QString m_strText;
	};
	const BTN_MAP conarrButtonMap[] = {
		{ CUICallback::Ok, 'K', CCLIProgDlg::tr("Ok") },
		{ CUICallback::Save, 'S', CCLIProgDlg::tr("Save") },
		{ CUICallback::SaveAll, 'A', CCLIProgDlg::tr("SaveAll") },
		{ CUICallback::Open, 'P', CCLIProgDlg::tr("Open") },
		{ CUICallback::Yes, 'Y', CCLIProgDlg::tr("Yes") },
		{ CUICallback::YesToAll, 'T', CCLIProgDlg::tr("YesToAll") },
		{ CUICallback::No, 'N', CCLIProgDlg::tr("No") },
		{ CUICallback::NoToAll, 'O', CCLIProgDlg::tr("NoToAll") },
		{ CUICallback::Abort, 'B', CCLIProgDlg::tr("Abort") },
		{ CUICallback::Retry, 'R', CCLIProgDlg::tr("Retry") },
		{ CUICallback::Ignore, 'I', CCLIProgDlg::tr("Ignore") },
		{ CUICallback::Close, 'L', CCLIProgDlg::tr("Close") },
		{ CUICallback::Cancel, 'C', CCLIProgDlg::tr("Cancel") },
		{ CUICallback::Discard, 'D', CCLIProgDlg::tr("Discard") },
		{ CUICallback::Help, 'H', CCLIProgDlg::tr("Help") },
		{ CUICallback::Apply, 'V', CCLIProgDlg::tr("Apply") },
		{ CUICallback::Reset, 'E', CCLIProgDlg::tr("Reset") },
		{ CUICallback::RestoreDefaults, 'F', CCLIProgDlg::tr("RestoreDefaults") },
		// ----
		{ CUICallback::NoButton, ' ', "" },
	};
};

// ============================================================================

CCLIProgDlg::CCLIProgDlg(QObject *pParent)
	:	QObject(pParent),
		m_bCanCancel(true),
		m_bKeyPressed(false),
		m_nKey(0)
{
}

void CCLIProgDlg::setProgressRange(int nMin, int nMax)
{
	Q_UNUSED(nMin);
	Q_UNUSED(nMax);
}

void CCLIProgDlg::setProgressPos(int nValue)
{
	Q_UNUSED(nValue);
	std::cerr << ".";
	std::cerr.flush();
}

void CCLIProgDlg::setProgressText(const QString &strStatus)
{
	writeMessage(strStatus);
}

void CCLIProgDlg::hookCancel(QObject *pObject, const char *member)
{
	pObject->connect(this, SIGNAL(cancel_triggered()), pObject, member);

}

void CCLIProgDlg::unhookCancel(QObject *pObject, const char *member)
{
	pObject->disconnect(this, SIGNAL(cancel_triggered()), pObject, member);
}

BTN_TYPE CCLIProgDlg::promptUser(PROMPT_TYPE nPromptType,
					const QString &strText,
					BTN_TYPE nButtons,
					BTN_TYPE nDefaultButton)
{
	if (!isInteractive()) return nDefaultButton;

	QString strPromptType;
	switch (nPromptType) {
		case PT_CRITICAL:
			strPromptType = "!!! ";
			break;
		case PT_INFORMATION:
			strPromptType = "--- ";
			break;
		case PT_QUESTION:
			strPromptType = "??? ";
			break;
		case PT_WARNING:
			strPromptType = "*** ";
			break;
	}
	std::cerr << std::endl << strPromptType.toUtf8().data() << strText.toUtf8().data() << std::endl;

	QStringList slButtons;
	const BTN_MAP *pMap = &conarrButtonMap[0];
	while (pMap->m_nBT != CUICallback::NoButton) {
		if (nButtons & pMap->m_nBT) {
			slButtons.append(QString(pMap->m_cKey) + "=" + pMap->m_strText);
		}
		++pMap;
	}

	std::cerr << std::endl << tr("Press:").toUtf8().data();

	bool bDone = slButtons.isEmpty();
	BTN_TYPE nBtn = nDefaultButton;
	while (!bDone) {
		std::cerr << std::endl << slButtons.join(", ").toUtf8().data() << std::endl << ">>> ";
		char ch = toupper(waitForKeyPress(false));
		std::cerr << ch;
		std::cerr.flush();

		if ((ch == ' ') || (ch == '\n') || (ch == '\x0A')) {		// Accept default
			bDone = true;
			continue;
		}

		pMap = &conarrButtonMap[0];
		while (pMap->m_nBT != CUICallback::NoButton) {
			if ((nButtons & pMap->m_nBT) && (ch == pMap->m_cKey)) {
				nBtn = pMap->m_nBT;
				bDone = true;
				break;
			}
			++pMap;
		}
	}
	std::cerr << std::endl;

	return nBtn;
}

void CCLIProgDlg::setRxDataStatusLED(bool bStatus)
{
	Q_UNUSED(bStatus);
}

void CCLIProgDlg::setTxDataStatusLED(bool bStatus)
{
	Q_UNUSED(bStatus);
}

void CCLIProgDlg::cancel()
{
	if (canCancel()) emit cancel_triggered();
}

void CCLIProgDlg::writeMessage(const QString &strMessage)
{
	std::cerr << strMessage.toUtf8().data() << std::endl;
}

void CCLIProgDlg::en_consoleKeyPressed(char nKey)
{
	m_bKeyPressed = true;
	m_nKey = nKey;
}

char CCLIProgDlg::waitForKeyPress(bool bEcho)
{
	CConsoleReader *pConsoleReader = new CConsoleReader(bEcho);
	connect(pConsoleReader, SIGNAL (KeyPressed(char)), this, SLOT(en_consoleKeyPressed(char)));
	pConsoleReader->start();

	m_bKeyPressed = false;
	while (!m_bKeyPressed) {
		QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
		QCoreApplication::sendPostedEvents();
	}

	return m_nKey;
}

// ============================================================================
