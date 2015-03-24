# PROJECT DESIGN #

## Introduction ##

Additionally to the project goals, the design was driven by following aims:

  * Must be a multi-platform solution at lest must run on Microsoft Windows, (hereinafter Windows) and GNU/Linux (hereinafter Linux). This way the application can be used in the most widespread operating system and the most successfully free operating system.

  * Must be free software (also open source) and based on free solutions, so it can be used, improved and extended in future scenarios.

  * Independence between logic and user interface, to provide a easy way to port it into new platforms.

To fulfill the last requirement, the project was divided into two parts: a library sustaining the logic, and the user interface.

Other main part of the design stage was the decision of the protocol(s) to be used for the data transmission.

The decision was to create a new protocol which adapts precisely to the
project goals. Of course existing protocols could be used, but in order of having margin to make some design decisions, these were discarded.

The protocol description can be consulted in the dedicated Wiki page.

## Library design ##

The library, named `libteteco`, provides a easy way to use all the functionality it is intended for:

  * Network management: which consists of name resolution DNS (symbolic to IP addresses translation), network communication procedures, including connection handling, data transmission and protocol implementation.

  * Audio management: procedures to select audio devices and capture and play sound from devices.

  * Audio compression: audio streams as they are used by sound devices, usually, are not suitable for network transmission, because of the size of the streams. There exists algorithms which can reduce the audio size significantly.
In particular speech-oriented and lossy algorithms are the most convenient for this application. These algorithms take advantage of the peculiarities of human-voice sound (for being speech oriented) and other psychoacoustic facts (for being lossy) to reduce the size of the sound data.


## User interface design ##

The user interface provides a way for users to use the \texttt{libteteco}
functionality
such as:

  * Configuration management: device selection, network configuration, etc.
  * Chat interface: to write and read text messages.

Additionally the implemented user interface include two more features:

  * Viewer: the integrated viewer can show the user sent/received files containing images or PDF documents which are sent within a session. Taken advantage of the application extensions of the protocol it also provides a way to remotely select the file to view or select the page within a PDF document.

  * Network graphs: More intended for developers, the UI can show real-time statistics of the traffic interchanged between peers.

No more functionalities were required for the intended objectives.