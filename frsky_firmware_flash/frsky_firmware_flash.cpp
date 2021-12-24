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

#include <PersistentSettings.h>

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

#include <frsky_sport_io.h>
#include <frsky_sport_firmware.h>
#include <CLIProgDlg.h>

#include <iostream>

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	app.setApplicationVersion("1.0.0");
	app.setApplicationName("frsky_sport_tool");		// Note: use package name here instead of this app so we can use its common settings
	app.setOrganizationName("Dewtronics");
	app.setOrganizationDomain("dewtronics.com");

	CPersistentSettings::instance()->loadSettings();

	SPORT_ID_ENUM nPort = CPersistentSettings::instance()->getFirmwareSportPort();
	QString strPort = CPersistentSettings::instance()->getDeviceSerialPort(nPort);
	bool bHavePortName =!strPort.isEmpty();

	if ((argc < 2) || (!bHavePortName && (argc < 3))) {
		std::cerr << "Frsky Firmware Flash Programming Tool" << std::endl;
		if (bHavePortName) {
			std::cerr << "Usage: frsky_firmware_flash <firmware-filename> [<port>]" << std::endl;
		} else {
			std::cerr << "Usage: frsky_firmware_flash <firmware-filename> <port>" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Where:" << std::endl;
		std::cerr << "    <firmware-filename> = File name/path to firmware file" << std::endl;
		std::cerr << "    <port> = Serial Port to use" << std::endl;
		if (bHavePortName) {
			std::cerr << "             (Optional, will use current setting of \"" << strPort.toUtf8().data() << "\" if not specified)" << std::endl;
		}
		std::cerr << std::endl << std::endl;

		return -1;
	}

	QString strOverridePort = ((argc >= 3) ? QString::fromUtf8(argv[2]) : QString());

	CFrskySportIO sport(nPort);
	if (!sport.openPort(strOverridePort)) {
		std::cerr << "Failed to open serial port" << std::endl;
		std::cerr << sport.getLastError().toUtf8().data() << std::endl;
		return -2;
	}

	QFile fileFirmware(argv[1]);
	if (!fileFirmware.open(QIODevice::ReadOnly)) {
		std::cerr << "Failed to open firmware file \"" << argv[1] << "\" for reading" << std::endl;
		return -3;
	}

	QFileInfo fiFirmware(fileFirmware);
	bool bIsFrsk = (fiFirmware.suffix().compare("frsk", Qt::CaseInsensitive) == 0);

	CCLIProgDlg dlgProg;
	CFrskyDeviceFirmwareUpdate fsm(sport, &dlgProg);

	if (!fsm.flashDeviceFirmware(fileFirmware, bIsFrsk, true)) {
		std::cerr << fsm.getLastError().toUtf8().data() << std::endl;
	} else {
		std::cerr << "Programming was successful" << std::endl;
	}

	// Don't save persistent settings here, since we aren't changing anything

	return 0;
}

// ============================================================================
