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

#include "main.h"
#include "MainWindow.h"

#include "PersistentSettings.h"

#include "version.h"

QPointer<QApplication> g_pMyApplication = nullptr;

int main(int argc, char *argv[])
{
	QApplication *pApp = new QApplication(argc, argv);
	g_pMyApplication = pApp;

	QString strREV = GIT_REV;
	QString strTAG = GIT_TAG;
	QString strBRANCH = GIT_BRANCH;

	QString strVersion;
	if (!strTAG.isEmpty()) {
		strVersion = strTAG;
	} else {
		strVersion = QString("%1/%2").arg(strBRANCH, strREV);
	}

	pApp->setApplicationVersion(strVersion);
	pApp->setApplicationName("frsky_sport_tool");
	pApp->setOrganizationName("Dewtronics");
	pApp->setOrganizationDomain("dewtronics.com");

	qRegisterMetaType<QSerialPortInfo>("QSerialPortInfo");

	Q_INIT_RESOURCE(frsky_sport_tool);

	CPersistentSettings::instance()->loadSettings();

	CMainWindow *pWindow = new CMainWindow();

	pWindow->show();
	int nRetVal = pApp->exec();

	delete pWindow;		// Must delete the window first so that any preserved settings get restored before quitting

	CPersistentSettings::instance()->saveSettings();

	delete pApp;

	return nRetVal;
}
