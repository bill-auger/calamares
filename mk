#!/bin/bash

readonly NO_UPGRADE=1
readonly RUN_INSTALLED=1
readonly THIS_BUILD_HOST==$(grep '^ID=' /etc/os-release | cut -d '=' -f 2)


function print() { printf "\033[01;34m%s\033[00m\n" "$(echo -e $*)" ; }


sudo echo -n
if pacman --version 2> /dev/null | grep libalpm > /dev/null
then print "\n--- running pacman ---\n"
     BASE_PKGS="$(pacman -Qg base-devel | cut -d ' ' -f 2)"
     CALAMARES_PKGS="boost extra-cmake-modules git kconfig kpmcore kservice kwindowsystem \
                     plasma-framework qt5-tools polkit-qt5 yaml-cpp" # squashfs-tools os-prober
#      PARABOLA_PKGS="arch-install-scripts"
     PKGS="$BASE_PKGS $CALAMARES_PKGS $PARABOLA_PKGS"
     pacman -Qi calamares > /dev/null 2>&1               && sudo pacman -R calamares
     pacman -Qi $PKGS     > /dev/null && (($NO_UPGRADE)) || sudo pacman -Sy --needed $PKGS || exit 1
elif which apt-get &> /dev/null
then print "\n--- running apt ---\n"
     sudo apt-get install -qqq extra-cmake-modules libatasmart-dev libboost-python-dev \
                               libkf5coreaddons-dev libkf5kio-dev libkf5plasma-dev     \
                               libkf5service-dev libkpmcore4-dev libparted-dev         \
                               libpolkit-qt5-1-dev qtdeclarative5-dev                    || exit 1
fi


[ "$(cat ./LAST_BUILD_HOST)" == "$THIS_BUILD_HOST" ] || (echo "cleaning build/ dir" ; sudo rm -rf ./build)
echo "$THIS_BUILD_HOST" > ./LAST_BUILD_HOST
if [ ! -d ./build ]
then print "\n--- preparing build environment ---\n"
     mkdir ./build
# else rm -f build/Makefile 2> /dev/null
fi
# if [ ! -d /etc/calamares/ ]
# then sudo mkdir                         /etc/calamares
#      sudo ln -s `pwd`/settings.conf     /etc/calamares/
#      sudo ln -s `pwd`/src/branding      /etc/calamares/
# fi


print "\n--- running cmake ---\n"
cd ./build
cmake -DCMAKE_BUILD_TYPE=Debug                                                 \
      -DCMAKE_INSTALL_PREFIX=/usr                                              \
      -DCMAKE_INSTALL_LIBDIR=lib                                               \
      -DSKIP_MODULES="dracut dracutlukscfg dummycpp dummyprocess dummypython   \
                      dummypythonqt initramfs initramfscfg interactiveterminal \
                      license plymouthcfg removeuser tracking webview"           ..


if [ -d /usr/share/calamares/ -o -f /usr/bin/calamares ]
then print "\n--- running make uninstall ---\n"
     sudo make uninstall
     sudo rm -rf /usr/share/calamares/ /usr/bin/calamares
fi


if [ ! -f Makefile ]
then print "\n--- Makefile does not exist - bailing ---\n"
     cd ..
     exit 1
fi


if (($RUN_INSTALLED))
then print "\n--- running make install ---\n"
     sudo make install || exit 1
else print "\n--- running make ---\n"
     make              || exit 1
fi
cd ..


if (($RUN_INSTALLED)) && [ -f /usr/bin/calamares ]
then print "\n--- launching calamares installed ---\n"
     sudo calamares
elif [ -f build/calamares ]
then print "\n--- launching calamares uninstalled ---\n"
     cp ./settings.conf ./build
     cd ./build
     sudo ./calamares -d
     cd ..
fi
