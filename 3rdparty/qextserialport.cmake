cmake_minimum_required(VERSION 3.10)

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

# -----------------------------------------------------------------------------

set(qextserialport_DIR ${CMAKE_CURRENT_LIST_DIR}/qextserialport/src)

set(qextserialport_SOURCES
	${qextserialport_DIR}/qextserialport.cpp
	${qextserialport_DIR}/qextserialenumerator.cpp
)

set(qextserialport_PUBLIC_HEADERS
	${qextserialport_DIR}/qextserialport.h
	${qextserialport_DIR}/qextserialenumerator.h
	${qextserialport_DIR}/qextserialport_global.h
)

set(qextserialport_HEADERS
	${qextserialport_PUBLIC_HEADERS}
	${qextserialport_DIR}/qextserialport_p.h
	${qextserialport_DIR}/qextserialenumerator_p.h
)

if (UNIX)
	list(APPEND qextserialport_SOURCES
		${qextserialport_DIR}/qextserialport_unix.cpp
	)
	if (LINUX)
		list(APPEND qextserialport_SOURCES
			${qextserialport_DIR}/qextserialenumerator_linux.cpp
		)
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		# macx:
		list(APPEND qextserialport_SOURCES
			${qextserialport_DIR}/qextserialenumerator_osx.cpp
		)
	else()
		list(APPEND qextserialport_SOURCES
			${qextserialport_DIR}/qextserialenumerator_unix.cpp
		)
	endif()
endif()

if (WIN32)
	list(APPEND qextserialport_SOURCES
		${qextserialport_DIR}/qextserialport_win.cpp
		${qextserialport_DIR}/qextserialenumerator_win.cpp
	)
endif()

add_library(qextserialport STATIC
	${qextserialport_SOURCES}
	${qextserialport_HEADERS}
)

target_link_libraries(qextserialport PRIVATE
	${QT_LINK_LIBS}
)

if (LINUX)
	option(QESP_LINUX_UDEV "Use qesp_linux_udev" OFF)

	if (NOT QESP_LINUX_UDEV)
		target_compile_definitions(qextserialport PRIVATE
			QESP_NO_UDEV
		)
	else()
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(UDEV REQUIRED IMPORTED_TARGET udev>=237)

		target_link_libraries(qextserialport PRIVATE
			PkgConfig::UDEV
		)
	endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	# macx:
	target_link_libraries(qextserialport PRIVATE
		IOKit
		CoreFoundation
	)
endif()

if (WIN32)
	target_link_libraries(qextserialport PRIVATE
		setupapi
		advapi32
		user32
	)
endif()

# moc doesn't detect Q_OS_LINUX correctly, so add this to make it work
if (LINUX)
	target_compile_definitions(qextserialport PRIVATE
		__linux__
	)
endif()

target_include_directories(qextserialport INTERFACE
	${qextserialport_DIR}
)

# -----------------------------------------------------------------------------
