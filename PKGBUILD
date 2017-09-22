# Maintainer: bill-auger <bill-auger@programmer.net>


pkgname=calamares
pkgver=3.1.0
pkgrel=2
pkgdesc='Distribution-independent installer framework - Parabola edition'
arch=('i686' 'x86_64')
license=('GPL')
url="https://calamares.io/"

depends=('boost-libs' 'dmidecode' 'gptfdisk' 'hwinfo' 'kconfig' 'kcoreaddons' 'ki18n'
         'kparts' 'kpmcore' 'polkit-qt5' 'python' 'squashfs-tools' 'solid' 'qt5ct'
         'qt5-styleplugins' 'qt5-svg' 'yaml-cpp')
makedepends=('boost' 'extra-cmake-modules' 'git' 'qt5-tools')

_git_tag="${pkgver}-parabola-alpha${pkgrel}"
source=("https://notabug.org/bill-auger/calamares/archive/v${_git_tag}.tar.gz")
# sha256sums=('6ae55f567d5eafdb781dc84988dec184637fa7cf8bd55d7a2157b2d920880d52') # alpha1
sha256sums=('a118ad28e7fd9b9b9603a7b79374c7e7d5c6ca9d41d6ba06137db94cdce05e5c') # alpha2


build() {
  mkdir -p ${srcdir}/calamares/build
  cd ${srcdir}/calamares/build
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
}
