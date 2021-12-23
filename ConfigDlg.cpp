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

#include "ConfigDlg.h"
#include "ui_ConfigDlg.h"

#include "main.h"
#include "PersistentSettings.h"

// ============================================================================

CConfigDlg::CConfigDlg(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CConfigDlg)
{
	ui->setupUi(this);

	ui->comboSport1SerialPort->clear();
	ui->comboSport1SerialPort->addItem(QString(), QVariant::fromValue(QSerialPortInfo()));
	foreach(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
#ifdef Q_OS_WIN
		ui->comboSport1SerialPort->addItem(port.portName(), QVariant::fromValue(port));
#else
		ui->comboSport1SerialPort->addItem(port.systemLocation(), QVariant::fromValue(port));
#endif
	}
	connect(ui->comboSport1SerialPort, SIGNAL(currentIndexChanged(int)), this, SLOT(en_selectSport1SerialPort(int)));
	ui->comboSport1SerialPort->setCurrentText(CPersistentSettings::instance()->getDeviceSerialPort(SPIDE_SPORT1));	// This will trigger change event

	ui->comboSport1BaudRate->clear();
	ui->comboSport1BaudRate->addItem("1200", QVariant(1200));
	ui->comboSport1BaudRate->addItem("2400", QVariant(2400));
	ui->comboSport1BaudRate->addItem("4800", QVariant(4800));
	ui->comboSport1BaudRate->addItem("9600", QVariant(9600));
	ui->comboSport1BaudRate->addItem("14400", QVariant(14400));
	ui->comboSport1BaudRate->addItem("19200", QVariant(19200));
	ui->comboSport1BaudRate->addItem("38400", QVariant(38400));
	ui->comboSport1BaudRate->addItem("57600", QVariant(57600));
	ui->comboSport1BaudRate->addItem("115200", QVariant(115200));
	ui->comboSport1BaudRate->setCurrentText(QString::number(CPersistentSettings::instance()->getDeviceBaudRate(SPIDE_SPORT1)));

	ui->comboSport1Parity->clear();
	ui->comboSport1Parity->addItem("N", QVariant(QChar('N')));
	ui->comboSport1Parity->addItem("E", QVariant(QChar('E')));
	ui->comboSport1Parity->addItem("O", QVariant(QChar('O')));
	ui->comboSport1Parity->setCurrentText(QString(QChar(CPersistentSettings::instance()->getDeviceParity(SPIDE_SPORT1))));

	ui->comboSport1DataBits->clear();
	ui->comboSport1DataBits->addItem("7", QVariant(7));
	ui->comboSport1DataBits->addItem("8", QVariant(8));
	ui->comboSport1DataBits->setCurrentText(QString::number(CPersistentSettings::instance()->getDeviceDataBits(SPIDE_SPORT1)));

	ui->comboSport1StopBits->clear();
	ui->comboSport1StopBits->addItem("1", QVariant(1));
	ui->comboSport1StopBits->addItem("2", QVariant(2));
	ui->comboSport1StopBits->setCurrentText(QString::number(CPersistentSettings::instance()->getDeviceStopBits(SPIDE_SPORT1)));

	// --------------------------------

	connect(this, &QDialog::accepted, [&]()->void {
#ifdef Q_OS_WIN
		CPersistentSettings::instance()->setDeviceSerialPort(SPIDE_SPORT1, m_selectedSport1SerialPort.portName());
#else
		CPersistentSettings::instance()->setDeviceSerialPort(SPIDE_SPORT1, m_selectedSport1SerialPort.systemLocation());
#endif
		CPersistentSettings::instance()->setDeviceBaudRate(SPIDE_SPORT1, ui->comboSport1BaudRate->currentData().toInt());
		CPersistentSettings::instance()->setDeviceParity(SPIDE_SPORT1, ui->comboSport1Parity->currentData().toChar().toLatin1());
		CPersistentSettings::instance()->setDeviceDataBits(SPIDE_SPORT1, ui->comboSport1DataBits->currentData().toInt());
		CPersistentSettings::instance()->setDeviceStopBits(SPIDE_SPORT1, ui->comboSport1StopBits->currentData().toInt());
	});

}

CConfigDlg::~CConfigDlg()
{
	delete ui;
	ui = nullptr;
}

// ----------------------------------------------------------------------------

void CConfigDlg::en_selectSport1SerialPort(int nIndex)
{
	if (nIndex == -1) {
		m_selectedSport1SerialPort = QSerialPortInfo();
	} else {
		m_selectedSport1SerialPort = ui->comboSport1SerialPort->currentData().value<QSerialPortInfo>();
	}
}

// ============================================================================
