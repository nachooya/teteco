/**
 * @file    teteco_net.c
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

#ifdef __WINDOWS__

#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#include "sys/stdio_gnu.h"

#define timersub(a, b, result)                                               \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                          \
    if ((result)->tv_usec < 0) {                                              \
      --(result)->tv_sec;                                                     \
      (result)->tv_usec += 1000000;                                           \
    }                                                                         \
  } while (0)

#else

#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/wait.h>

#define closesocket(a) close(a)

#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <libgen.h>

#include "log.h"
#include "util.h"
#include "teteco_net.h"
#include "protocol.h"

extern struct event_base           *event_base;
extern file_transfer_callback_ft   file_transfer_callback;

int teteco_net_dns_resolv (teteco_t* teteco, const char* fqdn, int port) {

    struct addrinfo *result;
    struct sockaddr_storage;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;      /* Allow IPv4 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    char* port_str = NULL;
    asprintf (&port_str, "%u", port);

    int s;
    s=getaddrinfo (fqdn, port_str, &hints, &result);
    util_free (port_str);

    if (s != 0) {
        log_print("[teteco_net]: getaddrinfo: %s", gai_strerror(s));
        return 0;
    }

    if (result != NULL) {
        memcpy (&teteco->remote_address_in, result->ai_addr, sizeof (struct sockaddr_in));
        //teteco->client_address_in = (struct sockaddr_in*)result->ai_addr;
        teteco->remote_address_len                = sizeof (teteco->remote_address_in);

        log_print ("[teteco_net]: peer at %s:%d", inet_ntoa(teteco->remote_address_in.sin_addr), ntohs(teteco->remote_address_in.sin_port));


        freeaddrinfo(result);
        return 1;
    }
    else {
        log_print ("[teteco_net]: Cannot resolve %s:%u", fqdn, port);
        return 0;
    }

}


int teteco_net_start (teteco_t* teteco, int local_port, int remote_port, const char* remote_address) {

    teteco->sd_tcp     = 0;
    teteco->fd         = NULL;

    // Resolv dns

    if (remote_address != NULL) {
        teteco_net_dns_resolv (teteco, remote_address, remote_port);
    }

    // Net related
#define local_address   (struct sockaddr*)&teteco->local_address_in
#define remote_address  (struct sockaddr*)&teteco->remote_address_in

    int                 res_bind, res_setsockopt;

    memset (&teteco->local_address_in , 0, sizeof (teteco->local_address_in));

#ifdef __WINDOWS__
    char on = 1;
#else
    int on  = 1;
#endif

    teteco->local_address_in.sin_family      = AF_INET;
    teteco->local_address_in.sin_port        = htons (local_port);
    teteco->local_address_in.sin_addr.s_addr = htonl (INADDR_ANY);

    teteco->local_address_len                 = sizeof(struct sockaddr_in);

    if ( 0 > (teteco->sd_udp = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP))) {
        log_print ("[teteco_net]: Error in socket: %m %s", strerror (errno));
        return 0;
    }
    else if ( 0 > (res_setsockopt = setsockopt (teteco->sd_udp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))) {
        log_print ("[teteco_net]: Error in setsockopt: %m");
        closesocket (teteco->sd_udp);
        return 0;
    }
    else if ( 0 > (res_bind = bind (teteco->sd_udp, local_address, sizeof(struct sockaddr_in)))) {
        log_print ("[teteco_net]: Error in bind: %m");
        closesocket (teteco->sd_udp);
        return 0;
    }

    socklen_t local_address_len = sizeof(struct sockaddr_in);
    getsockname (teteco->sd_udp, local_address, &local_address_len);

    teteco->udp_recv_event = event_new (event_base, teteco->sd_udp, EV_READ, teteco_udp_recv_callback, teteco);

    struct timeval ten_sec;
    ten_sec.tv_sec  = 5;
    ten_sec.tv_usec = 0;

    if (0 != event_add (teteco->udp_recv_event, (teteco->client_or_server==TETECO_NET_SERVER)?NULL:&ten_sec)) {
        log_print ("[teteco_net]: Error adding event");
        event_free (teteco->udp_recv_event);
        teteco->udp_recv_event = NULL;
        closesocket (teteco->sd_udp);
        return 0;
    }

    log_print ("[teteco_net]: listening...");
    return 1;
}

int teteco_net_stop (teteco_t* teteco) {

    log_print ("[teteco_net]: Stoping net\n");

    if (teteco->udp_send_event != NULL) {
        event_del  (teteco->udp_send_event);
        event_free (teteco->udp_send_event);
        teteco->udp_send_event = NULL;
    }

    if (teteco->udp_recv_event != NULL) {
        event_del  (teteco->udp_recv_event);
        event_free (teteco->udp_recv_event);
        teteco->udp_recv_event = NULL;
    }

    if (teteco->tcp_read_event != NULL) {
        event_del  (teteco->tcp_read_event);
        event_free (teteco->tcp_read_event);
        teteco->tcp_read_event = NULL;
    }

    if (teteco->tcp_send_event != NULL) {
        event_del  (teteco->tcp_send_event);
        event_free (teteco->tcp_send_event);
        teteco->tcp_send_event = NULL;
    }

    if (teteco->sd_tcp > 0) {
        if (-1 == closesocket (teteco->sd_udp)) {
            log_print ("[teteco_net]: Error closing socket: %m");
        }
    }

    if (event_base != NULL) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1;
        if (0 != event_base_loopexit (event_base, &tv)) {
            log_print ("[teteco_net]: Error in event loop break\n");
        }
        log_print ("[teteco_net]: Event loop finished\n");
    }



/*    else {
        if (teteco->sd_tcp > 0) close (teteco->sd_tcp);
        if (teteco->fd != NULL ) fclose (teteco->fd);

    }*/
    return 1;

}

