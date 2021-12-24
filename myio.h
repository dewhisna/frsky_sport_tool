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

//
// conio replacement for getch/getche to work on Linux
//

#ifndef MY_IO_H
#define MY_IO_H

#include <stdio.h>
#include <string.h>

extern char my_getch(void);
extern char my_getche(void);
extern char getch1(bool bEcho = true);
extern int my_kbhit(void);

// ============================================================================

constexpr int INPUT_BUF_SIZE = 512;

extern char g_inputBuf[INPUT_BUF_SIZE];

#define my_scanf(format, ...) ((fgets(g_inputBuf, sizeof(g_inputBuf), stdin) != nullptr) ? sscanf(g_inputBuf, format, ##__VA_ARGS__) : 0)

extern bool getDouble(double *aDouble);
extern bool getInt(int *anInt);
extern bool getUInt(unsigned int *anUInt);
extern bool getUIntHex(unsigned int *anUInt);
extern bool getString(char *pBuf, int nBufSize);

// ============================================================================

#endif	// MY_IO_H
