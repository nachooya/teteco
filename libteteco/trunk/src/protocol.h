/**
 * @file    protocol.h
 * @Author  Ignacio Martín Oya (nachooya@gmail.com)
 * @version 1.0
 * @date    October, 2010
 * @section LICENSE
 *
 *  Copyright (C) 2010  Ignacio Martin Oya
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>

#define PRO_TYPE_TYPE               uint8_t
#define PRO_TYPE_SIZE               sizeof(PRO_TYPE_TYPE)

#define PRO_TYPE_VOICE              0x01
#define PRO_HAS_VOICE(a)           a&PRO_TYPE_VOICE

#define PRO_TYPE_VOICE_ACK          0x02
#define PRO_HAS_VOICE_ACK(a)       a&PRO_TYPE_VOICE_ACK

#define PRO_TYPE_CONTROL            0x04
#define PRO_HAS_CONTROL(a)          a&PRO_TYPE_CONTROL

#define PRO_TYPE_CHAT               0x08
#define PRO_HAS_CHAT(a)             a&PRO_TYPE_CHAT

#define PRO_TYPE_CHAT_ACK           0x10
#define PRO_HAS_CHAT_ACK(a)         a&PRO_TYPE_CHAT_ACK

#define PRO_TYPE_FILE               0x20
#define PRO_HAS_FILE(a)             a&PRO_TYPE_FILE

#define PRO_CONTROL_TYPE_TYPE         uint8_t
#define PRO_CONTROL_TYPE_SIZE         sizeof(PRO_CONTROL_TYPE_TYPE)

#define PRO_CONTROL_HELO_SENDER         0x01
#define PRO_CONTROL_IS_HELO_SENDER(a)   a&PRO_CONTROL_HELO_SENDER
#define PRO_CONTROL_HELO_RECEIVER       0x02
#define PRO_CONTROL_IS_HELO_RECEIVER(a) a&PRO_CONTROL_HELO_RECEIVER

#define PRO_CONTROL_BYE               0x04
#define PRO_CONTROL_IS_BYE(a)         a&PRO_CONTROL_BYE

#define PRO_VOICE_SEQ_TYPE          uint16_t
#define PRO_VOICE_SEQ_SIZE          sizeof(PRO_VOICE_SEQ_TYPE)
#define PRO_VOICE_SIZE_TYPE         uint16_t
#define PRO_VOICE_SIZE_SIZE         sizeof(PRO_VOICE_SIZE_TYPE)
#define PRO_VOICE_MAX_PAYLOAD_SIZE  50000

#define PRO_VOICE_ACK_LAST_TYPE     uint32_t
#define PRO_VOICE_ACK_LAST_SIZE     sizeof(PRO_VOICE_ACK_LAST_TYPE)
#define PRO_VOICE_ACK_NUM_TYPE      uint32_t
#define PRO_VOICE_ACK_NUM_SIZE      sizeof(PRO_VOICE_ACK_NUM_TYPE)

#define PRO_CHAT_SEQ_TYPE          uint16_t
#define PRO_CHAT_SEQ_SIZE          sizeof(PRO_CHAT_SEQ_TYPE)
#define PRO_CHAT_SIZE_TYPE         uint8_t
#define PRO_CHAT_SIZE_SIZE         sizeof(PRO_CHAT_SIZE_TYPE)
#define PRO_CHAT_MAX_PAYLOAD_SIZE  100

#define PRO_CHAT_ACK_SEQ_TYPE      uint16_t
#define PRO_CHAT_ACK_SEQ_SIZE      sizeof(PRO_CHAT_ACK_SEQ_TYPE)

#define PRO_FILE_TYPE               uint8_t
#define PRO_FILE_TYPE_SIZE          sizeof(PRO_FILE_TYPE)
#define PRO_FILE_TYPE_SEND          0x00
#define PRO_FILE_TYPE_RECV          0x01
#define PRO_FILE_PORT_TYPE          uint16_t
#define PRO_FILE_PORT_TYPE_SIZE     sizeof (PRO_FILE_PORT_TYPE)

typedef struct {

    uint8_t status;

    struct {
        uint8_t                     has;
        PRO_VOICE_SEQ_TYPE          seq;
        PRO_VOICE_SIZE_TYPE         size;
        int8_t                      payload[PRO_VOICE_MAX_PAYLOAD_SIZE];
    } voice;

    struct {
        uint8_t                     has;
        PRO_VOICE_ACK_LAST_TYPE     last;
        PRO_VOICE_ACK_NUM_TYPE      number_received;
    } voice_ack;

    struct {
        uint8_t                     has;
        PRO_CONTROL_TYPE_TYPE       type;
    } control;

    struct {
        uint8_t                     has;
        PRO_CHAT_SEQ_TYPE           seq;
        PRO_CHAT_SIZE_TYPE          size;
        char                        payload[PRO_CHAT_MAX_PAYLOAD_SIZE];
    } chat;

    struct {
        uint8_t                     has;
        PRO_CHAT_ACK_SEQ_TYPE       seq;
    } chat_ack;

    struct {
        uint8_t                     has;
        PRO_FILE_TYPE               type;
        PRO_FILE_PORT_TYPE          port;
    } file;

} protocol_t;

typedef struct {

    struct {
        struct {
            PRO_VOICE_SEQ_TYPE seq;
        } voice;
    } sending;

    struct {
        struct {
            PRO_VOICE_ACK_LAST_TYPE     seq;
            PRO_VOICE_ACK_NUM_TYPE      num;
        } voice;
    } receiving;

} protocol_status_t;

#define protocol_init  {0,{0,0,0},{0,0,0},{0,0},{0,0,0},{0,0},{0,0,0}}

void       protocol_print          (protocol_t protocol);
protocol_t protocol_parse_datagram (char* datagram, int datagram_len);
int        protocol_build_datagram (protocol_t protocol, char** datagram, unsigned int *datagram_len);

#endif