int teteco_net_send (teteco_t* teteco, char* buffer, int len) {

    if (teteco == NULL) {
        log_print ("[teteco_net]: Network no initialized");
        return 0;
    }

    //log_print ("[teteco_net]: Enviando paquete a %s:%d [%d]", inet_ntoa(teteco->remote_address_in.sin_addr), ntohs(teteco->remote_address_in.sin_port), len);
    ssize_t sent_len;
    sent_len = sendto (teteco->sd_udp, buffer, len, 0, remote_address, teteco->remote_address_len);

    if (sent_len == -1) {
        log_print ("[teteco_net]: Error sending: %m");
    }

    return sent_len;
}

void teteco_udp_recv_callback (int sd, short event, void *teteco_ref) {

    teteco_t* teteco = (teteco_t*) teteco_ref;

    struct timeval ten_sec;
    ten_sec.tv_sec  = 5;
    ten_sec.tv_usec = 0;

    if (event == EV_TIMEOUT) {
        event_del  (teteco->udp_recv_event);
        event_free (teteco->udp_recv_event);
        teteco->udp_recv_event = NULL;
        log_print ("[teteco_net]: Timeout received");
        teteco_set_status (teteco, TETECO_STATUS_TIMEDOUT);
        return;
    }

    static PRO_VOICE_SEQ_TYPE last_received = 0;

    static uint16_t nr = 0;
    static uint16_t last_sent_ack = 0;
    static uint16_t ack = 0;

    int bye_received = 0;

    char input[MAX_DATAGRAM_SIZE] = {0};

    ssize_t recv_len = recvfrom (teteco->sd_udp, input, MAX_DATAGRAM_SIZE, 0, remote_address, &teteco->remote_address_len);
    if (recv_len == -1) {
        log_print ("[teteco_net]: Error receiving: %m");
        event_add (teteco->udp_recv_event, &ten_sec);
        return;
    }
    else {
        //log_print ("[teteco_net]: Received packet from %s:%d [%d]", inet_ntoa(teteco->remote_address_in.sin_addr), ntohs(teteco->remote_address_in.sin_port), recv_len);
    }

    teteco->total_bytes_in+=recv_len;

    protocol_t protocol;
    memset (&protocol, 0, sizeof (protocol));
    protocol = protocol_parse_datagram (input, recv_len);


    if (protocol.status) {
        if (protocol.control.has) {
            if (PRO_CONTROL_IS_HELO_RECEIVER (protocol.control.type) || PRO_CONTROL_IS_HELO_SENDER (protocol.control.type)) {
                log_print ("[receiver]: Received HELO");
                nr = 0;
                last_sent_ack = 0;
                ack = 0;
                if (teteco->status == TETECO_STATUS_WAITING || teteco->status == TETECO_STATUS_CONNECTING) {
                    teteco_set_status (teteco, TETECO_STATUS_CONNECTED);
                    if (teteco->client_or_server == TETECO_NET_SERVER) {
                        if (!teteco_send_helo (teteco)) {
                            log_print ("[receiver]: Error sending helo");
                            return;
                        }
                    }
                    else {
                        if (PRO_CONTROL_IS_HELO_RECEIVER (protocol.control.type))
                            teteco->audio_mode = TETECO_AUDIO_SENDER;
                        else
                            teteco->audio_mode = TETECO_AUDIO_RECEIVER;
                    }
                    if (!teteco_start_connection (teteco)) {
                        log_print ("[receiver]: Error on teteco start");
                        return;
                    }
                }
                else {
                    log_print ("[receiver]: Unexpected control HELLO");
                }
            }
            else if (PRO_CONTROL_IS_BYE (protocol.control.type)) {
                if (teteco->status == TETECO_STATUS_CONNECTED) {
                    log_print ("[receiver]: BYE received");
                    bye_received = 1;
                }
                else {
                    log_print ("[receiver]: Unexpected control BYE");
                }
            }
			else if (PRO_CONTROL_IS_APP (protocol.control.type)) {
				teteco_control_app_received (teteco, protocol.control.argument1, protocol.control.argument2);
			}
        }

        if (protocol.voice_ack.has) {
            //log_print ("\n\nACK seq:%u num:%u\n\n", protocol.voice_ack.last, protocol.voice_ack.number_received);
        }

        if (protocol.voice.has) {
            if (teteco->audio_mode == TETECO_AUDIO_RECEIVER) {
                enc_speex_put_sample (teteco->speex_state, protocol.voice.payload, protocol.voice.size);

                teteco->packets_received++;
                teteco->packets_expected+= (protocol.voice.seq < last_received)? ((USHRT_MAX - last_received) + protocol.voice.seq):(protocol.voice.seq - last_received);
                last_received = protocol.voice.seq;
                ack = last_received;
                nr++;

                if (ack-last_sent_ack >= teteco->voice_ack_every || ack < last_sent_ack) {

                    char*        datagram = NULL;
                    unsigned int datagram_len = 0;

                    protocol_t protocol2 = protocol_init;
                    protocol2.voice_ack.has = 1;
                    protocol2.voice_ack.last = ack;
                    protocol2.voice_ack.number_received = nr;
                    protocol_build_datagram (protocol2, &datagram, &datagram_len);
                    teteco_net_send (teteco, datagram, datagram_len);
                    util_free (datagram);
                    last_sent_ack = ack;
                    nr = 0;
                }
            }
            else {
                log_print ("[receiver]: Received audio but I am the sender!!");
            }
        }
        if (protocol.chat.has) {
            log_print ("[receiver]: Chat size: %d seq:%d\n", protocol.chat.size, protocol.chat.seq);
            char *chat_entry = malloc (protocol.chat.size+1);
            memcpy (chat_entry, protocol.chat.payload, protocol.chat.size);
            chat_entry [protocol.chat.size] = '\0';
            chat_ack_add (teteco->chat_data, protocol.chat.seq);
            teteco_chat_received (teteco, chat_entry);
        }
        if (protocol.chat_ack.has) {
            log_print ("[receiver]: Received Chat ack: %d\n", protocol.chat_ack.seq);
            chat_ack (teteco->chat_data, protocol.chat_ack.seq);
        }
        if (protocol.file.has) {
            log_print ("[receiver]: File at %u\n", protocol.file.port);
            if (protocol.file.type == PRO_FILE_TYPE_SEND) {
                if (protocol.file.port == 0) {
                    int local_port = 0;
                    teteco_net_file_listen (teteco, &local_port);
                    protocol_t protocol2 = protocol_init;
                    char*        datagram = NULL;
                    unsigned int datagram_len = 0;
                    protocol2.file.has = 1;
                    protocol2.file.type = PRO_FILE_TYPE_RECV;
                    protocol2.file.port = local_port;
                    protocol_build_datagram (protocol2, &datagram, &datagram_len);
                    teteco_net_send (teteco, datagram, datagram_len);
                    util_free (datagram);
                }
                if (protocol.file.type == PRO_FILE_TYPE_SEND) {
                    teteco_net_file_connect (teteco, protocol.file.port);
                }
            }
            else {
                log_print ("[receiver]: not implemented");
            }
        }
    }
    else {
        log_print ("[receiver]: Error parsing datagram\n");
    }



    if (bye_received && 
        (teteco->status == TETECO_STATUS_WAITING ||
         teteco->status == TETECO_STATUS_CONNECTED ||
         teteco->status == TETECO_STATUS_CONNECTING))

    {

        teteco_set_status (teteco, TETECO_STATUS_DISCONNECTED);
    }

    event_add (teteco->udp_recv_event, &ten_sec);

}


