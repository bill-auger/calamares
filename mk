#!/bin/bash


sudo echo
echo "\n--- running pacman ---\n"
pacman -Qi calamares > /dev/null && sudo pacman -R calamares
# PKGS="$(pacman -Qg base-devel | cut -d ' ' -f 2)"
# pacman -Qi $PKGS > /dev/null || sudo pacman -S $PKGS
PKGS="git extra-cmake-modules qt5-tools yaml-cpp polkit-qt5 \
      kpmcore boost"
#      kpmcore boost os-prober"
pacman -Qi $PKGS > /dev/null || sudo pacman -S --needed $PKGS
PKGS="arch-install-scripts"
# squashfs-tools
pacman -Qi $PKGS > /dev/null || sudo pacman -S --needed $PKGS


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
cmake -DCMAKE_BUILD_TYPE=Debug                                               \
      -DCMAKE_INSTALL_PREFIX=/usr                                            \
      -DCMAKE_INSTALL_LIBDIR=lib                                             \
      -DWITH_CRASHREPORTER=OFF                                               \
      -DSKIP_MODULES="dracut dracutlukscfg dummycpp dummyprocess dummypython \
                      dummypythonqt grubcfg initramfs initramfscfg           \
                      interactiveterminal license luksbootkeyfile            \
                      luksopenswaphookcfg plymouthcfg removeuser webview" ..

echo "\n--- running make uninstall ---\n"
sudo make uninstall
sudo rm -rf /usr/share/calamares/

echo "\n--- running make install ---\n"
[ -f Makefile  ] && sudo make install
cd ..

# [ "`lsmod | grep squashfs`" ] || sudo modprobe squashfs
[ -f /usr/bin/calamares     ] && sudo calamares
# [ -f build/calamares        ] && sudo build/calamares
