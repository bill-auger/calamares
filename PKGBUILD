# Maintainer: bill-auger <bill-auger@programmer.net>


pkgname=calamares
pkgver=3.1.0
pkgrel=1
pkgdesc='Distribution-independent installer framework'
arch=('i686' 'x86_64')
license=(GPL)
url="https://github.com/calamares"
license=('LGPL')

depends=( 'boost-libs' 'dmidecode' 'gptfdisk' 'hwinfo' 'kconfig' 'kcoreaddons' 'ki18n' 
          'kparts' 'kpmcore' 'polkit-qt5' 'python' 'squashfs-tools' 'solid' 'qt5ct'
          'qt5-styleplugins' 'qt5-svg' 'yaml-cpp' )
makedepends=( 'extra-cmake-modules' 'qt5-tools' 'git' 'boost' )
backup=( 'usr/share/calamares/modules/bootloader.conf'
         'usr/share/calamares/modules/displaymanager.conf'
         'usr/share/calamares/modules/initcpio.conf'
         'usr/share/calamares/modules/unpackfs.conf' )

_git_tag="${pkgver}-parabola"
source=("https://notabug.org/bill-auger/calamares/archive/v${_git_tag}.tar.gz")
sha256sums=('fa9ecaa6093112e1f9bcfea16cd20a0e8b03f192defa40e9d213207117f9d4f7')


build() {
  cd ${srcdir}/calamares

  mkdir -p build
  cd build
  cmake -DCMAKE_BUILD_TYPE=Debug                                               \
        -DCMAKE_INSTALL_PREFIX=/usr                                            \
        -DCMAKE_INSTALL_LIBDIR=lib                                             \
        -DWITH_CRASHREPORTER=OFF                                               \
        -DSKIP_MODULES="dracut dracutlukscfg dummycpp dummyprocess dummypython \
                        dummypythonqt grubcfg initramfs initramfscfg           \
                        interactiveterminal license luksbootkeyfile            \
                        luksopenswaphookcfg plymouthcfg removeuser webview" ..
}

package() {
  cd ${srcdir}/calamares/build
  make DESTDIR="$pkgdir" install

#  install -Dm644 "branding/parabola/squid.png" "$pkgdir/usr/share/icons/hicolor/scalable/apps/calamares.svg"
#  install -Dm644 "../data/calamares.desktop"   "$pkgdir/usr/share/applications/calamares.desktop"
}
