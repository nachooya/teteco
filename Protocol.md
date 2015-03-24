# TETECO Protocol #

TETECO protocol is a binary, byte aligned protocol. Although it operates over a transport-layer protocol, it is inspired in the SCTP, in particular in the message-based, multi-streaming capacities.

It is designed to be transport-protocol-independent, which means it can work over any OSI Layer 4 protocol, but taking into account it carries real-time audio data it should be a non reliable protocol to prevent retransmissions.

Current implementation uses UDP as transport-protocol due its extended usage.

### PDU description ###

Only one PDU type of variable size is used. The PDU contains a fixed header and several channels or streams carrying different information as shown in figure below:
Each channel has it owns subPDU.

http://teteco.googlecode.com/svn/wiki/img/TETECO_PDU.pdf

The PDU header is one byte length, and contains a bitmap of the channels (or sub-PDU) the PDU is carrying. The bitmap is defined in table below:

Channel   | Dec | Hex   | Binary
VOICE     | 1   | 0x01  | 0000 0001
VOICE ACK | 2   | 0x02  | 0000 0010
CONTROL   | 4   | 0x04  | 0000 0100
CHAT      | 8   | 0x08  | 0000 1000
CHAT ACK  | 16  | 0x016 | 0001 0000

So a Teteco protocol PDU can carry from 1 up to 5 channels.

The maximum PDU size is theoretically 65646 bytes long, but actually this size is not possible because:

  * Never all sub-PDU are sent in one PDU due to the protocol logic.
  * The underlaying protocol probably does not allow it.
  * Makes no sense

Largest PDU will be those carrying sound data. It is due to the implementation to define the maximum sound data size per PDU, but the following considerations must be taken into account:

  * If using an unreliable protocol (which is desirable due to the nature of the data to be transmitted) such as UDP, IP fragmentation should be avoided. In current IP based networks this limit the practical PDU size up to 1300 bytes.
  * Smaller sizes are desirable because PDU lost are less noticeable for the sound transmitted, and the lag is also reduced. However small PDU's implies more PDU's and that more resources usage at computers and networks elements. Usually sending, approximately, between 50 and 200 bytes (of sound per PDU) is a good choice.

Each channel has its own behavior in terms of flood, acknowledging and timers, as explained below.

### Voice channel ###
Its sub-PDU consisting of three fields: sequence number, size and data. The sequence number is 4 bytes long, it is a monotonic increased counter for
each voice sub-PDU sent. Size is a 2 byte field which indicates the size of the data field in bytes from 0 up to 65536. Finally the data field is variable sized, from 0 to 65536, and carries the sound information:

http://teteco.googlecode.com/svn/wiki/img/TETECO_PDU_VOICE.pdf

Voice channel has no flow control, it uses the Voice ACK channel for acknowledgement but only for statistics and as keep-alive signal.

The receiver uses the voice channel as connection monitor, firing an event to inform no peer connection after 5 seconds without receiving voice data.

### Voice ack channel ###

Voice ACK is a sub-PDU containing feedback information from the sound receiver.
It is fixed in size and consists of two fields: the sequence number of the last received Voice sub-PDU, 4 byte long, and the total number of received Voice sub-PDU, 4 bytes long:

http://teteco.googlecode.com/svn/wiki/img/TETECO_PDU_VOICE_ACK.pdf

This channel has no flow control and is not acknowledged. It is used by sender to collect statistics and as connection monitor firing an event to inform no peer connection after 5 seconds.

### Control channel ###

Control channel carries different types of control messages. It is a fixed in size sub-PDU with 4 fields (see Figure 2.4). The first byte is the TYPE field and indicates the type of control message. The following fields are 4 bytes long and are the ARGUMENT fields:

http://teteco.googlecode.com/svn/wiki/img/TETECO_PDU_CONTROL.pdf

The type of control messages and values representing them are exposed in the table 2.2.

Type           | Value (hex)
HELO SENDER    | 0x01
HELO RECEIVER  | 0x02
BYE            | 0x03
FILE           | 0x04
APPLICATION    | 0x05
APPLICATIONACK | 0x06

The HELO messages are sent to establish a session. HELO SENDER indicates the other peer the intention to be the audio sender. HELO RECEIVER indicates the intention to be the audio receiver. A session is established when a HELO message in sent to confirm a HELO message reception. HELO messages interchanged must be of different type.

BYE type is sent when one peer wants to release the session, after a sent BYE the sender peer can stop listening to the other peer. It is confirmed with another BYE message.

The FILE type is used to establish a new connection over the TCP for file transmission. This control message has as ARGUMENT1 the mode: LISTEN (0x00) meaning the peer is listening on for a connect, or CONNECT (0x01) meaning the peer is going to perform the connection. The second argument carry the port the peer is going to use. FILE transmission session is established when a FILE message of different type is received in confirmation to a sent FILE message.

APPLICATION type is intended to be used by applications in top of the protocol. Currently there are two reserved values for the ARGUMENT1: SET FILE (0x01) and SET PAGE (0x02).

The APPLICATION ACK type is used to confirm the reception of a CONTROL APPLICATION sub-PDU.

This channel performs flow control in an stop-and-wait way, a new message is sent only after the previous has been acknowledged. Each message is acknowledged by a same message type.

### Chat channel ###

Chat channel is intended to transmit short text messages. It is build of 4 fields: SEQUENCE a 2 byte length, it is a monotonic increased counter of the CHAT sub-PDU sent. SIZE, 1 byte length which indicates the DATA field length, from 0 up to 256 bytes. And the DATA FIELD which carries from 0 up to 256 characters.
Implementation can limit the DATA length to any required value (100 bytes is suggested).

http://teteco.googlecode.com/svn/wiki/img/TETECO_PDU_CHAT.pdf

This channel in conjunction with the CHAT ACK channel performs flow control in an stop-and-wait fashion, a new message is sent only after the previous has been acknowledged.

### Chat ack channel ###

Chat Ack channel is a simple sub-PDU carrying only a 2 bytes field with the sequence number of the last CHAT sub-PDU received.

http://teteco.googlecode.com/svn/wiki/img/TETECO_PDU_CHAT_ACK.pdf

### File transfer ###
TETECO protocol provides a mechanism for file transmission. It is not based in the channels schema due to efficiency. Although would be possible to implement file transmission using that schema it will add lot of complexity and probably will under-perform the solution used.

File transmission is done over a TCP} connection. The TCP} connection is established thanks to TETECO control messages. Connection negotiation is pretty simple. The peer which wants to send a file sends a CONTROL FILE with the first argument indicating if it expects get connected or it will connect, and the second argument indicates the port to be used. At reception the other peer sends a CONTROL FILE message as acknowledgement with the first argument with a complementary option to the received and the second argument contains the port to be used. At this time the peer which is going to perform the connection does it.

Data transmitted over this connection is always from file sender to receiver. First data sent is a 4 byte number with the file size, then a 2 bytes long number with the file name size, then the file name string and finally the file payload. Connection is closed after all data has been sent.

Figure below shows an exemplary traffic diagram for a TETECO session. It starts by sending the HELO control messages to establish the session. Then there are some PDU's interchange carrying voice and CHAT subPDU's, a file transfer negotiation ans some application specific signaling. Finally the session is released with the BYE messages.

http://teteco.googlecode.com/svn/wiki/img/TETECO_exemplary_session.pdf