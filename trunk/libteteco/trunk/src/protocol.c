/**
 * @file    protocol.c
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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "util.h"
#include "protocol.h"

protocol_status_t protocol_status;

void protocol_initialize (void) {

    protocol_status.sending.voice.seq = 0;
    protocol_status.receiving.voice.seq = 0;
    protocol_status.receiving.voice.num = 0;

}


protocol_t protocol_parse_datagram (char* datagram, int datagram_len) {

    protocol_t protocol;

    protocol.status     = 0;

    protocol.voice.has = 0;
    protocol.voice.seq  = 0;
    protocol.voice.size = 0;

    protocol.voice_ack.has = 0;
    protocol.voice_ack.last = 0;
    protocol.voice_ack.number_received = 0;

    protocol.control.has = 0;
    protocol.control.type = 0;

    protocol.chat.has = 0;
    protocol.chat.seq  = 0;
    protocol.chat.size = 0;

    protocol.chat_ack.has = 0;
    protocol.chat_ack.seq = 0;

    protocol.file.has = 0;
    protocol.file.type = 0; 
    protocol.file.port = 0;

    unsigned int cursor = 0;

    PRO_TYPE_TYPE protocol_type = 0;

    if (datagram == NULL || datagram_len == 0) {
        return protocol;
    }

    // Get Type of message
    memcpy (&protocol_type, &datagram[cursor], PRO_TYPE_SIZE);
    cursor+=PRO_TYPE_SIZE;

    if (PRO_HAS_VOICE(protocol_type)) {

        protocol.voice.has = 1;

        // Get voice seq number
        memcpy (&protocol.voice.seq, &datagram[cursor], PRO_VOICE_SEQ_SIZE);
        cursor+=PRO_VOICE_SEQ_SIZE;

        protocol_status.receiving.voice.seq = protocol.voice.seq;
        protocol_status.receiving.voice.num++;

        // Get voice size
        memcpy (&protocol.voice.size, &datagram[cursor], PRO_VOICE_SIZE_SIZE);
        cursor+=PRO_VOICE_SIZE_SIZE;

        // Get voice
        memcpy (protocol.voice.payload, &datagram[cursor], protocol.voice.size);
        cursor+=protocol.voice.size;
    }

    if (PRO_HAS_VOICE_ACK(protocol_type)) {

        protocol.voice_ack.has = 1;

        //Get last received
        memcpy (&protocol.voice_ack.last, &datagram[cursor], PRO_VOICE_ACK_LAST_SIZE);
        cursor+=PRO_VOICE_ACK_LAST_SIZE;

        //Get number received
        memcpy (&protocol.voice_ack.number_received, &datagram[cursor], PRO_VOICE_ACK_NUM_SIZE);
        cursor+=PRO_VOICE_ACK_NUM_SIZE;

    }

    if (PRO_HAS_CONTROL(protocol_type)) {

        protocol.control.has = 1;
        // Get control type
        memcpy (&protocol.control.type, &datagram[cursor], PRO_CONTROL_TYPE_SIZE);
        cursor+=PRO_CONTROL_TYPE_SIZE;
    }

    if (PRO_HAS_CHAT(protocol_type)) {

        protocol.chat.has = 1;

        // Get chat seq number
        memcpy (&protocol.chat.seq, &datagram[cursor], PRO_CHAT_SEQ_SIZE);
        cursor+=PRO_CHAT_SEQ_SIZE;

        // Get chat size
        memcpy (&protocol.chat.size, &datagram[cursor], PRO_CHAT_SIZE_SIZE);
        cursor+=PRO_CHAT_SIZE_SIZE;

        // Get voice
        memcpy (protocol.chat.payload, &datagram[cursor], protocol.chat.size);
        cursor+=protocol.chat.size;
    }

    if (PRO_HAS_CHAT_ACK(protocol_type)) {

        protocol.chat_ack.has = 1;

        // Get chat seq number
        memcpy (&protocol.chat_ack.seq, &datagram[cursor], PRO_CHAT_ACK_SEQ_SIZE);
        cursor+=PRO_CHAT_SEQ_SIZE;
    }

    if (PRO_HAS_FILE(protocol_type)) {
        protocol.file.has = 1;
        // Get file mode
        memcpy (&protocol.file.type, &datagram[cursor], PRO_FILE_TYPE_SIZE);
        cursor+=PRO_FILE_TYPE_SIZE;
        // Get port
        memcpy (&protocol.file.port, &datagram[cursor], PRO_FILE_PORT_TYPE_SIZE);
        cursor+=PRO_FILE_PORT_TYPE_SIZE;
    }

    //log_print ("cursor: %u datagram_len: %u\n", cursor, datagram_len);
    protocol.status = 1;

    return protocol;

}


int protocol_build_datagram (protocol_t protocol, char** datagram, unsigned int *datagram_len) {

    unsigned int cursor = PRO_TYPE_SIZE;
    PRO_TYPE_TYPE protocol_type = 0x0;

    if (*datagram != NULL) {
        log_print ("[protocol]: datagram must be null!\n");
        return 0;
    }

    // Calcule datagram size

    *datagram_len = PRO_TYPE_SIZE +
                    (protocol.voice.has ? PRO_VOICE_SEQ_SIZE + PRO_VOICE_SIZE_SIZE + protocol.voice.size : 0) +
                    (protocol.voice_ack.has ? PRO_VOICE_ACK_LAST_SIZE + PRO_VOICE_ACK_NUM_SIZE : 0) +
                    (protocol.control.has ? PRO_CONTROL_TYPE_SIZE : 0) +
                    (protocol.chat.has ? PRO_CHAT_SEQ_SIZE + PRO_CHAT_SIZE_SIZE + protocol.chat.size: 0) +
                    (protocol.chat_ack.has ? PRO_CHAT_ACK_SEQ_SIZE : 0) +
                    (protocol.file.has ? PRO_FILE_TYPE_SIZE + PRO_FILE_PORT_TYPE_SIZE : 0);


    if (*datagram_len == 0) return 0;

    *datagram = util_calloc (1, *datagram_len);

    if (protocol.voice.has) {

        protocol_type += PRO_TYPE_VOICE;

        memcpy (*datagram+cursor, &protocol_status.sending.voice.seq, PRO_VOICE_SEQ_SIZE);
        protocol_status.sending.voice.seq++;
        cursor += PRO_VOICE_SEQ_SIZE;

        memcpy (*datagram+cursor, &protocol.voice.size, PRO_VOICE_SIZE_SIZE);
        cursor += PRO_VOICE_SIZE_SIZE;

        memcpy (*datagram+cursor, protocol.voice.payload, protocol.voice.size);
        cursor += protocol.voice.size;

    }

    if (protocol.voice_ack.has) {

        protocol_type += PRO_TYPE_VOICE_ACK;

        memcpy (*datagram+cursor, &protocol.voice_ack.last, PRO_VOICE_ACK_LAST_SIZE);
        cursor += PRO_VOICE_ACK_LAST_SIZE;

        memcpy (*datagram+cursor, &protocol.voice_ack.number_received, PRO_VOICE_ACK_NUM_SIZE);
        cursor += PRO_VOICE_ACK_NUM_SIZE;

    }

    if (protocol.control.has) {

        protocol_type += PRO_TYPE_CONTROL;

        memcpy (*datagram+cursor, &protocol.control.type, PRO_CONTROL_TYPE_SIZE);
        cursor += PRO_CONTROL_TYPE_SIZE;

    }

    if (protocol.chat.has) {

        protocol_type += PRO_TYPE_CHAT;

        memcpy (*datagram+cursor, &protocol.chat.seq, PRO_CHAT_SEQ_SIZE);
        cursor += PRO_CHAT_SEQ_SIZE;

        memcpy (*datagram+cursor, &protocol.chat.size, PRO_CHAT_SIZE_SIZE);
        cursor += PRO_CHAT_SIZE_SIZE;

        memcpy (*datagram+cursor, protocol.chat.payload, protocol.chat.size);
        cursor += protocol.chat.size;

    }

    if (protocol.chat_ack.has) {

        protocol_type += PRO_TYPE_CHAT_ACK;

        memcpy (*datagram+cursor, &protocol.chat_ack.seq, PRO_CHAT_ACK_SEQ_SIZE);
        cursor += PRO_CHAT_ACK_SEQ_SIZE;

    }

    if (protocol.file.has) {

        protocol_type += PRO_TYPE_FILE;

        memcpy (*datagram+cursor, &protocol.file.type, PRO_FILE_TYPE_SIZE);
        cursor += PRO_FILE_TYPE_SIZE;

        memcpy (*datagram+cursor, &protocol.file.port, PRO_FILE_PORT_TYPE_SIZE);
        cursor += PRO_FILE_PORT_TYPE_SIZE;

    }


    memcpy (*datagram, &protocol_type, PRO_TYPE_SIZE);

    return 1;

}


void protocol_print (protocol_t protocol) {

    log_print ("protocol.status = %u\n", protocol.status);
    log_print ("protocol.voice.has = %u\n", protocol.voice.has);
    log_print ("protocol.voice.seq = %u\n", protocol.voice.seq);
    log_print ("protocol.voice.size = %u\n", protocol.voice.size);
    log_print ("protocol.voice.payload = %c\n", protocol.voice.payload[0]);
    log_print ("protocol.voice_ack.has = %u\n", protocol.voice_ack.has);
    log_print ("protocol.voice_ack.last = %u\n", protocol.voice_ack.last);
    log_print ("protocol.voice_ack.number_received = %u\n", protocol.voice_ack.number_received);
    log_print ("protocol.control.has = %u\n", protocol.control.has);
    log_print ("protocol.control.type = 0x%02X\n", protocol.control.type);
    log_print ("protocol.chat.has = %u\n", protocol.chat.has);
    log_print ("protocol.chat.seq = %u\n", protocol.chat.seq);
    log_print ("protocol.chat.size = %u\n", protocol.chat.size);
    log_print ("protocol.chat.payload = %.*s\n", protocol.chat.size, protocol.chat.payload);
    log_print ("protocol.chat_ack.has = %u\n", protocol.chat_ack.has);
    log_print ("protocol.chat_ack.seq = %u\n", protocol.chat_ack.seq);
    log_print ("protocol.file.has = %u\n", protocol.file.has);
    log_print ("protocol.file.type = %u\n", protocol.file.type);
    log_print ("protocol.file.port = %u\n", protocol.file.port);

}