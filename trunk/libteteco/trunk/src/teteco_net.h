/**
 * @file    teteco_net.h
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


#ifndef _TETECO_NET_H_
#define _TETECO_NET_H_

#define MAX_UDP_SIZE 65507

#ifdef __WINDOWS__

#include <winsock2.h>
#include <ws2tcpip.h>

#else

#include <arpa/inet.h>

#endif

#include "teteco.priv.h"

int teteco_net_start     (teteco_t* teteco, int local_port, int remote_port, const char* remote_address);
int teteco_net_stop      (teteco_t* teteco);
int teteco_net_send      (teteco_t* teteco, char* buffer, int len);
int teteco_net_receive   (teteco_t* teteco, char* buffer, int len);

int teteco_net_file_connect        (teteco_t* teteco, int remote_port);
int teteco_net_file_listen         (teteco_t* teteco, int *local_port);

void teteco_udp_send_callback      (int sd, short event, void *teteco_ref);
void teteco_udp_recv_callback      (int sd, short event, void *teteco_ref);
void teteco_net_file_recv_callback (int sd, short event, void *teteco_ref);
void teteco_net_file_send_callback (int sd, short event, void *teteco_ref);


//TODO investigate connect

#endif

