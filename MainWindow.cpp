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

#include "frsky_sport_io.h"
#include "frsky_sport_firmware.h"
#include "ProgDlg.h"

#include "AboutDlg.h"

#ifdef LUA_SUPPORT
#include "LuaScriptDlg.h"
#include "LuaLCD.h"
#endif

#include <QMessageBox>
#include <QTimer>
#include <QFileInfo>

#include <assert.h>

// ============================================================================

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);

	// ------------------------------------------------------------------------

	for (int nSport = 0; nSport < SPIDE_COUNT; ++nSport) {
		m_arrpSport[nSport] = new CFrskySportIO(static_cast<SPORT_ID_ENUM>(nSport), this);
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

	m_pFirmwareProgramAction = pFirmwareMenu->addAction(tr("&Program Firmware to Device..."), this, SLOT(en_firmwareProgram()));
	m_pFirmwareProgramAction->setEnabled(m_pConnectAction->isChecked());

	m_pFirmwareReadAction = pFirmwareMenu->addAction(tr("&Read Firmware from Device..."), this, SLOT(en_firmwareRead()));
	m_pFirmwareReadAction->setEnabled(m_pConnectAction->isChecked());

	// --------------------------------

#ifdef LUA_SUPPORT
	QMenu *pLuaScriptMenu = ui->menuBar->addMenu(tr("&Lua Script"));

	m_pRunLuaScriptAction = pLuaScriptMenu->addAction(tr("&Run Lua Script..."), this, SLOT(en_runLuaScript()));
	m_pRunLuaScriptAction->setEnabled(m_pConnectAction->isChecked());

	QMenu *pLuaScreenThemeMenu = pLuaScriptMenu->addMenu(tr("Screen &Theme"));
	QActionGroup *pLSTMActionGroup = new QActionGroup(pLuaScreenThemeMenu);
	pLSTMActionGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
	pLuaScreenThemeMenu->menuAction()->setActionGroup(pLSTMActionGroup);
	for (int i = 0; i < CLuaLCD::LCD_THEME_COUNT; ++i) {
		pAction = pLuaScreenThemeMenu->addAction(CLuaLCD::themeName(static_cast<CLuaLCD::LCD_THEME_ENUM>(i)));
		pAction->setCheckable(true);
		pAction->setData(i);
		pLSTMActionGroup->addAction(pAction);
		if (CPersistentSettings::instance()->getLuaScreenTheme() == i) {
			pAction->setChecked(true);
		}
		connect(pLSTMActionGroup, &QActionGroup::triggered, this, [](QAction *pAction)->void {
			assert(pAction !=nullptr);
			CPersistentSettings::instance()->setLuaScreenTheme(pAction->data().toInt());
		});
	}

	// Support for /scripts/ folder in AppImage:
	QString strAppDir = qgetenv("APPDIR");
	if (!strAppDir.isEmpty()) {
		QString strScriptDir = strAppDir + "/scripts/";
		QFileInfo fiScriptDir(strScriptDir);
		if (fiScriptDir.exists() && fiScriptDir.isDir()) {
			CPersistentSettings::instance()->setLuaScriptLastPath(strScriptDir);
		}
	}
#endif

	// --------------------------------

	QMenu *pHelpMenu = ui->menuBar->addMenu(tr("&Help"));

	pAction = pHelpMenu->addAction(tr("&About..."));
	connect(pAction, &QAction::triggered, this, [this]()->void {
		CAboutDlg dlg(this);
		dlg.exec();
	});

	// --------------------------------

	ui->toolBar->addSeparator();
	ui->toolBar->addAction(pQuitAction);

	// --------------------------------

	connect(m_pConnectAction, &QAction::toggled, this, [this](bool bConnected)->void {
		m_pConfigureAction->setDisabled(bConnected);
		m_pFirmwareIDAction->setEnabled(bConnected);
		m_pFirmwareProgramAction->setEnabled(bConnected);
		m_pFirmwareReadAction->setEnabled(bConnected);
#ifdef LUA_SUPPORT
		m_pRunLuaScriptAction->setEnabled(bConnected);
#endif
	});

	// --------------------------------

	m_logFile.resetTimer();
}

