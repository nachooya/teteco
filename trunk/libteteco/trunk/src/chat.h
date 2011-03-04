/**
 * @file    chat.h
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

#ifndef __CHAT_H__
#define __CHAT_H__

#ifdef __WINDOWS__

#include "sys/queue.h"

#else

#include <sys/queue.h>

#endif

#include <stdint.h>

typedef struct {

    TAILQ_HEAD (chat_queue_t,     chat_entry_t) chat_queue;
    TAILQ_HEAD (chat_ack_queue_t, chat_ack_t)   chat_ack_queue;

    uint16_t    comment_seq;
    uint16_t    last_recv_ack;

} chat_data_t;

typedef struct chat_entry_t {

    TAILQ_ENTRY (chat_entry_t) chat_entries;
    uint16_t    comment_number;
    char*       comment;
    uint16_t    size;

} chat_entry_t;

typedef struct chat_ack_t {

    TAILQ_ENTRY (chat_ack_t) chat_acks;
    uint16_t    ack_num;

} chat_ack_t;




chat_data_t*    chat_start      (void);
chat_data_t*    chat_stop       (chat_data_t* chat_data);
int             chat_add        (chat_data_t* chat_data, const char* comment);
chat_entry_t*   chat_get        (chat_data_t* chat_data);
int             chat_ack        (chat_data_t* chat_data, uint16_t seq);
int             chat_ack_add    (chat_data_t* chat_data, uint16_t ack_num);
chat_ack_t*     chat_ack_get    (chat_data_t* chat_data);

#endif