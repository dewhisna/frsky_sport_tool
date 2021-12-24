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

	QString strFirmware;
	SPORT_ID_ENUM nSport = CPersistentSettings::instance()->getFirmwareSportPort();
	QString strPort = CPersistentSettings::instance()->getDeviceSerialPort(nSport);
	bool bHavePortNameSetting =!strPort.isEmpty();
	int nBaudRate = CPersistentSettings::instance()->getDeviceBaudRate(nSport);
	bool bInteractive = false;
	bool bNeedUsage = false;
	int nArgsFound = 0;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = argv[ndx];
		if (!strArg.startsWith("-")) {
			switch (nArgsFound) {
				case 0:
					strFirmware = strArg;
					break;
				case 1:
					strPort = strArg;
					break;
				default:
					bNeedUsage = true;
					break;
			}
			++nArgsFound;
		} else if (strArg.startsWith("-b")) {
			if ((strArg == "-b") && (argc > ndx+1)) {
				nBaudRate = strtoul(argv[ndx+1], nullptr, 0);
				++ndx;
			} else {
				nBaudRate = strtoul(strArg.mid(2).toUtf8().data(), nullptr, 0);
			}
		} else if (strArg == "-i") {
			bInteractive = true;
		} else {
			bNeedUsage = true;
		}
	}
	if (strFirmware.isEmpty() || strPort.isEmpty()) bNeedUsage = true;

	if (bNeedUsage) {
		std::cerr << "Frsky Firmware Flash Programming Tool" << std::endl;
		if (bHavePortNameSetting) {
			std::cerr << "Usage: frsky_firmware_flash [-b <baudrate>] [-i] <firmware-filename> [<port>]" << std::endl;
		} else {
			std::cerr << "Usage: frsky_firmware_flash [-b <baudrate>] [-i] <firmware-filename> <port>" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Where:" << std::endl;
		std::cerr << "    -b <baudrate> = optional baud-rate specifier" << std::endl;
		std::cerr << "                    (if omitted, will use the current setting of " << CPersistentSettings::instance()->getDeviceBaudRate(nSport) << ")" << std::endl;
		std::cerr << "     -i = interactive mode, enables prompts" << std::endl;
		std::cerr << "    <firmware-filename> = File name/path to firmware file" << std::endl;
		std::cerr << "    <port> = Serial Port to use" << std::endl;
		if (bHavePortNameSetting) {
			std::cerr << "             (Optional, will use current setting of \"" << CPersistentSettings::instance()->getDeviceSerialPort(nSport).toUtf8().data() << "\" if not specified)" << std::endl;
		}
		std::cerr << std::endl << std::endl;

		return -1;
	}

	std::cerr << "Serial Port: " << strPort.toUtf8().data() << std::endl;
	std::cerr << "Baud Rate: " << nBaudRate << std::endl;

	CFrskySportIO sport(nSport);
	if (!sport.openPort(strPort, nBaudRate)) {
		std::cerr << "Failed to open serial port" << std::endl;
		std::cerr << sport.getLastError().toUtf8().data() << std::endl;
		return -2;
	}

	QFile fileFirmware(strFirmware);
	if (!fileFirmware.open(QIODevice::ReadOnly)) {
		std::cerr << "Failed to open firmware file \"" << strFirmware.toUtf8().data() << "\" for reading" << std::endl;
		return -3;
	}

	QFileInfo fiFirmware(fileFirmware);
	bool bIsFrsk = (fiFirmware.suffix().compare("frsk", Qt::CaseInsensitive) == 0);

	CCLIProgDlg dlgProg;
	CFrskyDeviceFirmwareUpdate fsm(sport, &dlgProg);

	dlgProg.setInteractive(bInteractive);

	if (!fsm.flashDeviceFirmware(fileFirmware, bIsFrsk, true)) {
		std::cerr << fsm.getLastError().toUtf8().data() << std::endl;
	} else {
		std::cerr << "Programming was successful" << std::endl;
	}

	// Don't save persistent settings here, since we aren't changing anything

	return 0;
}

// ============================================================================
