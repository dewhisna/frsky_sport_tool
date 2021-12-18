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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <QAction>
#include <QActionGroup>
#include <QScopedPointer>
#include <QFile>
#include <QElapsedTimer>
#include <QTextStream>

// ============================================================================

namespace Ui {
	class CMainWindow;
}

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CMainWindow(QWidget *parent = nullptr);
	~CMainWindow();

protected slots:
	void en_connect(bool bConnect);
	void en_configure();
	void en_writeLogFile(bool bOpen);

protected:
	QFile m_fileLogFile;
	QScopedPointer<QTextStream> m_pLogFile;			// Currently open log file
	QElapsedTimer m_timerLogFile;					// LogFile timestamp keeper

private:
	QPointer<QAction> m_pConnectAction;
	QPointer<QAction> m_pChkWriteLogFile;

	Ui::CMainWindow *ui;
};

// ============================================================================

#endif // MAINWINDOW_H