void teteco_udp_send_callback (int sd, short event, void *teteco_ref) {

    teteco_t* teteco = (teteco_t*) teteco_ref;

    if (teteco->status == TETECO_STATUS_DISCONNECTED) return;

    char*        datagram = NULL;
    unsigned int datagram_len = 0;
    protocol_t   protocol = protocol_init;

    // SEQ VOICE
    static PRO_VOICE_SEQ_TYPE last_sent = 1;

    if (teteco->status == TETECO_STATUS_CONNECTED) {

        memset (&protocol, 0, sizeof (protocol_t));

        // Only send voice if it is sender

        if (teteco->audio_mode == TETECO_AUDIO_SENDER) {
            // Get voice
            int FRAME_SIZE  = teteco->speex_state->frame_size;
            int SAMPLE_RATE = teteco->speex_state->sample_rate;
            int ENC_SIZE    = teteco->speex_state->encoded_frame_size+5;
            protocol.voice.size = circular_buffer_read (teteco->speex_state->buffer, &protocol.voice.payload, (SAMPLE_RATE/FRAME_SIZE)*ENC_SIZE);
            if (protocol.voice.size > 0) {
                protocol.status = 1;
                protocol.voice.has = 1;
                last_sent = last_sent == 0? 1:last_sent;
                protocol.voice.seq = last_sent++;
            }
        }

        // Get Chat
        chat_node_t* chat_node = chat_get (teteco->chat_data);

        if (chat_node != NULL) {
            //(log_print) (LOG_DEBUG, "[server]: Sending: %s", chat_entry);
            protocol.status = 1;
            protocol.chat.has = 1;
            protocol.chat.seq  = chat_node->comment_number;
            protocol.chat.size = chat_node->size;
            log_print ("[sender]: Chat entry: seq:%d size:%d comment:%s", chat_node->comment_number, chat_node->size, chat_node->comment);
            memcpy (&protocol.chat.payload, chat_node->comment, chat_node->size);
        }

        // Get Chat ack
        chat_ack_node_t* chat_ack = chat_ack_get (teteco->chat_data);

        if (chat_ack != NULL) {
            protocol.status = 1;
            protocol.chat_ack.has = 1;
            protocol.chat_ack.seq = chat_ack->ack_num;
        }

        if (protocol.status == 1) {
            if (protocol_build_datagram (protocol, &datagram, &datagram_len)) {
                int net_sent = teteco_net_send (teteco, datagram, datagram_len);
                teteco->total_bytes_out += net_sent;
                util_free (datagram);
            }
            else {
                log_print ("[sender]: Error building datagram");
            }
        }
        else {
            //log_print ("[sender]: Nothing to send");
        }
    }

    struct timeval ten_usec;
    ten_usec.tv_sec  = 0;
    ten_usec.tv_usec = 1000000/10;

    if (teteco->udp_send_event == NULL) {
        log_print ("[teteco_net]: This shouldn't happen\n");
        teteco->udp_send_event = event_new (event_base, -1, EV_TIMEOUT, teteco_udp_send_callback, teteco);
    }
    if (0 != event_add (teteco->udp_send_event, &ten_usec)) {
        log_print ("[teteco_net]: Error adding event\n");
    }

}

