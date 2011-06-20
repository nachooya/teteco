#!/bin/sh
# This will copy all the needed files under debian/ to be debianized.
# And then builds the package.
#

set -x

prefix="/usr"
version="1"
revision="0"

ARCH=`uname -r | awk -F "-" '{print $3}'`
if [ "$ARCH" != "amd64" ]
then
        ARCH=i386
fi

if [ -x "src/libteteco.so."$version"."$revision ]; then
    rm debian/$prefix -rf > /dev/null 2> /dev/null
    mkdir debian/DEBIAN -p
    mkdir debian/$prefix/lib -p
    mkdir debian/$prefix/include -p
    cp src/libteteco.so.$version.$revision debian/$prefix/lib
    OLD_PWD=`pwd`
    cd debian/$prefix/lib
    ln -sf libteteco.so.$version.$revision libteteco.so.$version
    ln -sf libteteco.so.$version.$revision libteteco.so
    cd $OLD_PWD
    cp include/teteco.h debian/$prefix/include

else
    echo "You need to compile before debianizing."
    exit 1
fi

echo "Package: libteteco" > control
echo "Depends: libportaudio2, libspeex1, libspeexdsp1, libevent-2.0-5" >> control
echo "Version: $version.$revision" >> control
echo "Section: base" >> control
echo "Priority: optional" >> control
echo "Architecture: $ARCH" >> control
echo "Maintainer: Ignacio Martín Oya <nachooya@gmail.com>" >> control
echo "Description: TEDECO TEleCOnference library" >> control
echo "Homepage: http://code.google.com/p/teteco" >> control

mv control debian/DEBIAN/control

# CREATE the packet
PKGNAME=libteteco-$version.$revision"_"$ARCH
echo $PKGNAME
mkdir $PKGNAME
cp debian/* $PKGNAME/. -R
find $PKGNAME -name ".svn" | xargs rm -rf
find $PKGNAME -name "*~" | xargs rm 2> /dev/null
dpkg-deb -b $PKGNAME
rm -rf $PKGNAME
rm -rf debian

exit 0