CMainWindow::~CMainWindow()
{
	delete ui;
	ui = nullptr;
}

// ----------------------------------------------------------------------------

void CMainWindow::en_connect(bool bConnect)
{
	for (int i = 0; i < SPIDE_COUNT; ++i) {
		assert(!m_arrpSport[i].isNull());
	}

	if (!bConnect) {
		for (int i = 0; i < SPIDE_COUNT; ++i) {
			m_arrpSport[i]->closePort();
		}
		return;
	}

	for (int i = 0; i < SPIDE_COUNT; ++i) {
		if (!CPersistentSettings::instance()->getDeviceSerialPort(static_cast<SPORT_ID_ENUM>(i)).isEmpty() &&
			!m_arrpSport[i]->openPort()) {
			QMessageBox::critical(this, tr("Connection Error"), tr("Opening Sport #%1").arg(i+1) + "\n" + m_arrpSport[i]->getLastError());
			// Note: Even though the above 'if' will guard against re-entrancy if
			//	we were to just call setChecked here directly, we must do this on
			//	a singleShot or else Qt won't propagate the toggled() signal to
			//	the connections for the other controls to get reenabled:
			QTimer::singleShot(1, m_pConnectAction, [this](){ m_pConnectAction->setChecked(false); });
			return;
		}
	}

	if (!m_pWriteLogFileAction->isChecked()) {
		m_logFile.resetTimer();
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

			if (!m_logFile.openLogFile(strFilePathName, QIODevice::WriteOnly)) {
				QMessageBox::warning(this, tr("Opening Log File"), tr("Error: Couldn't open Log File \"%1\" for writing.").arg(strFilePathName));
				// Note: Even though the above 'if' will guard against re-entrancy if
				//	we were to just call setChecked here directly, we must do this on
				//	a singleShot or else Qt won't propagate the toggled() signal to
				//	the connections for the other controls to get reenabled:
				QTimer::singleShot(1, m_pWriteLogFileAction, [this](){ m_pWriteLogFileAction->setChecked(false); });
				return;
			}

			m_logFile.resetTimer();
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
		m_logFile.closeLogFile();
	}
}

void CMainWindow::writeLogString(SPORT_ID_ENUM nSport, const QString &strLogString)
{
	if (m_logFile.isWritable()) {
		m_logFile.writeLogString(QString::number(nSport+1) + ": " + strLogString);
	}
}

// ----------------------------------------------------------------------------

void CMainWindow::en_firmwareID()
{
	assert(!m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()].isNull());

	if (!m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()]->isOpen()) {
		QMessageBox::critical(this, windowTitle(), tr("Sport #%1 is not open.  Check configuration!").arg(CPersistentSettings::instance()->getFirmwareSportPort()+1));
		return;
	}

	CProgDlg dlgProg(tr("ID Device Firmware"), this);
	CFrskyDeviceFirmwareUpdate fsm(*m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()], &dlgProg, this);
	if (!fsm.idDevice(true)) {
		QMessageBox::critical(dlgProg.parent(), dlgProg.title(), fsm.getLastError());
	}
}

