#!/bin/bash


sudo echo "\n--- running pacman ---\n"
BASE_PKGS="$(pacman -Qg base-devel | cut -d ' ' -f 2)"
CALAMARES_PKGS="boost extra-cmake-modules git kpmcore qt5-tools polkit-qt5 yaml-cpp" # squashfs-tools os-prober
PARABOLA_PKGS="arch-install-scripts"
PKGS="$BASE_PKGS $CALAMARES_PKGS $PARABOLA_PKGS"
pacman -Qi calamares > /dev/null 2>&1 && sudo pacman -R calamares
pacman -Qi $PKGS > /dev/null || sudo pacman -S --needed $PKGS


echo "\n--- preparing build environment ---\n"
if [ ! -d build ]
then mkdir build
# else rm -f build/Makefile 2> /dev/null
fi
# if [ ! -d /etc/calamares/ ]
# then sudo mkdir                         /etc/calamares
#      sudo ln -s `pwd`/settings.conf     /etc/calamares/
#      sudo ln -s `pwd`/src/branding      /etc/calamares/
# fi


echo "\n--- running cmake ---\n"
cd build
cmake -DCMAKE_BUILD_TYPE=Debug                                                       \
      -DCMAKE_INSTALL_PREFIX=/usr                                                    \
      -DCMAKE_INSTALL_LIBDIR=lib                                                     \
      -DSKIP_MODULES="dracut dracutlukscfg dummycpp dummyprocess dummypython         \
                      dummypythonqt grubcfg initramfs initramfscfg tracking          \
                      interactiveterminal license plymouthcfg removeuser webview" ..


echo "\n--- running make uninstall ---\n"
[ -d /usr/share/calamares/ ] && sudo make uninstall
sudo rm -rf /usr/share/calamares/


echo "\n--- running make install ---\n"
[ -f Makefile  ] && sudo make install
cd ..


echo "\n--- launching calamares ---\n"
# [ "`lsmod | grep squashfs`" ] || sudo modprobe squashfs
[ -f /usr/bin/calamares     ] && sudo calamares
# [ -f build/calamares        ] && sudo build/calamares
