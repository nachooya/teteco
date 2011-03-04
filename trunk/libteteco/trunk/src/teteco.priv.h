/**
 * @file    teteco.priv.h
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


#ifndef __TETECO_PRIV_H__
#define __TETECO_PRIV_H__

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include <event2/event.h>

#include "chat.h"
#include "audio.h"
#include "enc_speex.h"

#define MAX_DATAGRAM_SIZE 1300
#define CHAT_MAX_LENGTH 100

typedef enum {
    TETECO_SPEEX_WB = 0,
    TETECO_SPEEX_NB,
    TETECO_SPEEX_UWB
} teteco_speex_band_t;

typedef enum {
    LOG_DEBUG = 0
} log_levet_t;

typedef enum {
    TETECO_STATUS_DISCONNECTED = 0,
    TETECO_STATUS_CONNECTING,
    TETECO_STATUS_WAITING,
    TETECO_STATUS_CONNECTED,
    TETECO_STATUS_TIMEDOUT
} teteco_status_t;

typedef enum {
    TETECO_AUDIO_RECEIVER = 0,
    TETECO_AUDIO_SENDER,
    TETECO_AUDIO_UNDEFINED
} teteco_audio_mode_t;

typedef enum {
    TETECO_NET_SERVER = 0,
    TETECO_NET_CLIENT
} teteco_net_mode_t;

typedef enum {
    TETECO_FILE_TRANSFER_SENDING = 0,
    TETECO_FILE_TRANSFER_RECEIVING,
    TETECO_FILE_TRANSFER_END
} teteco_file_transfer_status_t;

typedef void(*chat_callback_ft)          (char*);
typedef void(*status_callback_ft)        (teteco_status_t);
typedef void(*file_transfer_callback_ft) (const char*, teteco_file_transfer_status_t, uint32_t, uint32_t);

typedef struct {

    float                   current_db;
    uint32_t                total_bytes_out;
    uint32_t                total_bytes_in;
    time_t                  time_start;
    teteco_status_t         status;
    uint32_t                packets_expected;
    uint32_t                packets_received;
    uint64_t                transfer_rate;

    // Protected

    uint16_t                remote_port;
    char*                   remote_address;
    teteco_speex_band_t     enc_speex_band;
    int                     enc_quality;
    int                     voice_ack_every;
    int                     audio_device_in;
    int                     audio_device_out;
    uint16_t                local_port;
    teteco_net_mode_t       client_or_server;
    teteco_audio_mode_t     audio_mode;
    chat_data_t             *chat_data;
    enc_speex_status_t      *speex_state;
    audio_data_t            *audio;

    // Private

    struct event            *udp_recv_event;
    struct event            *udp_send_event;
    struct event            *tcp_read_event;
    struct event            *tcp_send_event;

    // Net data
    int                 sd_udp;
    int                 sd_tcp;
    FILE*               fd;
    char*               file;
    char*               file_dir;
    struct sockaddr_in  local_address_in;
    socklen_t           local_address_len;
    struct sockaddr_in  remote_address_in;
    socklen_t           remote_address_len;
    unsigned int        max_transfer_rate;

    pthread_t           thread_main;


} teteco_t;

/**********************
* PROTECTED INTERFACE *
***********************/
// TO BE USED BY THE LIBRARY

void        teteco_set_status       (teteco_t* teteco, teteco_status_t status);
void        teteco_chat_received    (teteco_t* teteco, char* entry);
int         teteco_start_connection (teteco_t* teteco);
int         teteco_send_helo        (teteco_t* teteco);
teteco_t*   teteco_stop             (teteco_t* teteco);

#endif