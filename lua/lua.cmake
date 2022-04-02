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

#
# Lua importation script
#

cmake_minimum_required(VERSION 3.10)

include(ExternalProject)
find_program(MAKE_EXE NAMES gmake nmake make)

#
# Note: 'LUA_COMPAT_5_3' needed for lua_pushunsigned, and similar,
#	functions used in code hijacked from OpenTx
#

ExternalProject_Add(lua
	GIT_REPOSITORY	https://github.com/lua/lua.git
	GIT_TAG			v5.4.3
	PREFIX			"lua"
	BUILD_COMMAND	${MAKE_EXE} -f makefile TESTS=-DLUA_COMPAT_5_3 all
	BUILD_IN_SOURCE	true
	UPDATE_COMMAND	""
	INSTALL_COMMAND	""
	CONFIGURE_COMMAND	""
	BUILD_BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua/liblua.a"
)

SET(LUA_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lua/src)
SET(LUA_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua)

add_library(luaLib STATIC IMPORTED)
set_target_properties(luaLib PROPERTIES IMPORTED_LOCATION ${LUA_LIB_DIR}/liblua.a)
target_link_libraries(luaLib INTERFACE
	dl
)
target_compile_definitions(luaLib INTERFACE
	-DLUA_COMPAT_5_3
)
add_dependencies(luaLib lua)
target_include_directories(luaLib INTERFACE ${LUA_INCLUDE_DIR})
