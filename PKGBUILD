pkgname=hydra-git
pkgver=0.0.0
pkgrel=1
pkgdesc="hydra - a simple tabbed webkit browser, aims to suckless, while still being a full featured browser for the modern desktop."
arch=('i686' 'x86_64')
url="http://github.com/dluco/hydra/"
license=('GPL' 'MIT/X')
depends=('gtk2' 'libwebkit' 'dmenu-vertical' 'moreutils')
makedepends=('git')
provides=('hydra')
conflicts=('hydra')

_gitroot=git://github.com/dluco/hydra.git
_gitname=hydra

build() {
  cd $srcdir

  msg "connecting to github's GIT server...."

  if [ -d $startdir/src/$_gitname ] ; then
    cd $_gitname && git-pull origin
    msg "The local files are updated."
  else
    git clone $_gitroot
  fi

  msg "GIT checkout done"

  cd "$srcdir/$_gitname"
  make || return 1
  make DESTDIR="$pkgdir" install || return 1

}
