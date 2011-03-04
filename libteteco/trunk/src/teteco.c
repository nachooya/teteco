/**
 * @file    teteco.c
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


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include <portaudio.h>

#include "util.h"
#include "audio.h"
#include "enc_speex.h"
#include "log.h"
#include "protocol.h"

#include "teteco.priv.h"
#include "teteco_net.h"

#define TETECO_SERVER 0
#define TETECO_CLIENT 1

/**********
* PRIVATE *
***********/

struct event_base *event_base;

void log_callback_default (char* entry) {

    fprintf (stderr, "[log]: %s\n", entry);

}

void chat_callback_default (char* entry) {

    printf ("[chat]: %s\n", entry);

}

void status_callback_default (teteco_status_t status) {

    printf ("[status]: New status: %d\n", status);

}

void file_transfer_callback_default (const char* filename, teteco_file_transfer_status_t status, uint32_t total_size, uint32_t transmitted) {

    printf ("[file]: Name: %s Transfering: %d [%d/%d]\n", filename, status, transmitted, total_size);

}

int frame_func_play (int size, int16_t* buffer, void* teteco_ref) {

    float avg_amplitude = 0.0f;
    static unsigned int max_amplitude = 0;
    unsigned int num_samples = 0;

    if (teteco_ref == NULL) return 0;

    teteco_t* teteco = (teteco_t*) teteco_ref;

    enc_speex_decode  (teteco->speex_state, buffer);

    int i;
    for (i=0; i<size; i++) {
        int val = buffer[i];
        if (val < 0) val=-val;
        if (val > max_amplitude) max_amplitude = val;
        avg_amplitude = ((avg_amplitude * (float)num_samples) + (float)val)/(float)(num_samples+1);
        num_samples++;
    }

    float Db = avg_amplitude ? 20.0f * log10f (avg_amplitude) : 0.0f;

    //Db = ( Db - 30.0f);

    teteco->current_db = Db;

    return size;

}

int frame_fun_record (int size, int16_t *buffer, void* teteco_ref) {

    float avg_amplitude = 0.0f;
    float max_amplitude = 0.0f;
    unsigned int num_samples = 0;

    if (teteco_ref == NULL) return 0;

    teteco_t* teteco = (teteco_t*) teteco_ref;

    int i;
    for (i=0; i<size; i++) {
        int val = buffer[i];
        if (val < 0) val=-val;
        if (val > max_amplitude) max_amplitude = val;
        avg_amplitude = ((avg_amplitude * (float)num_samples) + (float)val)/(float)(num_samples+1);
        num_samples++;
    }


    float Db = avg_amplitude ? 20.0f * log10f (avg_amplitude) : 0.0f;

    //Db = ( Db - 30.0f);

    teteco->current_db = Db;

    int speex_size = enc_speex_encode (teteco->speex_state, buffer);

    return speex_size;

}

int teteco_send_helo (teteco_t* teteco) {

    if (teteco == NULL) return 0;

    char*           datagram        = NULL;
    unsigned int    datagram_len    = 0;
    protocol_t      protocol_helo   = protocol_init;

    protocol_helo.control.has = 1;

    protocol_helo.control.type = teteco->audio_mode == TETECO_AUDIO_RECEIVER ? PRO_CONTROL_HELO_RECEIVER:
                                 teteco->audio_mode == TETECO_AUDIO_SENDER   ? PRO_CONTROL_HELO_SENDER:
                                 PRO_CONTROL_HELO_RECEIVER;

    protocol_print (protocol_helo);
    protocol_build_datagram (protocol_helo, &datagram, &datagram_len);

    if (-1 == teteco_net_send (teteco, datagram, datagram_len)) {
        util_free (datagram);
        return 0;
    }
    util_free (datagram);

    return 1;

}

