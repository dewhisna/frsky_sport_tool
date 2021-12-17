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

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);

	// ------------------------------------------------------------------------

	QAction *pAction;
	QMenu *pFileMenu = ui->menuBar->addMenu(tr("&File"));

	pAction = pFileMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit"));
	pAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	connect(pAction, &QAction::triggered, g_pMyApplication, &QApplication::quit, Qt::QueuedConnection);
	QAction *pQuitAction = pAction;

	// --------------------------------

	// --------------------------------

//	ui->toolBar->addSeparator();
	ui->toolBar->addAction(pQuitAction);
}

CMainWindow::~CMainWindow()
{
	delete ui;
}
