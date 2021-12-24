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

#ifndef UI_CALLBACK_H
#define UI_CALLBACK_H

#include <QObject>
#include <QString>

// Note: So that this can work without a GUI (such as from a
//	command-line tool), we'll avoid including anything here
//	that requires QtWidgets, such as QMessageBox.  Instead,
//	we'll use standard types, such as int32_t for StandardButton
//	values, and caller can cast to/from GUI types as needed.

// ============================================================================

typedef int32_t BTN_TYPE;		// Base button value type for button enums (see above)

class CUICallback
{
public:
	enum PROMPT_TYPE {
		PT_CRITICAL,
		PT_INFORMATION,
		PT_QUESTION,
		PT_WARNING
	};

	// Buttons : Designed to mirror QMessageBox::Buttons,
	//	but included here so we can support non-GUI,
	//	command-line tools
	enum BUTTONS : BTN_TYPE {
		NoButton           = 0x00000000,
        Ok                 = 0x00000400,
        Save               = 0x00000800,
        SaveAll            = 0x00001000,
        Open               = 0x00002000,
        Yes                = 0x00004000,
        YesToAll           = 0x00008000,
        No                 = 0x00010000,
        NoToAll            = 0x00020000,
        Abort              = 0x00040000,
        Retry              = 0x00080000,
        Ignore             = 0x00100000,
        Close              = 0x00200000,
        Cancel             = 0x00400000,
        Discard            = 0x00800000,
        Help               = 0x01000000,
        Apply              = 0x02000000,
        Reset              = 0x04000000,
        RestoreDefaults    = 0x08000000,
	};

	virtual void setProgressRange(int nMin, int nMax) = 0;
	virtual void setProgressPos(int nValue) = 0;
	virtual void setProgressText(const QString &strStatus) = 0;
	virtual void enableCancel(bool bEnable) = 0;
	virtual void hookCancel(QObject *pObject, const char *member) = 0;
	virtual void unhookCancel(QObject *pObject, const char *member) = 0;
	// ----
	virtual BTN_TYPE promptUser(PROMPT_TYPE nPromptType,
						const QString &strText,
						BTN_TYPE nButtons,
						BTN_TYPE nDefaultButton) = 0;
	// ----
	virtual void setRxDataStatusLED(bool bStatus) = 0;		// Rx Data Status LED, On/Off or Color change
	virtual void setTxDataStatusLED(bool bStatus) = 0;		// Tx Data Status LED, On/Off or Color change
};

// ============================================================================

#endif	// UI_CALLBACK_H

