# Maintainer: Your Name <youremail@example.com>
pkgname=sysinfoviewer
pkgver=0.1.0
pkgrel=1
pkgdesc="System Information Viewer"
arch=('x86_64')
url="https://github.com/Magpiny/sysinfoviewer" # Placeholder, update if needed
license=('MIT') # Placeholder, check LICENSE file for actual license
depends=('wxwidgets' 'curl' 'alsa-lib')
makedepends=('cmake' 'gcc' 'git')
source=("git+https://github.com/Magpiny/sysinfoviewer.git#tag=v${pkgver}") # Assuming tags are used for versions
sha256sums=('SKIP') # Use 'SKIP' for git sources

build() {
  cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build build
}

package() {
  cmake --install build --prefix "$pkgdir/usr"
  # Ensure desktop file and icon are installed correctly
  # These are handled by cmake --install based on CMakeLists.txt
}
