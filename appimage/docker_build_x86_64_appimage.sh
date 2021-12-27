#!/bin/bash

set -ex

export DEBIAN_FRONTEND=noninteractive

export USER=ubuntu

export BUILD_HOME=/home/$USER
export INSTALL_DIR=/usr/local/bin/

export QT=$BUILD_HOME/Qt
export QT_DIR=$QT/5.15.2

export PROJECT_DIR=$BUILD_HOME/Documents/programs
export BUILD_TARGET=Qt_5_15_2_gcc_64


# ------------
# Create User:
# ------------
useradd -m -U -G adm,dialout,cdrom,sudo,dip,plugdev -s /bin/bash $USER
echo $USER:U6aMy0wojraho | chpasswd -e
#passwd --expire $USER


# ---------------------
# Install Dependencies:
# ---------------------
apt-get update
apt-get install -y software-properties-common
#sed -i '/^#\s*deb-src /s/^#\s*//' "/etc/apt/sources.list"
apt-get install -y build-essential
apt-get install -y pkg-config
apt-get install -y cmake
apt-get install -y gperf bison
apt-get install -y libgl1-mesa-dev
# Easiest to slurp in Qt5 build dependencies, since we are building Qt5 as a pre-requisite:
# apt-get build-dep -qq --yes libqt5core5a libqt5gui5 libqt5widgets5 libqt5serialport5
apt-get install -y libqt5core5a libqt5gui5 libqt5widgets5 libqt5serialport5
# These needed for plugins that don't get picked up with above
apt-get install -y libwebpmux3 libwebpdemux2 libwebp6
# A few helpers:
#apt-get install -y perl python
apt-get install -y binfmt-support
apt-get install -y wget
apt-get install -y curl git xz-utils
apt-get install -y zip unzip
apt-get install -y joe
apt-get install -y sudo


# ----------------------
# Install linuxdeployqt:
# ----------------------
cd $BUILD_HOME
wget --no-verbose -nc https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
chmod +x linuxdeployqt-continuous-x86_64.AppImage
./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract
mv squashfs-root linuxdeployqt
ln -s $BUILD_HOME/linuxdeployqt/AppRun /usr/local/bin/linuxdeployqt
rm -f linuxdeployqt-continuous-x86_64.AppImage


# ------------------------
# Setup Container Startup:
# ------------------------
(
cat <<'EOF'
#!/bin/bash

su -l ubuntu
EOF
) > /bin/startup
chmod 755 /bin/startup


# -------------------
# Get Git Repository:
# -------------------
cd $BUILD_HOME
mkdir -p $PROJECT_DIR
cd $PROJECT_DIR
git clone https://github.com/dewhisna/frsky_sport_tool.git
cd frsky_sport_tool
#sed -i 's/git@github.com:/https:\/\/github.com\//' .gitmodules
#git submodule update --init --recursive


# ----------------
# Get prebuilt Qt:
# ----------------
# From: http://download.qt-project.org/official_releases/qt/5.15/5.15.2/
#   Or: https://download.qt.io/official_releases/qt/5.15/5.15.2/
cd $BUILD_HOME
wget --no-verbose -nc https://cloud.dewtronics.com/hidden/ebe899bf-1dff-4b04-8e1b-8e63895a963d/Qt_5.15.2_18.04_bionic_gcc_64.tar.xz
echo '8287879f0bd6233a050befb3b3f893cf12a6d0cb  Qt_5.15.2_18.04_bionic_gcc_64.tar.xz' | sha1sum -c
tar -Jxf Qt_5.15.2_18.04_bionic_gcc_64.tar.xz
chown -R $USER:$USER Qt/
# ---------------------
# Link Qt5 Source -> Qt4 Paths for zlib:
#  This has already been applied to the prebuilt binary
#  above.  If building a custom Qt, apply the follow to
#  copy the 3rdparty/zlib* source folders (from the Qt
#  source code) to the Qt binary folder.
# ---------------------
# cd $$[QT_INSTALL_PREFIX]
# mkdir -p src/3rdparty
# cd src/3rdparty/
# cp -r $QT_SRC/qtbase/src/3rdparty/zlib* .
# ---------------------


