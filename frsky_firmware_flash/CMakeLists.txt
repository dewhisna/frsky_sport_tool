##*****************************************************************************
##
## Copyright (C) 2021 Donna Whisnant, a.k.a. Dewtronics.
## Contact: http://www.dewtronics.com/
##
## This file is part of the frsky_sport_tool Application.
##
## GNU General Public License Usage
## This file may be used under the terms of the GNU General Public License
## version 3.0 as published by the Free Software Foundation and appearing
## in the file gpl-3.0.txt included in the packaging of this file. Please
## review the following information to ensure the GNU General Public License
## version 3.0 requirements will be met:
## http://www.gnu.org/copyleft/gpl.html.
##
## Other Usage
## Alternatively, this file may be used in accordance with the terms and
## conditions contained in a signed written agreement between you and
## Dewtronics.
##
##*****************************************************************************

cmake_minimum_required(VERSION 3.10)

project(frsky_firmware_flash LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS SerialPort REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS SerialPort REQUIRED)
set(QT_LINK_LIBS
	Qt${QT_VERSION_MAJOR}::SerialPort
)

# -----------------------------------------------------------------------------

set(frsky_sport_tool_SOURCES
	frsky_firmware_flash.cpp
	../CLIProgDlg.cpp
	../myio.cpp
	../PersistentSettings.cpp
	../frsky_sport_io.cpp
	../frsky_sport_firmware.cpp
	../crc.cpp
)

set(frsky_sport_tool_HEADERS
	../CLIProgDlg.h
	../myio.h
	../defs.h
	../PersistentSettings.h
	../UICallback.h
	../frsky_sport_io.h
	../frsky_sport_firmware.h
	../crc.h
)

# -----------------------------------------------------------------------------

add_executable(frsky_firmware_flash
	${frsky_sport_tool_SOURCES}
	${frsky_sport_tool_HEADERS}
)

target_link_libraries(frsky_firmware_flash PRIVATE
	${QT_LINK_LIBS}
)

target_include_directories(frsky_firmware_flash PRIVATE ..)
