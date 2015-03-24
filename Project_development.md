# PROJECT DEVELOPMENT #

## Implementation ##

This section describes, divided by functional features, how finally the application was implemented. It explains some  of the decisions taken, and also how the integration among all parts was done.

As described in the design section, the solution is divided in two parts, a library and the application (mainly user interface). Figure 2.8 shows
how the final solution is structured in base to the libraries used. Blue blocks correspond to the library part, green ones to the application:

![http://teteco.googlecode.com/img/layers_diagram.png](http://teteco.googlecode.com/img/layers_diagram.png)

Blocks on top of other block implies a usage relationship. The layers diagram does not include inherited dependencies (libraries used by other libraries) which has not been used directly in the application development.


### Building process implications ###

First of all a brief definition about what "building process" means in this case and why is important:

Usually applications are composed of several modules with different functionality, referred as libraries. At application build time, it can be chosen if libraries will be dynamically loaded by the application, or if they will be integrated with the application (executable file). In practical terms, this mean, among other things, the application will consists of one file (executable) or it will also require access to other files (dynamic libraries).

As one of the objectives of the solution is to be multi-platform, must be ensured all solutions adopted meet this requirement. Additionally it is desirable to know the targeted operating systems distribute applications in order to ease distribution and installation.
In this section is described how get the application for Linux and Windows based operation systems:

Most widely used Linux distributions use a package manager, which in case of be necessary, installs the required dependencies (this is, other packages) for an application to run, at install time. For this reason a dynamic build was chosen for the Linux application.

In case of Windows based operating systems, usually applications are distributed with all required dependencies (files). To avoid complicate application distribution (and other problems such as DLL hell a static build process has been chosen in this case, so just a file (executable) is needed for the application to work.

Other approaches are possibles, but those are not described here.

The exposed above is the cause of some of the procedures explained in the following sections. The compiling and building process are fully described on the appendices B, C, D, and E, which are copies of the files distributed within the source code package.

## Library ##

Library development language is C due its portability and the resources available for this programing language.

The public `libteteco` API can be consulted in appendix B.

The following sections explains how the library accomplish the objectives exposed on the design section.

### Audio management ###

Accessing programmatically to audio devices is usually performed by using a dedicated API (some times more than one, in example Linux can use ALSA, OSS, Pulse, ARTS, ESD and Windows exposes DirectSound or WMME among others) exposed by the operating system or sound services. There exists, however, libraries which provide an easier and normalized way to access these devices regardless the operating system and the underlaying API used.

For the library two of these, which met our requirements, were take into consideration: PortAudio and OpenAL.

Portaudio provides a normalized way to access sound devices and use them for capture and playback. It is a very thin layer on top of the different sounds API's. It is based on asynchronous events, which means the library decides when sound data must be processed.

OpenAL provides the same as PortAudio plus some other functionalities intended for sophisticated audio processing (such positional audio).

Both solutions were suitable for our purposes, but taking into account PortAudio is simpler and, presumably, has a less overhead in size and computational power, this was chosen.

### PortAudio usage considerations ###

PortAudio usage is fully documented, and examples can be found, in the official web site.

There exists some considerations that were took when using this in `libteteco`:

PortAudio provide functions to get all available sound devices. However although those provide information about each device, is a good idea to test a particular device with the parameters the application needs (such as the sample rate) to show the user only those devices will are really usable.

PortAudio needs initialize a device before using it. It is mandatory to initialize the device which parameters (sample rate, resolution, mono/stereo) which match, in this case, the codec parameters. In the case of `libteteco`, the codec library (libspeex) provide functions to get these parameters.

As mentioned before, PortAudio is a event based library. This means that is the library which calls a provide callback function each time it need to get/set audio data. It is very important that the callback function to perform his actions (usually copy the data) in the shortest time possible to avoid buffers overflow and consequently sound artifacts.

### Audio compression ###

For the audio compression `libteteco` uses the Speex codec.
This is a speech and lossy audio compression coded. It is free of patents and there exists a library providing a free implementation.

Speex codec has several parameters which defines how audio compression will be performed. The ones `libteteco` uses are:


  * Mode: there are three modes NarrowBand, WideBand and UltraWideBand. This describes the sampling rate used by the codec (8kHz, 16 kHz and 32kHz). `libteteco` can use the three at user choice.

  * Quality: a value between 0 and 10 which defines the computational power used to encode a frame. Higher values means more computational power and more sound fidelity. `libteteco` allows the user to choose the quality value.

  * VAD: the codec can detect when there is no voice in the sound signal. This way number of bytes to be transmitted when there is no voice is reduce drastically.

  * Perceptual enhancement: From Speex web site: "Perceptual enhancement is a part of the decoder which, when turned on, attempts to reduce the perception of the noise/distortion produced by the encoding/decoding process. In most cases, perceptual enhancement brings the sound further from the original objectively (e.g. considering only SNR}), but in the end it still sounds better (subjective improvement)".

The bitrate of the output signal depends mainly of Mode and Quality parameters, but VAD} can reduce punctually this bitrate. There exists tables with this information at Speex web site.

This Speex library also provides an audio preprocesor to be used before encoding the sound. `libteteco` uses this preprocesor for:

  * Noise suppression: the input signal is filtered so it get a better SNR.
  * Automatic Gain Control: this feature provides a way to adjust input sound signal to a reference value so different audio setups work optimally (ie: different microphone gains).

The Speex library also provide a jitter buffer implementation which is used by `libteteco`. A jitter buffer is necessary because during data transmission over the network some audio frames can get lost, arrive out of order and also to compensate PDV. Jitter buffer store sound frames based on a timestamp (also used as a sequence number) so the decoder receives a coherent sound stream.

There is not special considerations to take into account with the Speex library usage.

### Network management ###

Network management consists mainly of two functionalities communication and protocol implementation.

For the communication task `libteteco` supports on two API's:

  * The Berkeley sockets API, which is a ``de facto'' standard for interprocess communications in general and for IP} networks communications
