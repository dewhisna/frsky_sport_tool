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
#include <LogFile.h>
#include <frsky_sport_io.h>
#include <frsky_sport_emu.h>
#include <CLIProgDlg.h>

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>

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
	QString strLogFile;
	SPORT_ID_ENUM nSport = CPersistentSettings::instance()->getFirmwareSportPort();
	QString strPort;
	int nBaudRate = CPersistentSettings::instance()->getDeviceBaudRate(nSport);
	bool bInteractive = false;
	bool bNeedUsage = false;
	int nArgsFound = 0;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = argv[ndx];
		if (!strArg.startsWith("-")) {
			switch (nArgsFound) {
				case 0:
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
		} else if (strArg.startsWith("-f")) {
			if ((strArg == "-f") && (argc > ndx+1)) {
				strFirmware = argv[ndx+1];
				++ndx;
			} else {
				strFirmware = strArg.mid(2);
			}
		} else if (strArg.startsWith("-l")) {
			if ((strArg == "-l") && (argc > ndx+1)) {
				strLogFile = argv[ndx+1];
				++ndx;
			} else {
				strLogFile = strArg.mid(2);
			}
		} else if (strArg == "-i") {
			bInteractive = true;
		} else {
			bNeedUsage = true;
		}
	}
	if (strPort.isEmpty()) bNeedUsage = true;

	// TODO : Add options for devices other than receivers

	if (bNeedUsage) {
		std::cerr << "Frsky Device Emulation Tool (for testing)" << std::endl;
		std::cerr << "Usage: frsky_device_emu [-b <baudrate>] [-l <logfile>] [-f <firmware>] [-i] <port>" << std::endl;
		std::cerr << std::endl;
		std::cerr << "Where:" << std::endl;
		std::cerr << "    -b <baudrate> = optional baud-rate specifier" << std::endl;
		std::cerr << "                    (if omitted, will use the current setting of " << CPersistentSettings::instance()->getDeviceBaudRate(nSport) << ")" << std::endl;
		std::cerr << "    -l <logfile>  = optional communications log file to generate" << std::endl;
		std::cerr << "    -f <firmware> = optional firmware filename to use for comparison" << std::endl;
		std::cerr << "                    (if omitted, will skip byte-wise checks for firmware content)" << std::endl;
		std::cerr << "    -i = interactive mode, enables prompts" << std::endl;
		std::cerr << std::endl;
		std::cerr << "    <firmware-filename> = File name/path to firmware file" << std::endl;
		std::cerr << "    <port> = Serial Port to use" << std::endl;
		std::cerr << std::endl << std::endl;

		return -1;
	}

	if (bInteractive) std::cerr << "Interactive Mode" << std::endl;
	std::cerr << "Serial Port: " << strPort.toUtf8().data() << std::endl;
	std::cerr << "Baud Rate: " << nBaudRate << std::endl;
	std::cerr << "Log File: " << strLogFile.toUtf8().data() << std::endl;

	CFrskySportIO sport(nSport);
	if (!sport.openPort(strPort, nBaudRate)) {
		std::cerr << "Failed to open serial port" << std::endl;
		std::cerr << sport.getLastError().toUtf8().data() << std::endl;
		return -2;
	}

	QFile fileFirmware(strFirmware);
	if (!strFirmware.isEmpty() && !fileFirmware.open(QIODevice::ReadOnly)) {
		std::cerr << "Failed to open firmware file \"" << strFirmware.toUtf8().data() << "\" for reading" << std::endl;
		return -3;
	}

	QFileInfo fiFirmware(fileFirmware);
	bool bIsFrsk = (fiFirmware.suffix().compare("frsk", Qt::CaseInsensitive) == 0);

	CLogFile logFile;
	if (!strLogFile.isEmpty()) {
		if (!logFile.openLogFile(strLogFile, QIODevice::WriteOnly)) {
			std::cerr << "Failed to open \"" << strLogFile.toUtf8().data() << "\" for writing" << std::endl;
			std::cerr << logFile.getLastError().toUtf8().data() << std::endl;
			return -4;
		}
		QObject::connect(&sport, &CFrskySportIO::writeLogString, &sport,
							[&logFile](SPORT_ID_ENUM nSport, const QString &strMessage)->void {
								Q_UNUSED(nSport);
								logFile.writeLogString(strMessage);
							});
	}

	CCLIProgDlg dlgProg;
	dlgProg.setInteractive(bInteractive);

	CFrskySportDeviceEmu emu(sport, &dlgProg);
	QObject::connect(&emu, SIGNAL(emulationErrorEncountered(QString)), &dlgProg, SLOT(writeMessage(QString)));

	if (fileFirmware.isOpen()) {
		if (!emu.setFirmware(fileFirmware, bIsFrsk)) {
			// No need to print the error, as that will happen in the
			//	write message callback above
			return -5;
		}
	}

	if (!emu.startDeviceEmulation(CFrskySportDeviceEmu::FRSKDEV_RX, true)) {
		// No need to print the error, as that will happen in the
		//	write message callback above
		return -6;
	} else {
		std::cerr << "Emulation was successful" << std::endl;
	}

	// Don't save persistent settings here, since we aren't changing anything

	return 0;
}

// ============================================================================