int teteco_net_file_listen (teteco_t* teteco, int *local_port) {

    struct sockaddr_in local_address_tcp_in;

    socklen_t address_in_len = sizeof (struct sockaddr_in);

    local_address_tcp_in = teteco->local_address_in;
    local_address_tcp_in.sin_port = htonl (0);

    int sd_listen = 0;

    if ( 0 > (sd_listen = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
        log_print ("[teteco_net]: Error in socket: %m");
        return 0;
    }
	
	 

    if ( 0 > bind (sd_listen, (const struct sockaddr*)&local_address_tcp_in, sizeof (struct sockaddr_in))) {
        log_print ("[teteco_net]: Error on bind: %m");
        closesocket (teteco->sd_tcp);
        teteco->sd_tcp = 0;
        return 0;
    }

    // Get the binded port

    if ( 0 > getsockname (sd_listen, (struct sockaddr*)&local_address_tcp_in, &address_in_len)) {
        log_print ("[teteco_net]: Error on getsockname: %m");
        closesocket (teteco->sd_tcp);
        teteco->sd_tcp = 0;
        return 0;
    }

    *local_port = ntohs (local_address_tcp_in.sin_port);

    log_print ("[teteco_net]: Local port is :%d\n", *local_port);

    if ( 0 > listen (sd_listen, 0)) {
        log_print ("[teteco_net]: Error on listen: %m");
        closesocket (teteco->sd_tcp);
        teteco->sd_tcp = 0;
        return 0;
    }

    if (teteco->audio_mode == TETECO_AUDIO_SENDER) {
        teteco->tcp_send_event = event_new (event_base, sd_listen, EV_READ, teteco_net_file_send_callback, teteco);
        event_add (teteco->tcp_send_event, NULL);
    }
    else {
        teteco->tcp_read_event = event_new (event_base, sd_listen, EV_READ, teteco_net_file_recv_callback, teteco);
        event_add (teteco->tcp_read_event, NULL);
    }

    teteco->transfer_rate = 0;

    return 1;

}

int teteco_net_file_connect (teteco_t* teteco, int remote_port) {

    struct sockaddr_in remote_address_tcp_in;

    remote_address_tcp_in = teteco->remote_address_in;
    remote_address_tcp_in.sin_port = htons (remote_port);

    if ( 0 > (teteco->sd_tcp = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP))) {
        log_print ("[teteco_net]: Error in socket: %m");
        return 0;
    }

    if ( 0 > connect (teteco->sd_tcp, (const struct sockaddr*)&remote_address_tcp_in, sizeof (struct sockaddr_in))) {
        log_print ("[teteco_net]: Error in connect: %m\n");
        closesocket (teteco->sd_tcp);
        teteco->sd_tcp = 0;
        return 0;
    }

    if (teteco->audio_mode == TETECO_AUDIO_SENDER) {
        teteco->tcp_send_event = event_new (event_base, -1, EV_TIMEOUT, teteco_net_file_send_callback, teteco);
        event_add (teteco->tcp_send_event, NULL);
    }
    else {
        teteco->tcp_read_event = event_new (event_base, teteco->sd_tcp, EV_READ, teteco_net_file_recv_callback, teteco);
        event_add (teteco->tcp_read_event, NULL);
    }

    teteco->transfer_rate = 0;

    return 1;

}