void CMainWindow::en_firmwareProgram()
{
	assert(!m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()].isNull());

	if (!m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()]->isOpen()) {
		QMessageBox::critical(this, windowTitle(), tr("Sport #%1 is not open.  Check configuration!").arg(CPersistentSettings::instance()->getFirmwareSportPort()+1));
		return;
	}

	QString strFilePathName = CSaveLoadFileDialog::getOpenFileName(
				this,
				tr("Load Firmware File", "FileFilters"),
				CPersistentSettings::instance()->getFirmwareLastReadPath(),
				tr("Frsky Firmware Files (*.frk *.frsk)", "FileFilters"),
				nullptr,
				QFileDialog::Options());
	if (strFilePathName.isEmpty()) return;

	CPersistentSettings::instance()->setFirmwareLastReadPath(strFilePathName);

	QFile fileFirmware(strFilePathName);
	if (!fileFirmware.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(this, tr("Opening Firmware File"), tr("Error: Couldn't open Firmware File \"%1\" for reading.").arg(strFilePathName));
		return;
	}

	QFileInfo fiFirmware(fileFirmware);
	bool bIsFrsk = (fiFirmware.suffix().compare("frsk", Qt::CaseInsensitive) == 0);

	CProgDlg dlgProg(tr("Program Firmware"), this);
	CFrskyDeviceFirmwareUpdate fsm(*m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()], &dlgProg, this);
	if (!fsm.flashDeviceFirmware(fileFirmware, bIsFrsk, true)) {
		QMessageBox::critical(dlgProg.parent(), dlgProg.title(), fsm.getLastError());
	} else {
		QMessageBox::information(dlgProg.parent(), dlgProg.title(), tr("Firmware Programming Successful!"));
	}

	fileFirmware.close();
}

void CMainWindow::en_firmwareRead()
{
	assert(!m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()].isNull());

	QMessageBox::warning(this, "Read Firmware", "WARNING: This function is experimental and exploits undocumented Frsky Protocol details and may destroy the Frsky Device!  Proceed with caution...");

	if (!m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()]->isOpen()) {
		QMessageBox::critical(this, windowTitle(), tr("Sport #%1 is not open.  Check configuration!").arg(CPersistentSettings::instance()->getFirmwareSportPort()+1));
		return;
	}

	QString strFilePathName = CSaveLoadFileDialog::getSaveFileName(
				this,
				tr("Save Firmware File", "FileFilters"),
				CPersistentSettings::instance()->getFirmwareLastWritePath(),
				tr("Frsky Firmware Files (*.frk)", "FileFilters"),
				"frk",
				nullptr,
				QFileDialog::Options());
	if (strFilePathName.isEmpty()) return;
	CPersistentSettings::instance()->setFirmwareLastWritePath(strFilePathName);

	QFile fileFirmware(strFilePathName);
	if (!fileFirmware.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, tr("Opening Firmware File"), tr("Error: Couldn't open Firmware File \"%1\" for writing.").arg(strFilePathName));
		return;
	}

	CProgDlg dlgProg(tr("Read Firmware"), this);
	CFrskyDeviceFirmwareUpdate fsm(*m_arrpSport[CPersistentSettings::instance()->getFirmwareSportPort()], &dlgProg, this);
	if (!fsm.readDeviceFirmware(fileFirmware, true)) {
		QMessageBox::critical(dlgProg.parent(), dlgProg.title(), fsm.getLastError());
	} else {
		QMessageBox::information(dlgProg.parent(), dlgProg.title(), tr("Firmware Reading Successful!"));
	}

	fileFirmware.close();
}

// ----------------------------------------------------------------------------

#ifdef LUA_SUPPORT
void CMainWindow::en_runLuaScript()
{
	assert(!m_arrpSport[CPersistentSettings::instance()->getDataConfigSportPort()].isNull());

	if (!m_arrpSport[CPersistentSettings::instance()->getDataConfigSportPort()]->isOpen()) {
		QMessageBox::critical(this, windowTitle(), tr("Sport #%1 is not open.  Check configuration!").arg(CPersistentSettings::instance()->getDataConfigSportPort()+1));
		return;
	}

	QString strFilePathName = CSaveLoadFileDialog::getOpenFileName(
				this,
				tr("Load Lua Script", "FileFilters"),
				CPersistentSettings::instance()->getLuaScriptLastPath(),
				tr("Lua Scripts (*.lua *.luac);;All Files (*.*)", "FileFilters"),
				nullptr,
				QFileDialog::Options());
	if (strFilePathName.isEmpty()) return;
	CPersistentSettings::instance()->setLuaScriptLastPath(strFilePathName);

	CLuaScriptDlg dlg(*m_arrpSport[CPersistentSettings::instance()->getDataConfigSportPort()], strFilePathName, this);
	dlg.exec();
}
#endif

// ============================================================================
