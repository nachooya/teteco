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

#ifdef __WINDOWS__
#define PTW32_STATIC_LIB 1
#include "sys/stdio_gnu.h"
#endif

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

struct event_base *event_base = NULL;

void libevent_log_callback (int severity, const char* data) {

	fprintf (stderr, "[libevent][%d]: %s\n", severity, data);
	log_print ("[libevent][%d]: %s", severity, data);

}

void log_callback_default (char* entry) {

    fprintf (stderr, "[log]: %s\n", entry);

}

void chat_callback_default (char* entry) {

    printf ("[chat]: %s\n", entry);

}

void app_control_callback_default (int32_t argument1, int32_t argument2) {

    printf ("[app_control]: Received: %d - %d\n", argument1, argument2);

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

    if (teteco_ref == NULL) return 0;
    teteco_t* teteco = (teteco_t*) teteco_ref;

    float avg_amplitude = 0.0f;
    float max_amplitude = 0.0f;
    unsigned int num_samples = 0;

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

    //log_print ("[record]: Size: %d", size);

    for (i = 0; i < teteco->frames_per_pdu; i++) {
        frame_list_add_frame (teteco->frame_list, teteco->speex_state->frame_size, buffer+(teteco->speex_state->frame_size*i));
    }

    return size;

}

int teteco_send_control (teteco_t* teteco, protocol_t protocol) {

    char*        datagram     = NULL;
    unsigned int datagram_len = 0;
    int          result       = 1;

    protocol.control.seq = teteco->pending_control_ack++;

    protocol_build_datagram (protocol, &datagram, &datagram_len);

    if (-1 == teteco_net_send (teteco, datagram, datagram_len)) {
        result = 0;
    }
    else {
        result = 1;
    }
    util_free (datagram);
    return result;
}

