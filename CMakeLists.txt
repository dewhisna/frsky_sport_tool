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

cmake_minimum_required(VERSION 3.13)

project(frsky_sport_tool LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------

# Compile Options

if(WIN32)
	set(lua_support_default OFF)
else()
	set(lua_support_default ON)
endif()
option(LUA_SUPPORT "Use Lua Scripting" ${lua_support_default})

# -----------------------------------------------------------------------------

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets SerialPort REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets SerialPort REQUIRED)
set(QT_LINK_LIBS
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::SerialPort
)

if(WIN32)
	set(GUI_TYPE WIN32)
endif()
if(APPLE)
	set(GUI_TYPE MACOSX_BUNDLE)
endif()

# -----------------------------------------------------------------------------

# Add a custom command that produces version.cpp, plus
# a dummy output that's not actually produced, in order
# to force version.cmake to always be re-run before the build
add_custom_target(VersionInfo ALL
	${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/version.cmake
	BYPRODUCTS	${CMAKE_BINARY_DIR}/version.cpp
				${CMAKE_BINARY_DIR}/_version.cpp
)

add_library(VersionInfoLib STATIC
	${CMAKE_BINARY_DIR}/version.cpp
)

add_dependencies(VersionInfoLib VersionInfo)

# -----------------------------------------------------------------------------

set(frsky_sport_tool_SOURCES
	main.cpp
	MainWindow.cpp
	PersistentSettings.cpp
	ConfigDlg.cpp
	AboutDlg.cpp
	ProgDlg.cpp
	LogFile.cpp
	frsky_sport_io.cpp
	frsky_sport_firmware.cpp
	frsky_sport_telemetry.cpp
	SaveLoadFileDialog.cpp
	crc.cpp
	${CMAKE_BINARY_DIR}/version.cpp
)
if (LUA_SUPPORT)
	list(APPEND frsky_sport_tool_SOURCES
		lua/LuaScriptDlg.cpp
		lua/LuaEngine.cpp
		lua/LuaEvents.cpp
		lua/LuaLCD.cpp
		lua/LuaGeneral.cpp
	)
endif()

set(frsky_sport_tool_HEADERS
	defs.h
	main.h
	MainWindow.h
	PersistentSettings.h
	ConfigDlg.h
	AboutDlg.h
	UICallback.h
	ProgDlg.h
	LogFile.h
	frsky_sport_io.h
	frsky_sport_firmware.h
	frsky_sport_telemetry.h
	SaveLoadFileDialog.h
	crc.h
	version.h
)
if (LUA_SUPPORT)
	list(APPEND frsky_sport_tool_HEADERS
		lua/LuaScriptDlg.h
		lua/LuaEngine.h
		lua/LuaEvents.h
		lua/LuaLCD.h
		lua/LuaGeneral.h
		lua/Lua_lrotable.h
	)
endif()

set(frsky_sport_tool_FORMS
	MainWindow.ui
	ConfigDlg.ui
	AboutDlg.ui
)
if (LUA_SUPPORT)
	list(APPEND frsky_sport_tool_FORMS
		lua/LuaScriptDlg.ui
		lua/LuaLCD.ui
	)
endif()

set(frsky_sport_tool_RESOURCES
	frsky_sport_tool.qrc
)

if(LUA_SUPPORT)
	include(lua/lua.cmake)
	set(LUA_LIBS luaLib)
else()
	set(LUA_LIBS)
endif()

# -----------------------------------------------------------------------------

add_executable(frsky_sport_tool ${GUI_TYPE}
	${frsky_sport_tool_SOURCES}
	${frsky_sport_tool_HEADERS}
	${frsky_sport_tool_FORMS}
	${frsky_sport_tool_RESOURCES}
)

target_link_libraries(frsky_sport_tool PRIVATE
	${QT_LINK_LIBS}
	VersionInfoLib
	${LUA_LIBS}
)

if(LUA_SUPPORT)
	target_compile_definitions(frsky_sport_tool PRIVATE
		LUA_SUPPORT
	)
	target_include_directories(frsky_sport_tool PRIVATE
		./lua
	)
endif()

add_subdirectory(frsky_firmware_flash)
add_subdirectory(frsky_device_emu)
