# LINUX BUILDING (Ubuntu - Debian) #
## LIBRARY ##
  * Edit your /etc/apt/sources.list and add
```
deb http://ppa.launchpad.net/kklimonda/libevent2/ubuntu natty main 
deb-src http://ppa.launchpad.net/kklimonda/libevent2/ubuntu natty main 
```
  * sudo apt-get update
  * sudo apt-get install libevent-dev portaudio19-dev libspeex-dev libspeexdsp-dev

  * Download libteteco-1.2.tar.gz
  * tar xzvf libteteco-1.2.tar.gz
  * cd libteteco-1.2
  * make
  * make deb
  * sudo dpkg -i libteteco-1.2\_i386.deb
## APPLICATION ##
  * sudo apt-get install g++ libqt4-dev libpoppler-qt4-dev libqwt5-qt4-dev

  * Download teteco-1.2.tar.gz
  * tar xzvf teteco-1.2.tar.gz
  * cd teteco-1.2
  * qmake teteco.linux.pro
  * make
  * ./debianize.sh
  * sudo dpkg -i teteco-1.2\_i386.deb