in particular. This API is usually exposed directly by the operating system.

  * The libevent library. This library provides a multi-platform, event based interface to handle network communications plus time based events. Basically this library execute provided callback functions when an event occurs, such as data has been received or a timer has expired.

Regarding the Berkeley socket API, its usage among different operating systems is different, in particular between Linux and Windows those are the differences:

On Linux, the API is provided by the standard operating system libraries and usage is just a matter to include the necessary header
files as shown below:

```
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/wait.h>
```

On Windows based systems the API is exposed by the the operating system library called Winsock2. To use it the library, this must be initialized and de-initialized as shown below:

```
#include <winsock2.h>
#include <ws2tcpip.h>

int network_init () {
#ifdef __WINDOWS__
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        return 0;
    }
    return 1;
#endif
}

int network_deinit () {
#ifdef __WINDOWS__
    if (WSACleanup() != 0) {
        return 0;
    }
    return 1
#endif
}

```

There are other differences between Linux and Windows Berkeley socket usage:

One is that in Linux sockets can be used as file descriptors, so read and write functions are allowed to receive or send data, but Windows differences between sockets and file descriptors, so it is better to always use recv and send function on both systems.

Related to this, closing a socket in Windows must be performed by the closesocket function and in linux using the close function.

Finally some parameters on network functions differs in the data types, in example the function setsockopt. Below there are some macros that to solve the differences:

```
#ifdef __WINDOWS__
    char on = 1;
#else
// Always use closesocket to close a socket.
#define closesocket(a) close(a)
    int on  = 1;
#endif
setsockopt (teteco->sd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)
```


### TETECO protocol ###

The TETECO protocol has been implemented following the guidelines of the protocol design proposed in the design section. There are, however, some implementation options, which are described here:

This implementation only piggybacks voice, voice ack, chat and chat ack channels.

For voice sub-PDU three Speex frames are sent per PDU. An Speex frame always represents 20ms of sound. To calculate the frame size this formula is used:

VoiceFrameSize = (bitrate / (SampleRate/FrameSize)).

In example, in NarrowBand mode sample rate is 8000 (Hz), with quality mode set to 7 the bitrate is 15000 bps. The frame size for 20 msec is 160. So the encoded frame size size is (15000/(8000/160)) = 300 bits = 38 bytes.

Each frame also includes header with a timestamp which is 4 bytes long and the frame size which is 1 byte long.

So in this example each voice sub-PDU is (4+2)+3**[(4+1+38)] = 153 bytes long.**

As one Voice PDU is sent each 60 ms the bitrate for audio is 2550 Bps.

For the chat channel, the maximum text transmitted in one PDU is 100 bytes.

So following with the previous example, the maximum bitrate will be:

1 + (2+1+100) + 153 = 257 bytes each 60 ms which implies 4284 Bps.

