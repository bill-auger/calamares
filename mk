#!/bin/bash


[ -d build ] || mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug                        \
      -DCMAKE_INSTALL_PREFIX=/usr                     \
      -DSKIP_MODULES="interactiveterminal webview" ..
[ -f Makefile ] && make
sudo make uninstall ; sudo make install
cd ..

[ "`lsmod | grep squashfs`" ] || sudo modprobe squashfs
[ -f /usr/bin/calamares     ] && sudo calamares
