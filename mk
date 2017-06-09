#!/bin/bash


# sudo pacman -S base-devel git extra-cmake-modules qt5-tools yaml-cpp polkit-qt5 \
#                kpmcore boost
# sudo pacman -S squashfs-tools arch-install-scripts


if [ ! -d build ]
then mkdir build
else rm -f build/Makefile 2> /dev/null
fi
# if [ ! -d /etc/calamares/ ]
# then sudo mkdir                         /etc/calamares
#      sudo ln -s `pwd`/settings.conf     /etc/calamares/
#      sudo ln -s `pwd`/src/branding      /etc/calamares/
# fi


cd build
cmake -DCMAKE_BUILD_TYPE=Debug                                               \
      -DCMAKE_INSTALL_PREFIX=/usr                                            \
      -DCMAKE_INSTALL_LIBDIR=lib                                             \
      -DWITH_CRASHREPORTER=OFF                                               \
      -DSKIP_MODULES="dracut dracutlukscfg dummycpp dummyprocess dummypython \
                      dummypythonqt grubcfg initramfs initramfscfg           \
                      interactiveterminal license luksbootkeyfile            \
                      luksopenswaphookcfg plymouthcfg removeuser webview" ..
[ -f Makefile  ] && make ; (($?)) && cd .. && exit
#sudo make uninstall
sudo make install
cd ..


[ "`lsmod | grep squashfs`" ] || sudo modprobe squashfs
[ -f /usr/bin/calamares     ] && sudo calamares
# [ -f build/calamares        ] && sudo build/calamares