![http://teteco.googlecode.com/svn/wiki/img/TETECO_Graph_BPS.png](http://teteco.googlecode.com/svn/wiki/img/TETECO_Graph_BPS.png)

The figure 2.9 shows a bandwidth usage graph with the parameters specified on the example during one minute. It noticeable bandwidth usage is not constant during time this is due to reasons:

  * The library is using VAD capabilities of the Speex codec, which prevent all frames to be sent.
  * In this case, only a chat PDU is sent every 5 seconds, which varies PDU size.

It is evident, also, bandwidth usage by audio receiver is very low, although it also send chat data every five seconds, which is one of the main objectives.

Regarding to file transmission, this has been implemented with an option to limit bandwidth usage for this connection, in order to prevent bandwidth flooding. This control is performed by an algorithm like the one implemented below:

```
static uint32_t chunk_size  = 10240;  // bytes
static uint64_t sleep_time  = 100000; // usec
static uint32_t sent_so_far = 0;      //bytes

while (!end) {
// Bandwidth control
    if (max_transfer_rate != 0) {

        if (transfer_rate > max_transfer_rate) {
            sleep_time = sleep_time * 1.02;
        }
        else if (transfer_rate < max_transfer_rate) {
            sleep_time = sleep_time * 0.98;
        }

        chunk_size = teteco->max_transfer_rate / 20;
        usleep (sleep_time);

    }
    else {
        chunk_size = 10240;
    }

    int read = fread (buffer, 1, chunk_size, teteco->fd);
    int written = 0;

    while (written < read) {
        written += send (sd_tcp, (void*)buffer, read, 0);
        if (written < 0 ) break;
    }
    sent_so_far += written;

    transfer_rate = sent_so_far / (sleep_time/1000000);

}

```


### libteteco ###
The TETECO library is event based this means the it executes a provide callback function each time some event occurs. The events fired are, caused by control messages, expired timers (disconnection) and chat reception.

There is a public API which is fully documented, which explains events in deep.

It is entirely written in C language, but offers a C++ interface. To accomplish this the public header file of the library includes the macros shown below:

```
#ifdef __cplusplus
extern "C" {
#endif

// Public API declaration

#ifdef __cplusplus
}
#endif
```

### Threading ###

As `libteteco` is event based, once it is initialized it runs it is own thread. To get threading functionality it uses the pthread library which is a POSIX standard for threads. As Linux is a POSIX-like system it provides this library by default. In Windows operating systems this library is not included, so `libteteco` uses a implementation of this standard for Windows called pthreads-win32.

The usage of this library under Windows systems require some additional steps, moreover in the case we want a static compilation. In particular the library must be initialized before its usage and de-initialized at termination as shown below:

```
void threads_init () {
#ifdef __WINDOWS__ && __STATIC__
    pthread_win32_process_attach_np();
}
#endif

void threads_deinit () {
#ifdef __WINDOWS__ && __STATIC__
    pthread_win32_process_detach_np();
#endif
}

```

## User Interface ##

The user interface is mainly programmed making use of the QT library, which is a multi-platform framework, intended, principally, for UI} programing.

There were another two libraries the TETECO application uses:

  * Poppler: is a library which allows to read and render PDF} documents. It also includes a QT binding which makes it easy to integrate into QT based applications.
  * Qwt : is a graph generator (among other functionalities) library build in top of the QT library. TETECO applications uses it to show the network statistics.

TETECO application programing language is C++ which is the native programming language for the QT library.

### Integration with libteteco ###

The main difficulty to integrate asynchronous events based libraries, and in particular the `libteteco` library is how to synchronize the events provided by the library with the threading mechanism used by QT.

`libteteco` uses his own thread to run, and consequently raises the events from that thread. On the other hand, QT has his own threads and mechanism to communicate among them.

The solution taken for this problem was the creation of a "Proxy" object within the user interface. This is a QT based object with will provide the callbacks required by the `libteteco` library and will translate those events into the SIGNALS-SLOT mechanism used by QT to handling intra-threads communications.

An example of this solution is shown the listing below:

```
class Proxy : public QObject {

    Q_OBJECT
    public:
        Proxy() {};
        void ChatCallback (char* entry) {
            QString qentry(entry);
            emit Chat (qentry);
        }

        static Proxy* singleton (void) {
            return m_singleton;
        }

    signals:
        void Chat       (QString         qentry);

    private:
        static Proxy *m_singleton;

};

Proxy * Proxy::m_singleton = new Proxy();

void ChatCallBack (char* entry) {

    Proxy::singleton()->ChatCallback (entry);

}

int main () {

    teteco_set_chat_callback (&ChatCallBack);
}

```

### Viewer ###

To implement the viewer functionality TETECO makes use of the methods provided by QT to show images. However QT does not has any PDF handling capacity.
For this reason the user interface makes of of the Poppler library.

Thanks to the QT binding provided for Poppler, integration with QT is easy.

However get a static version of that library for Windows brings some complications due to the build script. Step by step instructions are provided in specific Wiki page.