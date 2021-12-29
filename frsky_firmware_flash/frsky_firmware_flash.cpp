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

#include <LogFile.h>
#include <frsky_sport_io.h>
#include <frsky_sport_firmware.h>
#include <CLIProgDlg.h>

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

	QString strFirmware;
	QString strLogFile;
	SPORT_ID_ENUM nSport = CPersistentSettings::instance()->getFirmwareSportPort();
	QString strPort = CPersistentSettings::instance()->getDeviceSerialPort(nSport);
	bool bHavePortNameSetting =!strPort.isEmpty();
	int nBaudRate = CPersistentSettings::instance()->getDeviceBaudRate(nSport);
	int nDataBits = CPersistentSettings::instance()->getDeviceDataBits(nSport);
	char chParity = CPersistentSettings::instance()->getDeviceParity(nSport);
	int nStopBits = CPersistentSettings::instance()->getDeviceStopBits(nSport);
	bool bInteractive = false;
	bool bNeedUsage = false;
	int nArgsFound = 0;

	QStringList lstDefaultPortSettings;
	lstDefaultPortSettings.append(QString("%1").arg(CPersistentSettings::instance()->getDeviceDataBits(nSport)));
	lstDefaultPortSettings.append(QString("%1").arg(QChar(CPersistentSettings::instance()->getDeviceParity(nSport))));
	lstDefaultPortSettings.append(QString("%1").arg(CPersistentSettings::instance()->getDeviceStopBits(nSport)));

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
		} else if (strArg.startsWith("-s")) {
			QString strPortSettings;
			if ((strArg == "-s") && (argc > ndx+1)) {
				strPortSettings = argv[ndx+1];
				++ndx;
			} else {
				strPortSettings = strArg.mid(2);
			}
			QStringList lstPortSettings = strPortSettings.split(",", Qt::KeepEmptyParts);
			if (lstPortSettings.size() >= 1) {
				nDataBits = strtoul(lstPortSettings.at(0).toUtf8().data(), nullptr, 0);
			}
			if (lstPortSettings.size() >= 2) {
				if (lstPortSettings.at(1).size() > 0) chParity = lstPortSettings.at(1).toUpper().at(0).toLatin1();
			}
			if (lstPortSettings.size() >= 3) {
				nStopBits = strtoul(lstPortSettings.at(2).toUtf8().data(), nullptr, 0);
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
	if (strFirmware.isEmpty() || strPort.isEmpty()) bNeedUsage = true;

	if (bNeedUsage) {
		std::cerr << "Frsky Firmware Flash Programming Tool" << std::endl;
		std::cerr << "Version: " << strVersion.toUtf8().data() << std::endl << std::endl;
		if (bHavePortNameSetting) {
			std::cerr << "Usage: frsky_firmware_flash [options] <firmware-filename> [<port>]" << std::endl;
		} else {
			std::cerr << "Usage: frsky_firmware_flash [options] <firmware-filename> <port>" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Where:" << std::endl;
		std::cerr << "    <firmware-filename> = File name/path to firmware file (required)" << std::endl;
		std::cerr << "    <port> = Serial Port to use ";
		if (bHavePortNameSetting) {
			std::cerr	<< "(Optional, will use current setting"  << std::endl
						<< "             of \"" << CPersistentSettings::instance()->getDeviceSerialPort(nSport).toUtf8().data() << "\" if not specified)" << std::endl;
		} else {
			std::cerr << "(required)" << std::endl;
		}
		std::cerr << std::endl;
		std::cerr << "Options:" << std::endl;
		std::cerr << "    -b <baudrate> = optional baud-rate specifier" << std::endl;
		std::cerr << "                    (if omitted, will use the current setting of " << CPersistentSettings::instance()->getDeviceBaudRate(nSport) << ")" << std::endl;
		std::cerr << "    -s <port-settings> = where port-settings is a comma separated list of" << std::endl;
		std::cerr << "                    \"DataBit,Parity,StopBit\", such as \"8,E,2\"" << std::endl;
		std::cerr << "                    If omitted, will use current setting of \"" << lstDefaultPortSettings.join(',').toUtf8().data() << "\"" << std::endl;
		std::cerr << "    -l <logfile>  = optional communications log file to generate" << std::endl;
		std::cerr << "    -i = interactive mode, enables prompts" << std::endl;
		std::cerr << std::endl << std::endl;

		return -1;
	}

	std::cerr << "frsky_firmware_flash version: " << strVersion.toUtf8().data() << std::endl;

	CFrskySportIO sport(nSport);
	if (!sport.openPort(strPort, nBaudRate, nDataBits, chParity, nStopBits)) {
		std::cerr << "Failed to open serial port" << std::endl;
		std::cerr << sport.getLastError().toUtf8().data() << std::endl;
		return -2;
	}

	QStringList lstPortSettings;
	lstPortSettings.append(QString("%1").arg(sport.dataBits()));
	lstPortSettings.append(QString("%1").arg(QChar(sport.parity())));
	lstPortSettings.append(QString("%1").arg(sport.stopBits()));

	if (bInteractive) std::cerr << "Interactive Mode" << std::endl;
	std::cerr << "Serial Port: " << strPort.toUtf8().data() << std::endl;
	std::cerr << "Baud Rate: " << sport.baudRate() << std::endl;
	std::cerr << "Port Settings: " << lstPortSettings.join(',').toUtf8().data() << std::endl;
	if (!strLogFile.isEmpty()) {
		std::cerr << "Log File: " << strLogFile.toUtf8().data() << std::endl;
	}
	std::cerr << "Firmware File: " << strFirmware.toUtf8().data() << std::endl;


	QFile fileFirmware(strFirmware);
	if (!fileFirmware.open(QIODevice::ReadOnly)) {
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
	CFrskyDeviceFirmwareUpdate fsm(sport, &dlgProg);

	dlgProg.setInteractive(bInteractive);

	if (!fsm.flashDeviceFirmware(fileFirmware, bIsFrsk, true)) {
		std::cerr << fsm.getLastError().toUtf8().data() << std::endl;
		return -5;
	} else {
		std::cerr << "Programming was successful" << std::endl;
	}

	// Don't save persistent settings here, since we aren't changing anything

	return 0;
}

// ============================================================================
