# Frsky Sport Tool

Description
-----------

This is a free, open-source, cross-platform alternative to the closed-source, single-platform of the Frsky tools.  I wrote this because I use Linux and couldn't get the Frsky tools to work in Wine, and using a VM is a pain.  Plus, having my own tools, including command-line tools in addition to a GUI, provides a lot of flexibility and allows for experimentation with custom-devices and more extensive device testing.

This tool suite, though still a work-in-progress, is designed to eventually be a replacement for all Frsky Sport Tool functions.  It includes both a GUI-based tool and command-line based tools that can be used in scripting.  Originally developed on Linux, it is written in C++ and uses Qt5 (and should work with Qt6 and will be migrated there as Qt6 matures) and is designed to be compilable on Windows and Mac as well (though not as thoroughly tested there since the core development is on Linux).

Presently, Sport Firmware Flash Reprogramming is implemented and functioning, both in the GUI and in the stand-alone command-line tool (as I have personally used it to flash firmware into several of my Frsky Receivers).  Additional Sport Telemetry device configuration, monitoring, and emulation is still being worked on.  The included emulator tools, while originally designed to do code testing during development, can actually communicate on the Sport and emulate real devices.

This code will work with a variety of USB-to-Serial options.  One option is to just use the Frsky STK tool.  Another option is to get a cheap FTDI USB Serial adapter, reconfigure it for doing Rx/Tx signal inversion, and modify it to use a single half-duplex wire, as [described in this issue https://github.com/betaflight/betaflight/issues/3364](https://github.com/betaflight/betaflight/issues/3364) or other similar blogs on various RC forums.

Sport protocol specifics were derived from a composite of other open-source projects, such as [opentx](https://www.open-tx.org/), and with active bus monitoring and testing of real Frsky Sport devices, and from various random blogs and forum discussions found online.

**CAUTION:** This is an independent tool and isn't supported or acknowledged by Frsky.  Also, none of the communications protocols nor state flow have been confirmed by Frsky and no standards documentation exists nor has been published for the Sport Protocol to verify functionality.  While it's believed that this code is correct and functional and should work at least as well as "Official Frsky tools", your **use of these tools is at your own risk!!**  No warranty is expressed nor implied.

Running
--------

Linux x86_64-Bit Debian-Based (Ubuntu Bionic and newer, to be exact) AppImages have been created for each of the executables in this suite.  There are currently three executables:
- `frsky_sport_tool`, the GUI version of the tool
- `frsky_firmware_flash`, the command-line firmware flashing tool
- `frsky_device_emu`, the command-line device emulator tool

To run them, simply download the desired AppImage files(s) from the [Releases](https://github.com/dewhisna/frsky_sport_tool/releases), make the file(s) executable either from the file manager or by running `chmod +x appimage` on the command-line, where "appimage" is the name of the file to make executable.  Then simply run it.

Run the command-line tools with no arguments for usage details/help.  Beware that output files are overwritten without warning.  For example, if you use the "-l logfile.log" option on the command-line tools and the file "logfile.log" already exists, it will be overwritten without warning.  So, make sure you are careful with your command-line usage.

Presently, there are only prebuilt binaries for Linux.  For other operating systems, including other flavors of Linux, you'll need to build it yourself from the source code.

Building
--------

To build the Frsky Sport Tool suite, you will need a compiler for your operating system, along with Qt5 and the latest CMake.  On Linux, this usually means `sudo apt install build-essential qt5-default libqt5serialport5 cmake`.  While you can build from the command-line, the easiest solution is to use the `Qt Creator` IDE (`sudo apt install qtcreator`), configure Qt Creator for your system's Qt5 package and CMake, and simply open the `CMakeLists.txt` file from the root `frsky_sport_tool` git clone folder and select "build".

If you want to build from the command-line only, instead of the more preferred Qt Creator IDE, it will look something like this in Linux, assuming that you are using your system's Qt version and not a different Qt you've installed or built yourself, and that you've already installed the build prerequisites above:

```
mkdir -p ~/workspace
cd ~/workspace
git clone https://github.com/dewhisna/frsky_sport_tool.git
mkdir build-frsky_sport_tool
cd build-frsky_sport_tool
cmake -S ../frsky_sport_tool/ -DCMAKE_BUILD_TYPE=Release
make -j 4
```

After the build, the GUI will be in the `build-frsky_sport_tool` folder and the command-line tools will be in their own subfolders underneath that folder.

Note that if you want to build with a Qt other than your system Qt, you'll need to add `-DCMAKE_PREFIX_PATH=$QT_DIR` to the `cmake` line, where `$QT_DIR` is the path to your root Qt installation/build.  The Qt Creator IDE makes it much more convenient to build with different Qt versions, since you can simply select a different "Qt Kit" (compiler/Qt-build combination), have it configure the project for that kit, and then click "build".

Alternatively, there's also a Packer/Docker script for compiling and building the AppImages for this project in the `appimage` folder of the git clone.  To use it, you'll need to [install Packer](https://www.packer.io/) and [install Docker](https://docs.docker.com/engine/install/ubuntu/).  Then just run:

```
mkdir -p ~/workspace
cd ~/workspace
git clone https://github.com/dewhisna/frsky_sport_tool.git
cd frsky_sport_tool/appimage/
packer build packer_docker_build_x86_64_appimage.json
```

When it's done, there should be a `frsky_sport_tool.AppImage.zip` file in that folder with all of the AppImages compiled and ready to run.  There will also be a Docker image from the build that you can use for other build experimentation or code work.  When you are done with the Docker image, you can delete it with `docker image rm localhost:5000/dewtronics/frsky_sport_tool_appimage:latest`.


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
