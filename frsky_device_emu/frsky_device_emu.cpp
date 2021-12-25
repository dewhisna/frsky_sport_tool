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

#include <version.h>

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	QString strREV = GIT_REV;
	QString strTAG = GIT_TAG;
	QString strBRANCH = GIT_BRANCH;

	QString strVersion;
	if (!strTAG.isEmpty()) {
		strVersion = strTAG;
	} else {
		strVersion = QString("%1/%2").arg(strBRANCH, strREV);
	}

	app.setApplicationVersion(strVersion);
	app.setApplicationName("frsky_sport_tool");		// Note: use package name here instead of this app so we can use its common settings
	app.setOrganizationName("Dewtronics");
	app.setOrganizationDomain("dewtronics.com");

	CPersistentSettings::instance()->loadSettings();

	QString strFirmwareIn;
	QString strFirmwareOut;
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
				strFirmwareIn = argv[ndx+1];
				++ndx;
			} else {
				strFirmwareIn = strArg.mid(2);
			}
		} else if (strArg.startsWith("-w")) {
			if ((strArg == "-w") && (argc > ndx+1)) {
				strFirmwareOut = argv[ndx+1];
				++ndx;
			} else {
				strFirmwareOut = strArg.mid(2);
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
		std::cerr << "Version: " << strVersion.toUtf8().data() << std::endl;
		std::cerr << "Usage: frsky_device_emu [-b <baudrate>] [-l <logfile>] [-f <firmware-in>] [-f <firmware-out>] [-i] <port>" << std::endl;
		std::cerr << std::endl;
		std::cerr << "Where:" << std::endl;
		std::cerr << "    -b <baudrate> = optional baud-rate specifier" << std::endl;
		std::cerr << "                    (if omitted, will use the current setting of " << CPersistentSettings::instance()->getDeviceBaudRate(nSport) << ")" << std::endl;
		std::cerr << "    -l <logfile>  = optional communications log file to generate" << std::endl;
		std::cerr << "    -f <firmware-in> = optional input firmware filename to use for comparison" << std::endl;
		std::cerr << "                    (if omitted, will skip byte-wise checks for firmware content)" << std::endl;
		std::cerr << "    -w <firware-out> = optional output firmware filename to write received data" << std::endl;
		std::cerr << "                    (written firmware will always be in headerless .frk format)" << std::endl;
		std::cerr << "    -i = interactive mode, enables prompts" << std::endl;
		std::cerr << std::endl;
		std::cerr << "    <firmware-filename> = File name/path to firmware file" << std::endl;
		std::cerr << "    <port> = Serial Port to use" << std::endl;
		std::cerr << std::endl << std::endl;

		return -1;
	}

	std::cerr << "frsky_device_emu version: " << strVersion.toUtf8().data() << std::endl;

	if (bInteractive) std::cerr << "Interactive Mode" << std::endl;
	std::cerr << "Serial Port: " << strPort.toUtf8().data() << std::endl;
	std::cerr << "Baud Rate: " << nBaudRate << std::endl;
	if (!strLogFile.isEmpty()) {
		std::cerr << "Log File: " << strLogFile.toUtf8().data() << std::endl;
	}
	if (!strFirmwareIn.isEmpty()) {
		std::cerr << "Input (comparison) Firmware File: " << strFirmwareIn.toUtf8().data() << std::endl;
	}
	if (!strFirmwareOut.isEmpty()) {
		std::cerr << "Output (received) Firmware File: " << strFirmwareOut.toUtf8().data() << std::endl;
	}

	CFrskySportIO sport(nSport);
	if (!sport.openPort(strPort, nBaudRate)) {
		std::cerr << "Failed to open serial port" << std::endl;
		std::cerr << sport.getLastError().toUtf8().data() << std::endl;
		return -2;
	}

	QFile fileFirmwareIn(strFirmwareIn);
	if (!strFirmwareIn.isEmpty() && !fileFirmwareIn.open(QIODevice::ReadOnly)) {
		std::cerr << "Failed to open input firmware file \"" << strFirmwareIn.toUtf8().data() << "\" for reading" << std::endl;
		return -3;
	}

	QFileInfo fiFirmwareIn(fileFirmwareIn);
	bool bIsFrsk = (fiFirmwareIn.suffix().compare("frsk", Qt::CaseInsensitive) == 0);

	QFile fileFirmwareOut(strFirmwareOut);
	if (!strFirmwareOut.isEmpty() && !fileFirmwareOut.open(QIODevice::WriteOnly)) {
		std::cerr << "Failed to open output firmware file \"" << strFirmwareOut.toUtf8().data() << "\" for writing" << std::endl;
		return -4;
	}

	if (!strFirmwareOut.isEmpty()) {
		QFileInfo fiFirmwareOut(fileFirmwareOut);
		if (fiFirmwareOut.suffix().compare("frsk", Qt::CaseInsensitive) == 0) {
			// TODO : Add support for writing .frsk firmware output
			std::cerr << "*** Warning: Output firmware filename is .frsk suffix, but will be written as headerless .frk format and won't be readable by tools unless renamed" << std::endl;
		}
	}

	CLogFile logFile;
	if (!strLogFile.isEmpty()) {
		if (!logFile.openLogFile(strLogFile, QIODevice::WriteOnly)) {
			std::cerr << "Failed to open \"" << strLogFile.toUtf8().data() << "\" for writing" << std::endl;
			std::cerr << logFile.getLastError().toUtf8().data() << std::endl;
			return -5;
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

	if (fileFirmwareIn.isOpen()) {
		if (!emu.setFirmware(fileFirmwareIn, bIsFrsk)) {
			// No need to print the error, as that will happen in the
			//	write message callback above
			return -6;
		}
	}

	bool bSuccess = emu.startDeviceEmulation(CFrskySportDeviceEmu::FRSKDEV_RX, true);

	if (fileFirmwareOut.isOpen() && fileFirmwareOut.isWritable() && !emu.getFirmware().isEmpty()) {
		fileFirmwareOut.write(emu.getFirmware());
	}

	if (fileFirmwareIn.isOpen()) fileFirmwareIn.close();
	if (fileFirmwareOut.isOpen()) fileFirmwareOut.close();

	if (bSuccess) {
		std::cerr << "Emulation was successful" << std::endl;
	} else {
		// No need to print the error, as that will happen in the
		//	write message callback above
		return -7;
	}

	// Don't save persistent settings here, since we aren't changing anything

	return 0;
}

// ============================================================================
