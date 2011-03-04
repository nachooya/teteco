
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

#include "log.h"
#include "util.h"
#include "chat.h"

chat_data_t* chat_start (void) {

    chat_data_t* chat_data = util_malloc (sizeof(chat_data_t));

    TAILQ_INIT (&chat_data->chat_queue);
    TAILQ_INIT (&chat_data->chat_ack_queue);

    chat_data->comment_seq   = 0;
    chat_data->last_recv_ack = 0;

    return chat_data;
}

chat_data_t* chat_stop (chat_data_t* chat_data) {

    chat_entry_t* chat_entry = chat_data->chat_queue.tqh_first;

    while (chat_entry != NULL) {
        TAILQ_REMOVE(&chat_data->chat_queue, chat_entry, chat_entries);
        util_free (chat_entry->comment);
        util_free (chat_entry);
        chat_entry = chat_data->chat_queue.tqh_first;
    }

    chat_ack_t* chat_ack = chat_data->chat_ack_queue.tqh_first;

    while (chat_ack != NULL) {
        TAILQ_REMOVE(&chat_data->chat_ack_queue, chat_ack, chat_acks);
        util_free (chat_ack);
        chat_ack = chat_data->chat_ack_queue.tqh_first;
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

        chat_entry_t* chat_entry = util_calloc (1, sizeof(chat_entry_t));

        chat_entry->comment_number = chat_data->comment_seq++;

        chat_entry->comment = util_calloc (now_write, sizeof(char));

        memcpy (chat_entry->comment, next_to_write, now_write);
        chat_entry->size = now_write;

        bytes_to_write -= now_write;
        next_to_write += now_write;

        TAILQ_INSERT_TAIL (&chat_data->chat_queue, chat_entry, chat_entries);
    }

    return 1;

}

chat_entry_t* chat_get (chat_data_t* chat_data) {

    chat_entry_t* chat_entry = chat_data->chat_queue.tqh_first;

    return chat_entry;

}

int chat_ack (chat_data_t* chat_data, uint16_t seq) {

    chat_entry_t* chat_entry = chat_data->chat_queue.tqh_first;

    if (chat_entry != NULL) {
        TAILQ_REMOVE(&chat_data->chat_queue, chat_entry, chat_entries);
        chat_data->last_recv_ack = seq;
        return 1;
    }

    log_print ("[chat]: Extrange ack for unexistang chat entry");

    return 0;

}


int chat_ack_add (chat_data_t* chat_data, uint16_t ack_num) {

    if (chat_data == NULL) {
        return 0;
    }

    chat_ack_t* chat_ack = util_malloc (sizeof(chat_ack_t));

    chat_ack->ack_num = ack_num;

    TAILQ_INSERT_TAIL (&chat_data->chat_ack_queue, chat_ack, chat_acks);

    return 1;

}

chat_ack_t* chat_ack_get (chat_data_t* chat_data) {

    chat_ack_t* chat_ack = chat_data->chat_ack_queue.tqh_first;

    if (chat_ack != NULL) {
        TAILQ_REMOVE(&chat_data->chat_ack_queue, chat_ack, chat_acks);
    }

    return chat_ack;

}
