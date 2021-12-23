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

#ifndef DEFS_H_
#define DEFS_H_

#include <stdint.h>

#if !defined(UNUSED)
  #define UNUSED(x)           ((void)(x)) /* to avoid warnings */
#endif

#if !defined(_countof)
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

#if __GNUC__
  #define PACK( __Declaration__ )      __Declaration__ __attribute__((__packed__))
#else
  #define PACK( __Declaration__ )      __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

#endif	// DEFS_H_