void teteco_net_file_send_callback (int sd, short event, void *teteco_ref) {

    teteco_t* teteco = (teteco_t*) teteco_ref;

    static unsigned int     chunk_size        = 10240; // Bytes
    static unsigned int     sent_so_far       = 0;    // Bytes
    static struct timeval   start_time;
    struct timeval          current_time;
    struct timeval          total_time;
    static struct timeval   sleep_time;
    int8_t buffer[chunk_size];

    const char* file_name      = basename (teteco->file);
    uint16_t    file_name_size = strlen   (file_name);
    static uint32_t file_size = 0;

    //log_print ("[teteco_net]: file send callback\n");

    // Connection establishement

    if (teteco->sd_tcp <= 0) {

        struct sockaddr_in remote_address_tcp_in;
        socklen_t address_in_len = sizeof (struct sockaddr_in);

        log_print ("[teteco_net]: Connection establishement\n");

        event_free (teteco->tcp_send_event);
        teteco->tcp_send_event = NULL;

        teteco->sd_tcp = accept (sd, (struct sockaddr*)&remote_address_tcp_in, &address_in_len);

        if (0 > teteco->sd_tcp) {
            log_print ("[teteco_net]: Error on aceept: %m");
            closesocket (sd);
            teteco->sd_tcp = 0;
            return;
        }
        closesocket (sd);

        sleep_time.tv_sec = 0;
        sleep_time.tv_usec = 1;

        if (teteco->max_transfer_rate != 0) {
            teteco->tcp_send_event = event_new (event_base, -1, EV_TIMEOUT, teteco_net_file_send_callback, teteco);
        }
        else {
            teteco->tcp_send_event = event_new (event_base, teteco->sd_tcp, EV_WRITE, teteco_net_file_send_callback, teteco);
        }
        event_add (teteco->tcp_send_event, &sleep_time);
        return;

    }

    if (teteco->fd == NULL) {

        log_print ("[teteco_net]: Opening file: %s\n", teteco->file);

        sent_so_far = 0;
        gettimeofday (&start_time, NULL);

        sleep_time.tv_sec   = 0;
        sleep_time.tv_usec  = 1000000 / 10;

        if ((teteco->fd = fopen (teteco->file, "rb")) == NULL) {
            log_print ("[teteco_net]: Error opening file: %s\n", teteco->file);
            file_transfer_callback (teteco->file, TETECO_FILE_TRANSFER_END, 0, 0);
			return;
        }

        struct stat st;
        stat (teteco->file, &st);
        file_size = st.st_size;

        log_print ("[teteco_net]: File size is: %u\n", file_size);

        if ( -1 == send (teteco->sd_tcp, (void*)&file_size, sizeof(uint32_t), 0)) {
            log_print ("[teteco_net]: Error sending file size: %s\n", strerror(errno));
            closesocket (teteco->sd_tcp);
            teteco->sd_tcp = 0;
            fclose (teteco->fd);
            teteco->fd = NULL;
            file_transfer_callback (teteco->file, TETECO_FILE_TRANSFER_END, 0, 0);
        }

        if ( -1 == send (teteco->sd_tcp, (void*)&file_name_size, sizeof(file_name_size), 0)) {
            log_print ("[teteco_net]: Error sending file name size\n");
            closesocket (teteco->sd_tcp);
            teteco->sd_tcp = 0;
            fclose (teteco->fd);
            teteco->fd = NULL;
            file_transfer_callback (teteco->file, TETECO_FILE_TRANSFER_END, 0, 0);
        }

        log_print ("[teteco_net]: Sendinf file name: %s\n", file_name);

        if ( -1 == send (teteco->sd_tcp, (void*)file_name, strlen (file_name), 0)) {
            log_print ("[teteco_net]: Error sending file name\n");
            closesocket (teteco->sd_tcp);
            teteco->sd_tcp = 0;
            fclose (teteco->fd);
            teteco->fd = NULL;
            file_transfer_callback (teteco->file, TETECO_FILE_TRANSFER_END, 0, 0);
        }

    }


    int read = fread (buffer, 1, chunk_size, teteco->fd);
    int written = 0;

    while (written < read) {
        written += send (teteco->sd_tcp, (void*)buffer, read, 0);
        if (written < 0 ) break;
    }

    sent_so_far += written;

    //log_print ("[teteco_net]: Sent %d of %d\n", written, read);

    if (written == -1 ) {
        log_print ("[teteco_net]: Error sending file\n");
        closesocket (teteco->sd_tcp);
        teteco->sd_tcp = 0;
        fclose (teteco->fd);
        teteco->fd = NULL;
        file_transfer_callback (teteco->file, TETECO_FILE_TRANSFER_END, 0, 0);
    }

    gettimeofday (&current_time, NULL);
    timersub (&current_time, &start_time, &total_time);

    teteco->transfer_rate = (float) sent_so_far / (float)total_time.tv_sec + ((float)total_time.tv_usec*0.0000001F);

    // Bandwidth control
    if (teteco->max_transfer_rate != 0) {

        float sleep_time_tmp = sleep_time.tv_sec*1000000 + sleep_time.tv_usec;

        if (teteco->transfer_rate > teteco->max_transfer_rate) {
            sleep_time_tmp = sleep_time_tmp * 1.02;
        }
        else if (teteco->transfer_rate < teteco->max_transfer_rate) {
            sleep_time_tmp = sleep_time_tmp * 0.98;
        }

        sleep_time.tv_sec  = sleep_time_tmp / 1000000;
        sleep_time.tv_usec = (uint64_t)sleep_time_tmp % 1000000;

        log_print ("Sleep time is %u.%06u\n", sleep_time.tv_sec, sleep_time.tv_usec);
        log_print ("Current transfer rate = %u\n", teteco->transfer_rate);

        chunk_size = teteco->max_transfer_rate / 20;

    }
    else {
        chunk_size = 10240;
    }

    if (!feof (teteco->fd)) {
        file_transfer_callback (teteco->file, TETECO_FILE_TRANSFER_SENDING, file_size, sent_so_far);

        if (teteco->max_transfer_rate != 0) {
            event_add (teteco->tcp_send_event, &sleep_time);
        }
        else {
            event_add (teteco->tcp_send_event, NULL);
        }
    }
    else {
        log_print ("File completly sent\n");
        file_transfer_callback (teteco->file, TETECO_FILE_TRANSFER_END, file_size, sent_so_far);
        closesocket (teteco->sd_tcp);
        teteco->sd_tcp = 0;
        fclose (teteco->fd);
        teteco->fd = NULL;
        free (teteco->file);
        teteco->file = NULL;
    }
}



