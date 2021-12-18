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
#include "ui_MainWindow.h"

#include "PersistentSettings.h"
#include "ConfigDlg.h"

#include <assert.h>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

// ============================================================================

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);

	// ------------------------------------------------------------------------

	QAction *pAction;
	QMenu *pConnectionMenu = ui->menuBar->addMenu(tr("&Connection"));

	QIcon iconConnect;
	iconConnect.addPixmap(QPixmap(":/res/iconfinder_dedicated-server_4263512_connected_512.png"), QIcon::Normal, QIcon::On);
	iconConnect.addPixmap(QPixmap(":/res/iconfinder_dedicated-server_4263512_512.png"), QIcon::Normal, QIcon::Off);
//	iconConnect.addPixmap(iconConnect.pixmap(QSize(512, 512), QIcon::Disabled, QIcon::Off), QIcon::Normal, QIcon::Off);
	m_pConnectAction = pConnectionMenu->addAction(iconConnect, tr("&Connect"));
	m_pConnectAction->setCheckable(true);
	connect(m_pConnectAction, SIGNAL(toggled(bool)), this, SLOT(en_connect(bool)));		// Toggled instead of triggered so that it works with software as well as user

	ui->toolBar->addAction(m_pConnectAction);

	// ----------

	m_pChkWriteLogFile = pConnectionMenu->addAction(tr("Write &Log File..."));
	m_pChkWriteLogFile->setCheckable(true);
	connect(m_pChkWriteLogFile, SIGNAL(toggled(bool)), this, SLOT(en_connect(bool)));	// Toggled instead of triggered so that it works with software as well as user

	// ----------

	pConnectionMenu->addSeparator();

	pAction = pConnectionMenu->addAction(tr("Con&figure..."), this, SLOT(en_configure()));
	connect(m_pConnectAction, SIGNAL(toggled(bool)), pAction, SLOT(setDisabled(bool)));

	// ----------

	pConnectionMenu->addSeparator();

	pAction = pConnectionMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit"));
	pAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	connect(pAction, &QAction::triggered, g_pMyApplication, &QApplication::quit, Qt::QueuedConnection);
	QAction *pQuitAction = pAction;

	// --------------------------------


	// --------------------------------

	ui->toolBar->addSeparator();
	ui->toolBar->addAction(pQuitAction);
}

CMainWindow::~CMainWindow()
{
	delete ui;
}

// ----------------------------------------------------------------------------

void CMainWindow::en_connect(bool bConnect)
{
	if (!bConnect) return;
}

void CMainWindow::en_configure()
{
	CConfigDlg dlg;
	dlg.exec();
}

void CMainWindow::en_writeLogFile(bool bOpen)
{
}

// ============================================================================