int teteco_send_bye (teteco_t* teteco) {

    if (teteco == NULL) return 0;

    log_print ("[teteco]: Sending bye\n");

    char*           datagram        = NULL;
    unsigned int    datagram_len    = 0;
    protocol_t      protocol_bye    = protocol_init;

    protocol_bye.control.has = 1;
    protocol_bye.control.type = PRO_CONTROL_BYE;
    protocol_print (protocol_bye);
    protocol_build_datagram (protocol_bye, &datagram, &datagram_len);

    if (-1 == teteco_net_send (teteco, datagram, datagram_len)) {
        util_free (datagram);
        return 0;
    }
    util_free (datagram);

    return 1;

}

void* teteco_main_thread (void* data) {

    if (data == NULL) {
        log_print ("[main]: Receive thread: teteco is NULL. Terminatting thread");
        return NULL;
    }

    log_print ("[main]: Main thread started");

    //teteco_t *teteco = (teteco_t*) data;

    switch (event_base_dispatch (event_base)) {

        case -1: 
            log_print ("[teteco]: Error on event_base_loop");
            break;
        case 0:
            log_print ("[teteco]: Main thread stoped");
            break;
        case 1: 
            log_print ("[teteco]: No events registered");
            break;
        default:
            log_print ("[teteco: Unknown libevent error");
            break;
    }


    return NULL;

}


/************
* PROTECTED *
************/

log_callback_ft             log_callback           = &log_callback_default;
chat_callback_ft            chat_callback          = &chat_callback_default;
status_callback_ft          status_callback        = &status_callback_default;
file_transfer_callback_ft   file_transfer_callback = &file_transfer_callback_default;


void teteco_set_status (teteco_t* teteco, teteco_status_t status) {

    teteco->status = status;
    (status_callback) (status);

}

void teteco_chat_received (teteco_t* teteco, char* entry) {

    (chat_callback) (entry);
    free (entry);

}

int teteco_start_connection (teteco_t* teteco) {

    if (teteco == NULL) {
        log_print ("[tedeco]: Teteco not initialized");
        return 0;
    }

    teteco->time_start = time(NULL);

    if (teteco->audio_mode == TETECO_AUDIO_RECEIVER) {

        teteco->speex_state      = enc_speex_start (ENC_SPEEX_MODE_DECODER, ENC_SPEEX_NB, 8);
        teteco->audio            = audio_init (teteco->speex_state->sample_rate, teteco->speex_state->frame_size, &frame_func_play, teteco);

        if (teteco->audio == NULL) {
            log_print ("[teteco]: Cannot start audio");
            return 0;
        }
        if (audio_play (teteco->audio_device_out, teteco->audio) == -1) {
            log_print ("[receiver]: Error playing");
            return 0;
        }
        struct timeval ten_usec;
        ten_usec.tv_sec  = 0;
        ten_usec.tv_usec = 1000000/10;

        teteco->udp_send_event = event_new (event_base, -1, EV_TIMEOUT, teteco_udp_send_callback, teteco);
        event_add (teteco->udp_send_event, &ten_usec);

    }
    else if (teteco->audio_mode == TETECO_AUDIO_SENDER) {

        teteco->speex_state      = enc_speex_start (ENC_SPEEX_MODE_ENCODER, ENC_SPEEX_NB, 8);
        teteco->audio            = audio_init (teteco->speex_state->sample_rate, teteco->speex_state->frame_size, &frame_fun_record, teteco);

        if (teteco->audio == NULL) {
            log_print ("[teteco]: Cannot start audio");
            return 0;
        }
        if (audio_record (teteco->audio_device_in, teteco->audio) == -1) {
            log_print ("[receiver]: Error playing");
            return 0;
        }
        struct timeval ten_usec;
        ten_usec.tv_sec  = 0;
        ten_usec.tv_usec = 1000000/10;

        teteco->udp_send_event = event_new (event_base, -1, EV_TIMEOUT, teteco_udp_send_callback, teteco);
        event_add (teteco->udp_send_event, &ten_usec);

    }
    else {
        log_print ("[teteco]: Undefined audio mode");
        return 0;
    }

    return 1;

}

/*********
* PUBLIC *
**********/


