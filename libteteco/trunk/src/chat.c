
/**
 * @file    chat.c
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

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "log.h"
#include "util.h"
#include "chat.h"

chat_data_t* chat_start (void) {

    chat_data_t* chat_data = util_malloc (sizeof(chat_data_t));

    chat_data->chat_list.head = NULL;
    chat_data->chat_list.tail  = NULL;
    chat_data->chat_ack_list.head = NULL;
    chat_data->chat_ack_list.tail  = NULL;

    chat_data->comment_seq   = 1;
    chat_data->last_recv_seq = 0;
    chat_data->last_recv_ack = 0;
    chat_data->last_send_seq = 0;

    return chat_data;
}

chat_data_t* chat_stop (chat_data_t* chat_data) {

    if (chat_data == NULL) return NULL;

    chat_node_t* chat_node = chat_data->chat_list.head;

    while (chat_node != NULL) {
        chat_data->chat_list.head = chat_node->prev_entry;
        util_free (chat_node->entry);
        util_free (chat_node);
        chat_node = chat_data->chat_list.head;
    }

    chat_ack_node_t* chat_ack_node = chat_data->chat_ack_list.head;

    while (chat_ack_node != NULL) {
        chat_data->chat_ack_list.head = chat_ack_node->prev_ack;
        util_free (chat_ack_node);
        chat_ack_node = chat_data->chat_ack_list.head;
    }

    util_free (chat_data);

    chat_data = NULL;

    return chat_data;
}

int chat_add (chat_data_t* chat_data, const char* comment) {

    if (chat_data == NULL || comment == NULL) {
        return 0;
    }

    unsigned int bytes_to_write = strlen (comment);
    char* next_to_write = (char*)comment;

    // Look for space on previous entry
    // Then write the rest splitted in CHAT_MAX_LENGTH

    #define CHAT_MAX_LENGTH 100

    while (bytes_to_write > 0) {

        unsigned now_write = bytes_to_write > CHAT_MAX_LENGTH ? CHAT_MAX_LENGTH : bytes_to_write;

        chat_node_t* chat_node = util_calloc (1, sizeof(chat_node_t));

        chat_node->comment_number = chat_data->comment_seq++;

        chat_node->entry = util_calloc (now_write, sizeof(char));

        memcpy (chat_node->entry, next_to_write, now_write);
        chat_node->size = now_write;
        chat_node->prev_entry = NULL;

        bytes_to_write -= now_write;
        next_to_write  += now_write;

        if (chat_data->chat_list.head == NULL) {
            chat_data->chat_list.head = chat_node;
        }
        else {
            ((chat_node_t*)(chat_data->chat_list.tail))->prev_entry = chat_node;
        }
        chat_data->chat_list.tail = chat_node;

    }

    return 1;

}

chat_node_t* chat_get (chat_data_t* chat_data) {

    chat_node_t* chat_node = chat_data->chat_list.head;

    if (chat_node == NULL) {
        return NULL;
    }
    else if (chat_node->comment_number > chat_data->last_send_seq) {
//         log_print ("[chat]: get seq: %d", chat_node->comment_number);
//         log_print ("[chat]: get last: %d", chat_data->last_send_seq);
        chat_data->last_send_seq = chat_node->comment_number;
        gettimeofday (&chat_data->last_send_time, NULL);
        return chat_node;
    }
    else {
        struct timeval now;
        struct timeval interval;
        gettimeofday (&now, NULL);
        timersub (&now, &chat_data->last_send_time, &interval);

//         log_print ("[chat]: interval: %d.%06d", interval.tv_sec, interval.tv_usec);

        if (interval.tv_sec >= 1 || interval.tv_usec > 500000) {
            gettimeofday (&chat_data->last_send_time, NULL);
            return chat_node;
        }
        else {
            return NULL;
        }
    }

}

int chat_ack (chat_data_t* chat_data, uint16_t seq) {

    chat_node_t* chat_node = chat_data->chat_list.head;

    if (chat_node != NULL) {
        chat_data->chat_list.head = chat_node->prev_entry;
        if (chat_data->chat_list.head == NULL) chat_data->chat_list.tail = NULL;
        //chat_data->last_recv_ack = seq;
        free (chat_node->entry);
        free (chat_node);
        return 1;
    }

    log_print ("[chat]: Extrange ack for unexistang chat entry");

    return 0;

}


int chat_ack_add (chat_data_t* chat_data, uint16_t ack_num) {

    if (chat_data == NULL) {
        return 0;
    }
//     else if (ack_num <= chat_data->last_recv_ack) {
//         return 0;
//     }

//     log_print ("[chat]: Adding ack: %d", ack_num);

    chat_ack_node_t* chat_ack_node = util_malloc (sizeof(chat_ack_node_t));

    chat_ack_node->ack_num = ack_num;
    chat_ack_node->prev_ack = NULL;

    if (chat_data->chat_ack_list.head == NULL) {
//         log_print ("[chat]: chat_ack was void");
        chat_data->chat_ack_list.head = chat_ack_node;
        chat_data->chat_ack_list.tail = chat_ack_node;
    }
    else {
//         log_print ("[chat]: chat_ack was NO void");
        ((chat_ack_node_t*)(chat_data->chat_ack_list.tail))->prev_ack = chat_ack_node;
    }
    chat_data->chat_ack_list.tail = chat_ack_node;

    return 1;

}

int32_t chat_ack_get (chat_data_t* chat_data) {

    int32_t result = -1;

    chat_ack_node_t* chat_ack_node = chat_data->chat_ack_list.head;

    //log_print ("[chat]: chat_ack_get: %d", (chat_ack_node==NULL)?-555:chat_ack_node->ack_num);

    if (chat_ack_node != NULL) {
        chat_data->chat_ack_list.head = chat_ack_node->prev_ack;
        if (chat_data->chat_ack_list.head == NULL) chat_data->chat_ack_list.tail = NULL;
        result=chat_ack_node->ack_num;
        free (chat_ack_node);
    }

    return result;

}
