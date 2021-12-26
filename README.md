# Frsky Sport Tool

Description
-----------

This is a free, open-source, cross-platform alternative to the closed-source, single-platform of the Frsky tools.  I wrote this because I use Linux and couldn't get the Frsky tools to work in Wine, and using a VM is a pain.  Plus, having my own tools, including command-line tools in addition to a GUI, provides a lot of flexibility and allows for experimentation with custom-devices and more extensive device testing.

This tool suite, though still a work-in-progress, is designed to eventually be a replacement for all Frsky Sport Tool functions.  It includes both a GUI-based tool and command-line based tools that can be used in scripting.  Originally developed on Linux, it is written in C++ and uses Qt5 (and should work with Qt6 and will be migrated there as Qt6 matures) and is designed to be compilable on Windows and Mac as well (though not as thoroughly tested there since the core development is on Linux).

Presently, Sport Firmware Flash Reprogramming is implemented and functioning, both in the GUI and in the stand-alone command-line tool (as I have personally used it to flash firmware into several of my Frsky Receivers).  Additional Sport Telemetry device configuration, monitoring, and emulation is still being worked on.  The included emulator tools, while originally designed to do code testing during development, can actually communicate on the bus and emulate real devices.

This code will work with a variety of USB-to-Serial options.  One option is to just use the Frsky STK tool.  Another option is to get a cheap FTDI USB Serial adapter, reconfigure it for doing Rx/Tx signal inversion, and modify it to use a single half-duplex wire, as [described in this blog https://github.com/betaflight/betaflight/issues/3364](https://github.com/betaflight/betaflight/issues/3364) or other similar blogs on various RC forums.

Sport protocol specifics were derived from a composite of other open-source projects, such as [opentx](https://www.open-tx.org/), and with active bus monitoring and testing of real Frsky Sport devices, and from various random blogs and forum discussions.

To build the Frsky Sport Tool suite, you will need a compiler for your operating system, along with Qt5 and the latest CMake.  On Linux, this usually means `sudo apt install build-essential qt5-default cmake`.  While you can build from the command-line, the easiest solution is to use the `Qt Creator` IDE (`sudo apt install qtcreator`), configure Qt Creator for your system's Qt5 package and CMake, and simply open the `CMakeLists.txt` file from the root `frsky_sport_tool` git clone folder and select "build".

License
-------

Frsky Sport Tool

Copyright(c) 2021 Donna Whisnant, a.k.a. Dewtronics.

Contact: <http://www.dewtronics.com/>

GNU General Public License Usage

This content may be used under the terms of the GNU General Public License
version 3.0 as published by the Free Software Foundation and appearing
in the file gpl-3.0.txt included in the packaging of this app. Please
review the following information to ensure the GNU General Public License
version 3.0 requirements will be met:

<http://www.gnu.org/copyleft/gpl.html>


Other Usage:

Alternatively, this repository may be used in accordance with the terms
and conditions contained in a signed written agreement between you and
Dewtronics, a.k.a. Donna Whisnant.

See '[LICENSE](./LICENSE.txt)' for the full content of the license.