# --------------------
# Configure and Build:
# --------------------
mkdir -p $PROJECT_DIR/build-frsky_sport_tool-$BUILD_TARGET/Release
cd $PROJECT_DIR/build-frsky_sport_tool-$BUILD_TARGET/Release
cmake -S ../../frsky_sport_tool/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$QT_DIR
make -j 4


# ----------------
# Build appimages:
# ----------------
# Export Version:
cd $PROJECT_DIR/frsky_sport_tool/
export VERSION=`git describe --always --dirty=+`
# Stage Build Area:
mkdir -p $PROJECT_DIR/appimage/frsky_firmware_flash
mkdir -p $PROJECT_DIR/appimage/frsky_device_emu
mkdir -p $PROJECT_DIR/appimage/frsky_sport_tool
# frsky_firmware_flash CLI:
cd $PROJECT_DIR/appimage/frsky_firmware_flash/
cp $PROJECT_DIR/build-frsky_sport_tool-$BUILD_TARGET/Release/frsky_firmware_flash/frsky_firmware_flash .
cp $PROJECT_DIR/frsky_sport_tool/frsky_firmware_flash/frsky_firmware_flash.desktop .
cp $PROJECT_DIR/frsky_sport_tool/frsky_firmware_flash/frsky_firmware_flash.png .
linuxdeployqt frsky_firmware_flash.desktop -appimage -no-translations -qmake=$QT_DIR/bin/qmake
# frsky_device_emu CLI:
cd $PROJECT_DIR/appimage/frsky_device_emu/
cp $PROJECT_DIR/build-frsky_sport_tool-$BUILD_TARGET/Release/frsky_device_emu/frsky_device_emu .
cp $PROJECT_DIR/frsky_sport_tool/frsky_device_emu/frsky_device_emu.desktop .
cp $PROJECT_DIR/frsky_sport_tool/frsky_device_emu/frsky_device_emu.png .
linuxdeployqt frsky_device_emu.desktop -appimage -no-translations -qmake=$QT_DIR/bin/qmake
# frsky_sport_tool GUI:
cd $PROJECT_DIR/appimage/frsky_sport_tool/
cp $PROJECT_DIR/build-frsky_sport_tool-$BUILD_TARGET/Release/frsky_sport_tool .
cp $PROJECT_DIR/frsky_sport_tool/frsky_sport_tool.desktop .
cp $PROJECT_DIR/frsky_sport_tool/frsky_sport_tool4.png .
linuxdeployqt frsky_sport_tool.desktop -appimage -no-translations -qmake=$QT_DIR/bin/qmake


# --------------
# Zip appimages:
# --------------
cd $PROJECT_DIR
mv $PROJECT_DIR/appimage/frsky_firmware_flash/*.AppImage .
mv $PROJECT_DIR/appimage/frsky_device_emu/*.AppImage .
mv $PROJECT_DIR/appimage/frsky_sport_tool/*.AppImage .
zip frsky_sport_tool.AppImage.zip *.AppImage
mv frsky_sport_tool.AppImage.zip $BUILD_HOME/
chmod 664 $BUILD_HOME/frsky_sport_tool.AppImage.zip


# ---------------------
# Cleanup Permissions
# ---------------------
chown -R $USER:$USER $BUILD_HOME


# ---------------------
# Cleanup Image
# ---------------------
#rm -rf $PROJECT_DIR/frsky_sport_tool/
#rm -rf $PROJECT_DIR/build-frsky_sport_tool-$BUILD_TARGET/
apt-get clean -y
apt-get autoclean -y
apt-get autoremove -y

# This would save ~150MB, at the expense of breaking apt-get
# until a fresh apt-get update. Omitted for now
#rm -r /var/lib/apt/lists/*