int teteco_set_log_callback (log_callback_ft log_callback_ref) {

    if (log_callback_ref != NULL) {
        log_set_callback (log_callback_ref);
    }

    (log_callback) ("[teteco]: Log Working!!");

    return 1;

}

int teteco_set_chat_callback (chat_callback_ft chat_callback_ref) {

    if (chat_callback_ref != NULL) {
        chat_callback = chat_callback_ref;
    }

    (log_callback)   ("[teteco]: Chat callback set: Testing...");

    return 1;

}

int teteco_set_status_callback (status_callback_ft status_callback_ref) {

   if (status_callback_ref != NULL) {
        status_callback = status_callback_ref;
   }

   (log_callback)   ("[teteco]: Status callback set");

   return 1;

}

int teteco_set_file_transfer_callback (file_transfer_callback_ft file_transfer_callback_ref) {

   if (file_transfer_callback_ref != NULL) {
        file_transfer_callback = file_transfer_callback_ref;
   }

   (log_callback)   ("[teteco]: File callback set");


    return 1;

}


int teteco_init () {

    PaError error = Pa_Initialize();

    if (error == paNoError) {
        log_print ("[teteco]: PortAudio version= %s - %d", Pa_GetVersionText(), Pa_GetVersion());
        return 0;
    }
    else {
        log_print ("[teteco]: Error initializing PortAudio: %s", Pa_GetErrorText (error));
        return 1;

        event_base = event_base_new ();

    }

}

int teteco_end (void) {

    PaError error = Pa_Terminate();

    if (error == paNoError) {
        log_print ("[teteco]: Stopped");

        event_base_free (event_base);

        return 1;
    }
    else {
        log_print ("[audio]: Error finishing PortAudio: %s", Pa_GetErrorText (error));
        return -1;
    }

}



teteco_t* teteco_start        (teteco_net_mode_t    client_or_server,
                               uint16_t             local_port,
                               uint16_t             remote_port,
                               const char*          remote_address,
                               int                  audio_device_in,
                               int                  audio_device_out,
                               teteco_audio_mode_t  audio_mode,
                               teteco_speex_band_t  enc_speex_band,
                               int                  enc_quality,
                               const char*          user_directory)

{

    if (user_directory == NULL) {
        return NULL;
    }

    log_print ("[teteco]: Starting server");
    teteco_t* teteco = malloc (sizeof(teteco_t));

    if (teteco != NULL) {

        teteco->file_dir         = strdup (user_directory);
        teteco->client_or_server = client_or_server;
        teteco->audio_device_in  = audio_device_in;
        teteco->audio_device_out = audio_device_out;
        teteco->local_port       = local_port;
        teteco->remote_port      = remote_port;
        teteco->remote_address   = remote_address == NULL? NULL: strdup (remote_address);
        teteco->enc_quality      = enc_quality;
        teteco->voice_ack_every  = 10;
        teteco->audio_mode       = audio_mode;
        teteco->status           = TETECO_STATUS_DISCONNECTED;
        teteco->current_db       = 0.0f;
        teteco->speex_state      = NULL;
        teteco->audio            = NULL;

#ifdef __LINUX__
        teteco->thread_main      = 0;
#endif

        teteco->total_bytes_in   = 0;
        teteco->total_bytes_out  = 0;
        teteco->time_start       = 0;
        teteco->packets_expected = 0;
        teteco->packets_received = 0;
        teteco->transfer_rate    = 0;

        teteco->tcp_read_event   = NULL;
        teteco->tcp_send_event   = NULL;
        teteco->udp_send_event   = NULL;
        teteco->udp_recv_event   = NULL;

        teteco->sd_udp = 0;
        teteco->sd_tcp = 0;
        teteco->fd = NULL;
        teteco->file = NULL;
        teteco->max_transfer_rate = 0;
        teteco->local_address_len = sizeof (struct sockaddr_in);
        teteco->remote_address_len = sizeof (struct sockaddr_in);

        teteco->enc_speex_band   = (enc_speex_band==TETECO_SPEEX_NB)?ENC_SPEEX_NB:
                                   (enc_speex_band==TETECO_SPEEX_WB)?ENC_SPEEX_WB:
                                   (enc_speex_band==TETECO_SPEEX_UWB)?ENC_SPEEX_UWB:0;

        event_base = event_base_new ();

        if (!teteco_net_start (teteco, local_port, remote_port, remote_address)) {
            log_print ("[teteco]: Error iniating network");
            return teteco_stop (teteco);
        }

        teteco->chat_data        = chat_start ();

        if (client_or_server == TETECO_CLIENT) {
            if (!teteco_send_helo (teteco)) {
                log_print ("[teteco]: Error sending HELO");
                return teteco_stop (teteco);
            }
            else {
                teteco_set_status (teteco, TETECO_STATUS_CONNECTING);
            }
        }
        else {
            teteco_set_status (teteco, TETECO_STATUS_WAITING);
            log_print ("[server]: Waiting for client HELO...");
        }

        if (0 != pthread_create (&teteco->thread_main, NULL, teteco_main_thread, teteco)) {
            log_print ("[client]: Error starting main thread: %m");
            return teteco_stop (teteco);
        }
    }

    return teteco;

}