int teteco_send_helo (teteco_t* teteco) {

    if (teteco == NULL) return 0;

    protocol_t      protocol_helo   = protocol_init;

    protocol_helo.control.has = 1;

    protocol_helo.control.type = teteco->audio_mode == TETECO_AUDIO_RECEIVER ? PRO_CONTROL_HELO_RECEIVER:
                                 teteco->audio_mode == TETECO_AUDIO_SENDER   ? PRO_CONTROL_HELO_SENDER:
                                 PRO_CONTROL_HELO_RECEIVER;

    int result = teteco_send_control (teteco, protocol_helo);

    return result;

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

    log_print ("[teteco]: Main thread started");

    //teteco_t *teteco = (teteco_t*) data;

    switch (event_base_dispatch (event_base)) {

        case -1:
            log_print ("[teteco]: Error on event_base_loop: event_base is: %p", event_base);
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
app_control_callback_ft     app_control_callback   = &app_control_callback_default;
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

void teteco_control_app_received (teteco_t* teteco, int32_t argument1, int32_t argument2) {

    log_print ("[teteco]: Received App control: %d - %d", argument1, argument2);
    (app_control_callback) (argument1, argument2);

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

    }
    else if (teteco->audio_mode == TETECO_AUDIO_SENDER) {


        teteco->speex_state = enc_speex_start (ENC_SPEEX_MODE_ENCODER, ENC_SPEEX_NB, 8);
        teteco->frame_list  = frame_list_new (9, teteco->speex_state->frame_size, 3);
        teteco->audio       = audio_init (teteco->speex_state->sample_rate,
                                          teteco->speex_state->frame_size*teteco->frames_per_pdu,
                                          &frame_fun_record,
                                          teteco);

        if (teteco->audio == NULL) {
            log_print ("[teteco]: Cannot start audio");
            return 0;
        }

        if (!teteco_udp_send_ready (teteco)) {
            return 0;
        }

        if (audio_record (teteco->audio_device_in, teteco->audio) == -1) {
            log_print ("[receiver]: Error playing");
            return 0;
        }

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

int teteco_set_app_control_callback (app_control_callback_ft app_control_callback_ref) {

    if (app_control_callback_ref != NULL) {
        app_control_callback = app_control_callback_ref;
    }

    (log_callback)   ("[teteco]: App control callback set..");

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

#ifdef __WINDOWS__
    pthread_win32_process_attach_np();
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        log_print ("WSAStartup failed.\n");
        return 0;
    }
#endif

    evthread_use_pthreads ();
	event_enable_debug_mode ();
	event_set_log_callback (&libevent_log_callback);

    PaError error = Pa_Initialize();

    if (error == paNoError) {
        log_print ("[teteco]: PortAudio version= %s - %d", Pa_GetVersionText(), Pa_GetVersion());
        event_base = event_base_new ();
		log_print ("[teteco]: Event base is %p", event_base);
		return 1;
    }
    else {
        log_print ("[teteco]: Error initializing PortAudio: %s", Pa_GetErrorText (error));
        return 0;
    }
}

int teteco_end (void) {

	log_print ("[teteco]: teteco_end");
#ifdef __WINDOWS__
    WSACleanup();
    pthread_win32_process_detach_np();
#endif

    PaError error = Pa_Terminate();

    if (error == paNoError) {
        log_print ("[teteco]: Stopped");
        event_base_free (event_base);
		event_base = NULL;
        return 1;
    }
    else {
        log_print ("[teteco]: Error finishing PortAudio: %s", Pa_GetErrorText (error));
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
        teteco->pending_file_ack = 0;
        teteco->pending_control_ack = 0;
        teteco->frames_per_pdu   = 3;
        teteco->frame_list       = NULL;

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

        teteco->chat_data        = chat_start ();

        teteco->enc_speex_band   = (enc_speex_band==TETECO_SPEEX_NB)?ENC_SPEEX_NB:
                                   (enc_speex_band==TETECO_SPEEX_WB)?ENC_SPEEX_WB:
                                   (enc_speex_band==TETECO_SPEEX_UWB)?ENC_SPEEX_UWB:0;

        //event_base = event_base_new ();
		//log_print ("[teteco]: Event base is %p", event_base);
        

        if (!teteco_net_start (teteco, local_port, remote_port, remote_address)) {
            log_print ("[teteco]: Error iniating network");
            return teteco_stop (teteco);
        }

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

    teteco->chat_data  = chat_stop       (teteco->chat_data);
    teteco->frame_list = frame_list_free (teteco->frame_list);



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

int teteco_udp_send_ready (teteco_t* teteco) {

    if (teteco->udp_send_event == NULL) {
        log_print ("[teteco]: This shouldn't happen two times");
        teteco->udp_send_event = event_new (event_base, frame_list_get_read_pipe_fd (teteco->frame_list), EV_READ |EV_PERSIST, teteco_udp_send_callback, teteco);
        if (0 != event_add (teteco->udp_send_event, NULL)) {
            log_print ("[teteco]: Error adding event");
            return 0;
        }
    }

    return 1;

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
            if (teteco->audio_mode == TETECO_AUDIO_RECEIVER) {
                teteco_udp_send_callback (teteco->sd_udp, -1, teteco);
            }
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

int teteco_app_control_send (teteco_t* teteco, int32_t argument1, int32_t argument2) {

    protocol_t protocol2 = protocol_init;

    protocol2.control.has = 1;
    protocol2.control.type = PRO_CONTROL_APP;
    protocol2.control.argument1 = argument1;
    protocol2.control.argument2 = argument2;

    int result = teteco_send_control (teteco, protocol2);

    log_print ("[teteco]: Sent APP Control :%d %d", argument1, argument2);

    return result;

}

int teteco_file_send (teteco_t* teteco, char* file_path) {

    if (teteco == NULL) {
        log_print ("[tedeco]: On file send: teteco not initialized");
        return 0;
    }
    // else if (teteco->pending_file_ack%2 == 1) {
        // log_print ("[tedeco]: On file send: expecting ack");
        // return 0;
    // }
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
        teteco->pending_file_ack++;
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