void teteco_net_file_recv_callback (int sd, short event, void *teteco_ref) {

    static struct timeval   start_time;
    struct timeval          current_time;
    struct timeval          total_time;

    struct sockaddr_in remote_address_tcp_in;

    socklen_t address_in_len = sizeof (struct sockaddr_in);

    teteco_t* teteco = (teteco_t*) teteco_ref;

    // Connection establishement

    if (teteco->sd_tcp != sd) {

        log_print ("[teteco_net]: Connection establishement\n");

        event_free (teteco->tcp_read_event);
        teteco->tcp_read_event = NULL;

        teteco->sd_tcp = accept (sd, (struct sockaddr*)&remote_address_tcp_in, &address_in_len);

        if (0 > teteco->sd_tcp) {
            log_print ("[teteco_net]: Error on listen: %m");
            closesocket (sd);
            teteco->sd_tcp = 0;
            return;
        }
        closesocket (sd);

        teteco->tcp_read_event = event_new (event_base, teteco->sd_tcp, EV_READ|EV_PERSIST, teteco_net_file_recv_callback, teteco);
        event_add (teteco->tcp_read_event, NULL);

        return;

    }

    // Read 

    static int      received = 0;
    unsigned int    buffer_size = 10240;
    int8_t          buffer[buffer_size];
    uint16_t        file_name_size = 0;
    static uint32_t file_size      = 0;
    static char*    filename       = NULL;

    // Read size and file name

    if (teteco->fd == NULL) {

        received = 0;
        file_size = 0;

        if (recv (sd, (void*)&file_size, sizeof (file_size), 0) < sizeof (file_size)) {
            log_print ("[teteco_net]: Error on read (file size): %m\n");
            event_del  (teteco->tcp_read_event);
            event_free (teteco->tcp_read_event);
            teteco->tcp_read_event = NULL;
            closesocket (sd);
            file_transfer_callback ("", TETECO_FILE_TRANSFER_END, 0, 0);
        }

        log_print ("[teteco_net]: Got file size: %u\n", file_size);

        if (recv (sd, (void*)&file_name_size, sizeof (file_name_size), 0) < sizeof (file_name_size)) {
            log_print ("[teteco_net]: Error on read (file name size): %m\n");
            event_del  (teteco->tcp_read_event);
            event_free (teteco->tcp_read_event);
            teteco->tcp_read_event = NULL;
            closesocket (sd);
            file_transfer_callback ("", TETECO_FILE_TRANSFER_END, 0, 0);
        }

        log_print ("[teteco_net]: Got file name size: %u\n", file_name_size);

        char file_name[file_name_size+1];
        file_name[file_name_size] = '\0';

        if (recv (sd, (void*)&file_name, file_name_size, 0) < file_name_size) {
            log_print ("[teteco_net]: Error on read (file name): %m\n");
            if (teteco->tcp_read_event != NULL) {
                event_del  (teteco->tcp_read_event);
                event_free (teteco->tcp_read_event);
                teteco->tcp_read_event = NULL;
            }
            closesocket (sd);
            file_transfer_callback ("", TETECO_FILE_TRANSFER_END, 0, 0);
        }

        char* file_path = NULL;

        #ifdef __WINDOWS__
        asprintf (&file_path, "%s\\%s", teteco->file_dir, file_name);
        #else
        asprintf (&file_path, "%s/%s", teteco->file_dir, file_name);
        #endif

        filename = strdup (file_path);

        log_print ("[teteco_net]: Receiving file: %s\n", file_path);

        if ((teteco->fd = fopen (file_path, "wb")) == NULL) {
            log_print ("[teteco_net]: Error opening file: %s\n", file_path);
			if (teteco->tcp_read_event != NULL) {
                event_del  (teteco->tcp_read_event);
                event_free (teteco->tcp_read_event);
                teteco->tcp_read_event = NULL;
            }
            closesocket (sd);
            free (file_path);
            file_transfer_callback ("", TETECO_FILE_TRANSFER_END, 0, 0);
        }
        free (file_path);


        gettimeofday (&start_time, NULL);

    }

    // Read file payload

    int readed = 0;

    readed = recv (sd, (void*)buffer, buffer_size, 0);
    fwrite (buffer, 1, readed, teteco->fd);
    received += readed;

    gettimeofday (&current_time, NULL);
    timersub (&current_time, &start_time, &total_time);

    teteco->transfer_rate = (float) received / (float)total_time.tv_sec + ((float)total_time.tv_usec*0.0000001F);

    if (readed < 0) {
        log_print ("[teteco_net]: Error on read: %m\n");
        event_del  (teteco->tcp_read_event);
        event_free (teteco->tcp_read_event);
        teteco->tcp_read_event = NULL;
        closesocket (sd);
        fclose (teteco->fd);
        teteco->fd = NULL;
    }
    else if (readed == 0) {
        log_print ("[teteco_net]: TCP socket closed\n");
        event_del  (teteco->tcp_read_event);
        event_free (teteco->tcp_read_event);
        teteco->tcp_read_event = NULL;
        closesocket (sd);
        fclose (teteco->fd);
        teteco->fd = NULL;
        file_transfer_callback (filename, TETECO_FILE_TRANSFER_END, file_size, received);
        free (filename);
        filename = NULL;
    }
    else {
        file_transfer_callback (filename, TETECO_FILE_TRANSFER_RECEIVING, file_size, received);
        event_add (teteco->tcp_read_event, NULL);
    }

}