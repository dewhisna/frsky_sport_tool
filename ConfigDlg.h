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

#ifndef CONFIGDLG_H
#define CONFIGDLG_H

#include <QDialog>

#include <QSerialPortInfo>

// ============================================================================

namespace Ui {
	class CConfigDlg;
}

class CConfigDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CConfigDlg(QWidget *parent = nullptr);
	~CConfigDlg();

	const QSerialPortInfo &Sport1SerialPortInfo() const { return m_selectedSport1SerialPort; }

protected slots:
	void en_selectSport1SerialPort(int nIndex);

private:
	QSerialPortInfo m_selectedSport1SerialPort;

	Ui::CConfigDlg *ui;
};

// ============================================================================

#endif // CONFIGDLG_H
