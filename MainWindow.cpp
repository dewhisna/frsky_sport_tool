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
#include "SaveLoadFileDialog.h"

#include <QMessageBox>
#include <QTimer>

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

	for (int nSport = 0; nSport < SPIDE_COUNT; ++nSport) {
		m_arrpSport[nSport] = new Cfrsky_sport_io(static_cast<SPORT_ID_ENUM>(nSport), this);
		connect(m_arrpSport[nSport], SIGNAL(writeLogString(SPORT_ID_ENUM,QString)),
				this, SLOT(writeLogString(SPORT_ID_ENUM,QString)));
	}

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

	m_pWriteLogFileAction = pConnectionMenu->addAction(tr("Write &Log File..."));
	m_pWriteLogFileAction->setCheckable(true);
	connect(m_pWriteLogFileAction, SIGNAL(toggled(bool)), this, SLOT(en_writeLogFile(bool)));	// Toggled instead of triggered so that it works with software as well as user

	// ----------

	pConnectionMenu->addSeparator();

	m_pConfigureAction = pConnectionMenu->addAction(tr("Con&figure..."), this, SLOT(en_configure()));
	m_pConfigureAction->setDisabled(m_pConnectAction->isChecked());

	// ----------

	pConnectionMenu->addSeparator();

	pAction = pConnectionMenu->addAction(QIcon(":/res/exit.png"), tr("E&xit"));
	pAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
	connect(pAction, &QAction::triggered, g_pMyApplication, &QApplication::quit, Qt::QueuedConnection);
	QAction *pQuitAction = pAction;

	// --------------------------------

	QMenu *pFirmwareMenu = ui->menuBar->addMenu(tr("&Firmware"));

	m_pFirmwareIDAction = pFirmwareMenu->addAction(tr("&ID Firmware..."), this, SLOT(en_firmwareID()));
	m_pFirmwareIDAction->setEnabled(m_pConnectAction->isChecked());


	// --------------------------------

	ui->toolBar->addSeparator();
	ui->toolBar->addAction(pQuitAction);

	// --------------------------------

	connect(m_pConnectAction, &QAction::toggled, this, [this](bool bConnected)->void {
		m_pConfigureAction->setDisabled(bConnected);
		m_pFirmwareIDAction->setEnabled(bConnected);
	});

	// --------------------------------

	m_timerLogFile.start();
}

CMainWindow::~CMainWindow()
{
	delete ui;
}

// ----------------------------------------------------------------------------

void CMainWindow::en_connect(bool bConnect)
{
	if (!bConnect) return;

	assert(!m_arrpSport[SPIDE_SPORT1].isNull());

	if (!m_arrpSport[SPIDE_SPORT1]->openPort()) {
		QMessageBox::critical(this, tr("Connection Error"), m_arrpSport[SPIDE_SPORT1]->getLastError());
		// Note: Even though the above 'if' will guard against re-entrancy if
		//	we were to just call setChecked here directly, we must do this on
		//	a singleShot or else Qt won't propagate the toggled() signal to
		//	the connections for the other controls to get reenabled:
		QTimer::singleShot(1, m_pConnectAction, [this](){ m_pConnectAction->setChecked(false); });
		return;
	}

	if (!m_pWriteLogFileAction->isChecked()) {
		m_timerLogFile.restart();
	}
}

void CMainWindow::en_configure()
{
	CConfigDlg dlg;
	dlg.exec();
}

void CMainWindow::en_writeLogFile(bool bOpen)
{
	if (bOpen) {
		QString strFilePathName = CSaveLoadFileDialog::getSaveFileName(
					this,
					tr("Save Log File", "FileFilters"),
					CPersistentSettings::instance()->getLogFileLastPath(),
					tr("Log Files (*.log)", "FileFilters"),
					"log",
					nullptr,
					QFileDialog::Options());
		if (!strFilePathName.isEmpty()) {
			CPersistentSettings::instance()->setLogFileLastPath(strFilePathName);

			m_fileLogFile.setFileName(strFilePathName);
			if (!m_fileLogFile.open(QIODevice::WriteOnly)) {
				QMessageBox::warning(this, tr("Opening Log File"), tr("Error: Couldn't open Log File \"%1\" for writing.").arg(strFilePathName));
				// Note: Even though the above 'if' will guard against re-entrancy if
				//	we were to just call setChecked here directly, we must do this on
				//	a singleShot or else Qt won't propagate the toggled() signal to
				//	the connections for the other controls to get reenabled:
				QTimer::singleShot(1, m_pWriteLogFileAction, [this](){ m_pWriteLogFileAction->setChecked(false); });
				return;
			}
			m_pLogFile.reset(new QTextStream(static_cast<QIODevice *>(&m_fileLogFile)));

			m_timerLogFile.restart();
		} else {
			// Note: Even though the above 'if' will guard against re-entrancy if
			//	we were to just call setChecked here directly, we must do this on
			//	a singleShot or else Qt won't propagate the toggled() signal to
			//	the connections for the other controls to get reenabled:
			QTimer::singleShot(1, m_pWriteLogFileAction, [this](){ m_pWriteLogFileAction->setChecked(false); });
			return;
		}
	} else {
		// Close log file:
		m_pLogFile.reset();
		m_fileLogFile.close();
	}
}

void CMainWindow::writeLogString(SPORT_ID_ENUM nSport, const QString &strLogString)
{
	if (!m_pLogFile.isNull() && m_pLogFile->device()->isOpen() && m_pLogFile->device()->isWritable()) {
		QStringList lstLogData;
		double nElapsedTime = static_cast<double>(m_timerLogFile.nsecsElapsed())/1000000.0;
		(*m_pLogFile) << QString("%1").arg(nElapsedTime, 0, 'f', 4) << ": " << QString::number(nSport) << ": " << strLogString << Qt::endl;
	}
}

void CMainWindow::en_firmwareID()
{
}

// ============================================================================
