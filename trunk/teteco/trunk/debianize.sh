#!/bin/sh
# This will copy all the needed files under debian/ to be debianized.
# And then builds the package.
#

prefix="/usr"
version="1"
revision="2"

ARCH=`uname -r | awk -F "-" '{print $3}'`
if [ "$ARCH" != "amd64" ]
then
        ARCH=i386
fi

if [ -x teteco ]; then
    rm debian/$prefix -rf > /dev/null 2> /dev/null
    mkdir debian/DEBIAN -p
    mkdir debian/$prefix/bin -p
    cp teteco debian/$prefix/bin/teteco
    cp -R share/ debian/$prefix

else
    echo "You need to compile before debianizing."
    exit 1
fi

echo "Package: teteco" > control
echo "Depends: libteteco (>= 1.2), libqtcore4, libqtgui4, libqt4-svg, libqt4-xml, libqwt5-qt4, libpoppler-qt4-3" >> control
echo "Version: $version.$revision" >> control
echo "Section: base" >> control
echo "Priority: optional" >> control
echo "Architecture: $ARCH" >> control
echo "Maintainer: Ignacio Martín Oya <nachooya@gmail.com>" >> control
echo "Description: TEDECO TeleConference " >> control
echo "Homepage: http://code.google.com/p/teteco" >> control

mv control debian/DEBIAN/control

# CREATE the packet
PKGNAME=teteco-$version.$revision"_"$ARCH
echo $PKGNAME
mkdir $PKGNAME
cp debian/* $PKGNAME/. -R
find $PKGNAME -name ".svn" | xargs rm -rf
find $PKGNAME -name "*~" | xargs rm 2> /dev/null
dpkg-deb -b $PKGNAME
rm -rf $PKGNAME
rm -rf debian

exit 0
