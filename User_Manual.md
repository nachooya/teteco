# USER MANUAL #
## Installation ##
### Linux (Debian - Ubuntu) ###
For the installation under Debian or Ubuntu (and in general any deb packaging based distribution) you need to get both libteteco-x.x.deb and teteco-x.x.deb packages.
Library libteteco package depends of the following packages:
```
libportaudio2, libspeex1, libspeexdsp1, libevent (>=2.0-5)
```
Teteco application requires:
```
libqtcore4, libqtgui4, libqt4-svg, libqt4-xml, libqwt5-qt4, libpoppler-qt4-3
```
As the libevent version supplied with Debian (Lenny) and Ubuntu (10.10) is older you have to add the following repository to you sources (/etc/apt/sources.list):
```
deb http://ppa.launchpad.net/kklimonda/libevent2/ubuntu natty main
deb-src http://ppa.launchpad.net/kklimonda/libevent2/ubuntu natty main
```
Then, in order to install the required packages, execute:
```
apt-get update
apt-get install libportaudio2 libspeex1 libspeexdsp1 libevent libqtcore4 libqtgui4 libqt4-svg libqt4-xml libqwt5-qt4 libpoppler-qt4-3
```
Finally install Teteco packages by issuing:
```
dpkg -i libteteco-x.x.deb teteco-x.x.deb.
```
Now you have find a direct access to the TETECO application under the Internet sub-menu of your desktop application launcher.
### Windows ###

Under Windows no installation is required. Just download the zip file with the application, decompress it on your desired location and make double-click to run it.

## Configuration ##

First step which should be accomplished before usage TETECO is configuration. Configuration is performed by selecting "Preferences" option in the menu bar of Teteco under the ``File'' menu. Usually default configuration will work, but special attention must me paid in sound device configuration.

![http://teteco.googlecode.com/svn/wwiki/img/TETECO_Configuration.png](http://teteco.googlecode.com/svn/wwiki/img/TETECO_Configuration.png)

  * Audio Devices Configuration

  * Device IN

If you have more than one Audio Capture Device, you can chose what use.
Usually you have to select the default device.
On Linux systems this is usually called ``[ALSA](ALSA.md) Default''.
On Windows ``Microsoft sound asigner''.

**Device OUT**

If you have more than one Audio Output Device, you can chose what use.
Usually you have to select the default device. On Linux systems this is usually called ```[ALSA] Default''. On Windows ```Microsoft sound asigner''.

  * Speex Configuration

  * Mode

You can choose among three Speex modes which defines the sample rate used:

  * NarrowBand 	(8  kHz)
  * WideBand		(16 kHz)
  * UtrawideBand	(32 kHz)

  * Quality

This parameter defines the bitrate and computational power used to encode the audio stream. See Speex Manual to know the different values.

  * Server Configuration

  * Listening port

This is the UDP port where TETECO will listen for incoming connections when in server mode.

  * Maximum transfer rate for files

For file transferring you can select a maximum bandwidth usage in order to prevent link flood. Value of 0 means no limit.

  * File Reception

You can select the folder where received files will be stored.

## Main Window ##

The TETECO main window shows a menu bar with the following menus:

  * File
  * Preferences: Opens the configuration windows.
  * Exit: Closes the application.
  * View
  * Viewer: Shows/hides the image/document viewer.
  * Statistics: Shows the statistics window.
  * Log: Shows/hides the log viewer.
  * Bookmarks
  * Add current: Create a new bookmark with the address in the address bar.
  * For every bookmark a item with the bookmark name is displayed with the following options
  * Connect: Connect to the remote peer stored in the bookmark
  * Delete: Delete the bookmark
  * Help
  * User Manual: Open the User Manual window.
  * About: Open the TETECO information window.

![http://teteco.googlecode.com/sv/wiki/img/TETECO_Initial.png](http://teteco.googlecode.com/sv/wiki/img/TETECO_Initial.png)

The main window also shows a button bar with two buttons:

  * Server Mode: To start TETECO server mode.
  * Send File: To send a file when connected and in server mode.

Below it's the address bar, to write the address to connect to and the connect button to connect to the TETECO server.
Down the address bar (if viewer is hidden) is the chat area, below the chat input area with the chat send button.
Finally at bottom of window it is the status bar, which show the TETECO status (DISCONNECTED, WAITING, CONNECTING, CONNECTED) and the audio level monitor.
When viewer is shown, it is between the address bar and the chat area. Viewer has two parts, the viewer itself and the file list.
When log viewer is displayed it is between the chat input area and the status bar.
You can resize the main window and distribute the space used by each area.

## Server mode ##

To active server mode press the ``Server Mode'' button. TETECO will wait for incoming connections on port selected in configuration window.

Upon connection establishment the remote address will show remote peer address and audio meter will show audio capture audio level which is being transmitted.

You receive and send chat messages by writing them and pressing send button.

Sensing of files is also available in this mode. Just press "Send File" button and select the file to send.
The ```viewer'' will appear and on the ```Send Files'' list you can observer the file transmission progress.

![http://teteco.googlecode.com/sv/wiki/img/TETECO_Server.png](http://teteco.googlecode.com/sv/wiki/img/TETECO_Server.png)

After a correct file transmission, if file sent is one of the supported format for the TETECO viewer, you will see the image or document in the left part of the viewer.

In case the sent file were a PDF file you can set the page to view on both peers, local and remote.

If more than one file have been transferred, you can set the file to see in both peers by selecting it on the file list.

## Client mode ##

Client mode is activated when you enter an address to connect to. You can specify an address by introducing a IP address or a fully qualified domain name.
Optionally you can also introduce the port number. If not specified the default one (22022) is used. Schema part of the URL (teteco://) is not mandatory.

Examples:

teteco://tedeco.fi.upm.es

127.0.0.1:12345

![http://teteco.googlecode.com/svn/wiki/img/TETECO_Client.png](http://teteco.googlecode.com/svn/wiki/img/TETECO_Client.png)

When in client mode you will hear audio emitted by the server. You can monitor audio level looking at the audio monitor. You will also receive files sent by the server.
Teteco will show you previews of supported files (images and PDF documents). The file previewed is selected by the server peer.
All received files are stored at the folder selected in the configuration window, so you can open them with external applications.

In client mode you can use the chat to provide feedback to the server operator.