teteco_t* teteco_stop (teteco_t* teteco) {

    log_print ("[teteco]: Stopping...");

    if (teteco == NULL) return teteco;


    if (!teteco_send_bye (teteco)) {
        log_print ("[teteco]: Error sending BYE");
    }

    if (teteco->audio) {
        if (audio_end (teteco->audio) == -1) {
            log_print ("[teteco]: Error ending: %m");
        }
        teteco->audio = NULL;
    }

    //if (teteco->remote_address != NULL) free (teteco->remote_address);


    if (teteco->speex_state != NULL) {
        if (-1 == enc_speex_stop (teteco->speex_state)) {
            log_print ("[teteco]: Error stopping speex");
        }
    }

    if (!teteco_net_stop (teteco)) {
        log_print ("[teteco]: Error stopping net");
    }

    teteco->chat_data = chat_stop (teteco->chat_data);



/*    if (teteco->fd != NULL)         fclose (teteco->fd);
    if (teteco->file != NULL)       free   (teteco->file);*/
    //if (teteco->file_dir != NULL)   free   (teteco->file_dir);

//     int ret;
//     log_print ("[teteco]: Waiting for thread\n");
//     if ((ret = pthread_join (teteco->thread_main, NULL)) != 0) {
//         log_print ("[teteco]: Error joining thread: %d: %s: %m\n", ret, strerror(ret));
//     }

    //teteco_set_status (teteco, TETECO_STATUS_DISCONNECTED);

    free (teteco);

    log_print ("[teteco]: Stopped");

    return NULL;

}

int teteco_get_in_devices (int** index, char*** devices) {

    return audio_get_devices (AUDIO_DEV_IN, index, devices);

}

int teteco_get_out_devices (int** index, char*** devices) {

    return audio_get_devices (AUDIO_DEV_OUT, index, devices);

}

int teteco_chat_send (teteco_t* teteco, const char* comment) {

    if (teteco == NULL) {
        log_print ("[tedeco]: On chat send: teteco not initialized");
        return 0;
    }
    else if (teteco->status == TETECO_STATUS_CONNECTED) {
        if (chat_add (teteco->chat_data, comment)) {
            teteco_udp_send_callback (teteco->sd_udp, -1, teteco);
        }
        else {
            log_print ("[teteco]: Cannon insert chat entry");
            return 0;
        }
    }
    else {
        log_print ("[tedeco]: On chat send: Teteco not connected");
        return 0;
    }
    return 1;
}

