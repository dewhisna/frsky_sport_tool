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

#include "AboutDlg.h"
#include "ui_AboutDlg.h"

#include "version.h"

// ============================================================================

CAboutDlg::CAboutDlg(QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
	ui(new Ui::CAboutDlg)
{
	ui->setupUi(this);

	QString strREV = GIT_REV;
	QString strTAG = GIT_TAG;
	QString strBRANCH = GIT_BRANCH;

	if (!strTAG.isEmpty()) {
		ui->lblVersion->setText(tr("Version: %1").arg(strTAG));
	} else {
		ui->lblVersion->setText(tr("Git Build Version: %1/%2").arg(strBRANCH, strREV));
	}

	ui->editLicense->setText(
				tr("This program is free software; you can redistribute it and/or modify it under the terms "
				"of the GNU General Public License as published by the Free Software Foundation; either "
				"version 3 of the License, or (at your option) any later version.\n\n"
				"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
				"without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  "
				"See the GNU General Public License for more details.\n\n"
				"You should have received a copy of the GNU General Public License along with this program; "
				"if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n"
				"Copyright (C) 2021 Donna Whisnant, a.k.a. Dewtronics.\n"
				"Contact: http://www.dewtronics.com/\n", "AboutBox"));

//	updateGeometry();
//	adjustSize();
}

CAboutDlg::~CAboutDlg()
{
	delete ui;
	ui = nullptr;
}

// ============================================================================
