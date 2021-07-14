#!/bin/sh
set -ex

git clone https://github.com/davisking/dlib.git
cd dlib
git checkout $(git describe --tags --abbrev=0 --exclude="*-rc*")
patch -p1 < ../ci/common/dlib-slim.patch
patch -p1 < ../ci/common/dlib-cmake-no-openblasp.patch
cd ..

git clone https://github.com/norihiro/obs-ptz.git
cd obs-ptz
git checkout ${OBSPTZTag}
patch -p1 < ../ci/common/obs-ptz-WITH_PTZ_SERIAL.patch
cd -

cp LICENSE data/LICENSE-plugin
cp dlib/LICENSE.txt data/LICENSE-dlib

sed -i 's;${CMAKE_INSTALL_FULL_LIBDIR};/usr/lib;' CMakeLists.txt

mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DWITH_PTZ_SERIAL=ON ..
make -j4