int teteco_file_send (teteco_t* teteco, char* file_path) {

    if (teteco == NULL) {
        log_print ("[tedeco]: On file send: teteco not initialized");
        return 0;
    }
    else if (teteco->audio_mode != TETECO_AUDIO_SENDER) {
        log_print ("[tedeco]: On file send: you are not sender!");
        return 0;
    }
    else if (teteco->status == TETECO_STATUS_CONNECTED) {

        int local_port = 0;

        char*        datagram = NULL;
        unsigned int datagram_len = 0;

        teteco->file = strdup (file_path);

        protocol_t protocol2 = protocol_init;
        protocol2.file.has = 1;
        protocol2.file.type = PRO_FILE_TYPE_SEND;
        if (teteco->client_or_server == TETECO_NET_SERVER) {
            if (!teteco_net_file_listen (teteco, &local_port)) {
                return 0;
            }
        }
        protocol2.file.port = local_port;
        protocol_build_datagram (protocol2, &datagram, &datagram_len);
        teteco_net_send (teteco, datagram, datagram_len);
        util_free (datagram);
    }
    else {
        log_print ("[tedeco]: On chat send: Teteco not connected");
        return 0;
    }
    return 1;
}

void teteco_set_max_transfer_rate (teteco_t* teteco, uint32_t transfer_rate) {

    teteco->max_transfer_rate = transfer_rate * 1024;

}

char* teteco_get_remote_address (teteco_t* teteco) {

    if (teteco->status != TETECO_STATUS_CONNECTED) {
        return NULL;
    }

    char* address = NULL;

    asprintf (&address, "%s:%u", inet_ntoa(teteco->remote_address_in.sin_addr), ntohs(teteco->remote_address_in.sin_port));

    return address;

}

float teteco_get_current_db (teteco_t* teteco) {
    return teteco->current_db;
}

uint32_t teteco_get_total_bytes_out (teteco_t* teteco) {
    return teteco->total_bytes_out;
}

uint32_t teteco_get_total_bytes_in (teteco_t* teteco) {
    return teteco->total_bytes_in;
}

uint32_t teteco_get_time_start (teteco_t* teteco) {
    return teteco->time_start;
}

teteco_status_t teteco_get_status (teteco_t* teteco) {
    return teteco->status;
}

uint32_t teteco_get_packets_expected (teteco_t* teteco) {
    return teteco->packets_expected;
}

uint32_t teteco_get_packets_received (teteco_t* teteco) {
    return teteco->packets_received;
}

uint32_t teteco_get_transfer_rate (teteco_t* teteco) {
    return teteco->transfer_rate;
}

void teteco_print_conf (teteco_t* teteco) {

    if (teteco == NULL) return;

    printf ("\tSTATUS         : %s\n", teteco->status==TETECO_STATUS_DISCONNECTED?"Disconnected":
                              teteco->status==TETECO_STATUS_CONNECTING?"Connecting":
                              teteco->status==TETECO_STATUS_WAITING?"Waiting":
                              teteco->status==TETECO_STATUS_CONNECTED?"Connected":"Undefined");
    printf ("\tNet Mode:      : %s\n", teteco->client_or_server==TETECO_NET_SERVER?"SERVER":
                                       teteco->client_or_server==TETECO_NET_CLIENT?"CLIENT":"Undefined");
    printf ("\tAudio Mode     : %s\n", teteco->audio_mode==TETECO_AUDIO_SENDER?"Sender":
                                       teteco->audio_mode==TETECO_AUDIO_RECEIVER?"Receiver":"Undefined");
    printf ("\tLocal address  : %s:%u\n", "localhost", teteco->local_port);
    printf ("\tRemote address : %s:%u\n", teteco->remote_address, teteco->remote_port);
    printf ("\tSpeex Mode     : %s\n", teteco->enc_speex_band==TETECO_SPEEX_NB?"Narrow Band":
                                       teteco->enc_speex_band==TETECO_SPEEX_WB?"Wide Band":
                                       teteco->enc_speex_band==TETECO_SPEEX_UWB?"Ultra Wide Band":"Undefined");
    printf ("\tSpeex Qualite  : %d\n", teteco->enc_quality);
    printf ("\tVoice ACK      : %d\n", teteco->voice_ack_every);
    printf ("\tAudio In       : %d\n", teteco->audio_device_in);
    printf ("\tAudio Out      : %d\n", teteco->audio_device_out);

}
