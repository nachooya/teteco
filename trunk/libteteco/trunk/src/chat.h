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

#include <stdint.h>

typedef struct chat_node {

	char*       entry;
    char*       comment;
    uint16_t    size;
	uint16_t    comment_number;
	struct chat_node* next_entry;

} chat_node_t;

typedef struct chat_ack_node {
	uint16_t	          ack_num;
	struct chat_ack_node* next_ack;
} chat_ack_node_t;

typedef struct {

	void* first;
	void* last;
	
} linked_list_t;

typedef struct {

	linked_list_t  chat_list;
	linked_list_t  chat_ack_list;

    uint16_t       comment_seq;
    uint16_t       last_recv_ack;

} chat_data_t;


chat_data_t*     chat_start      (void);
chat_data_t*     chat_stop       (chat_data_t* chat_data);
int              chat_add        (chat_data_t* chat_data, const char* comment);
chat_node_t*     chat_get        (chat_data_t* chat_data);
int              chat_ack        (chat_data_t* chat_data, uint16_t seq);
int              chat_ack_add    (chat_data_t* chat_data, uint16_t ack_num);
chat_ack_node_t* chat_ack_get    (chat_data_t* chat_data);

#endif