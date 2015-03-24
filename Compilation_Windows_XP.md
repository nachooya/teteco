# Compilation in Windows XP #

### MinGW32 ###

> Use this vesrion of MinGW so Qt doesn't complain:
> http://get.qt.nokia.com/misc/MinGW-gcc440_1.zip
> I decompressed it at c:/MinGW

> If you are as me and feel more comfortable with Unix like shells install MSYS
> http://downloads.sourceforge.net/mingw/MSYS-1.0.11.exe
> I installed it at C:\msys\1.0

  * Make a build directory. For example c:/teteco

## LIBRARY ##

### pthreads-w32 ###
  * Download [ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.tar.gz](ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.tar.gz)
  * tar xzvf pthreads-w32-2-8-0-release.tar.gz
  * cd pthreads-w32-2-8-0-release.tar.gz
  * make clean GC-static
  * cp pthread.h /c/teteco/include/
  * cp libpthreadGC2.a /c/teteco/lib

### libevent ###
  * Download http://monkey.org/~provos/libevent/
  * http://monkey.org/~provos/libevent-2.0.10-stable.tar.gz
  * extract it at build directory.
  * tar xzvf libevent-2.0.10-stable.tar.gz
  * cd libevent-2.0.10-stable
  * ./configure --prefix=/c/teteco/ --disable-openssl --disable-malloc-replacement
  * make
  * make install

### portaudio ###
  * Download http://www.portaudio.com/archives/pa_stable_candidate_v19_20110317.tgz
  * extract it at build directory
  * tar xzvf pa\_stable\_candidate\_v19\_20110317.tgz.tar.gz
  * cd portaudio
  * ./configure --prefix=/c/teteco/ --enable-static=yes --enable-shared=no
  * make
  * make install

### Speex ###
  * Download http://downloads.xiph.org/releases/speex/speex-1.2rc1.tar.gz
  * tar xzvf speex-1.2rc1.tar.gz
  * cd speex-1.2rc1
  * ./configure --prefix=/c/teteco
  * make
  * make install

### libteteco ###
  * Download
  * tar xzvf libteteco-1.2.tar.gz
  * cd libteteco-1.2
  * make -f Makefile.win PREFIX=/c/teteco/
  * make -f Makefile.win PREFIX=/c/teteco/ install

## APPLICATION ##

### Qt ###

  * Download Qt libraries from http://qt.nokia.com/downloads/windows-cpp
  * I got qt-win-opensource-4.7.2-mingw.exe
  * Install it. I did on default directory C:\Qt\4.7.2
  * Open MSYS shell and
  * Edit /c/Qt/4.2.7/mkspecs/mkspecs/win32-g++/qmake.conf: add "-static" to QMAKE\_LFLAGS
  * Go to installation directory and execute (to get static libraries):
  * ./configure.exe -static -no-exceptions -nomake examples -nomake demos -fast -opensource -release -no-accessibility -no-sql-sqlite -no-qt3support -no-opengl -qt-zlib -qt-gif -qt-libpng -qt-libmng -qt-libtiff -qt-libjpeg -no-multimedia -no-webkit -qt-style-windows -qt-style-windowsxp -qt-style-windowsvista
  * make

### Qwt ###

  * Download from http://downloads.sourceforge.net/project/qwt/qwt/5.2.1/qwt-5.2.1.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fqwt%2Ffiles%2Fqwt%2F5.2.1%2F&ts=1301161041&use_mirror=switch
  * extract it
  * tar zjvf qwt-5.2.1.tar.bz2
  * cd qwt-5.2.1
  * edit qwtconfig.pri

<table border='1'>
<tr><td><b>FROM</b></td><td><b>TO</b></td></tr>
<tr>
<td><pre><code><br>
CONFIG           += release     # release/debug/debug_and_release<br>
#CONFIG           += debug_and_release<br>
#CONFIG           += build_all<br>
</code></pre></td>
<td><pre><code><br>
#CONFIG           += release     # release/debug/debug_and_release<br>
CONFIG           += debug_and_release<br>
CONFIG           += build_all<br>
</code></pre></td>
</tr>
<tr>
<td><pre><code><br>
CONFIG           += QwtDll<br>
</code></pre></td>
<td><pre><code><br>
#CONFIG           += QwtDll<br>
</code></pre></td>
</tr>
<tr>
<td><pre><code><br>
win32 {<br>
INSTALLBASE    = c:\qwt-5.2.1<br>
}<br>
</code></pre></td>
<td><pre><code><br>
win32 {<br>
INSTALLBASE    = /c/teteco<br>
}<br>
</code></pre></td>
</tr>
</table>
  * qmake (if necessary export PATH=$PATH:/c/Qt/4.7.2/bin)
  * make

### freetype ###

  * Download from http://download.savannah.gnu.org/releases/freetype/freetype-2.4.4.tar.gz
  * tar xzvf freetype-2.4.4.tar.gz
  * cd freetype-2.4.4
  * ./configure prefix=/c/teteco
  * make
  * make install

### poppler ###

  * Download from http://poppler.freedesktop.org/poppler-0.16.3.tar.gz
  * tar xzvf poppler-0.16.3.tar.gz
  * cd poppler-0.16.3 (if necessary export PATH=$PATH:/c/Qt/4.7.2/bin)
  * due to a bug in mingw-4.4.0 you have to edit the file
> /c/mingw/lib/gcc/mingw32/4.4.0/libstdc++.dll.a

<table border='1'>
<tr><td><b>FROM</b></td><td><b>TO</b></td></tr>
<tr>
<td>
<pre><code>library_names='libstdc++.a.dll'</code></pre>
</td>
<td>
<pre><code>library_names='libstdc++.a'</code></pre>
</td>
</tr>
</table>
  * POPPLER\_QT4\_CFLAGS=-I/c/Qt/4.7.2/include/ \
> POPPLER\_QT4\_CXXFLAGS=-I/c/Qt/4.7.2/include/ \
> POPPLER\_QT4\_LIBS='-L/c/Qt/4.7.2/lib/ -lQtCore4 -lQtGui4 -lQtXml4' \
> POPPLER\_QT4\_TEST\_CFLAGS=-I/c/Qt/4.7.2/include/ \
> POPPLER\_QT4\_TEST\_LIBS='-L/c/Qt/4.7.2/lib/ -lQtCore4 -lQtGui4 -lQtXml4' \
> FREETYPE\_CONFIG=/c/teteco/bin/freetype-config \
> CFLAGS='-ULIB\_IMPORT -UPOPPLER\_QT4\_EXPORT' \
> CPPFLAGS='-ULIB\_IMPORT -UPOPPLER\_QT4\_EXPORT' \
> ./configure --prefix=/c/teteco --enable-poppler-qt4 --disable-utils

  * make
  * make install

### teteco ###
  * Download
  * PREFIX=/c/teteco/ qmake teteco.win.pro
  * make