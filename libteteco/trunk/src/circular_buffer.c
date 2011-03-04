


/**
 * @file    circular_buffer.c
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
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "log.h"
#include "util.h"
#include "circular_buffer.h"

circular_buffer_t* circular_buffer_new (unsigned int size, unsigned int type_size) {

    circular_buffer_t* buffer;

    log_print ("[cb]: Initializing with %u*%u=%u bytes", size, type_size);

    buffer                  = util_calloc (1, sizeof (circular_buffer_t));
    memset (buffer, 0, sizeof (circular_buffer_t));
    buffer->type_size       = type_size;
    buffer->size            = size*type_size;
    buffer->n_read          = 0;
    buffer->n_written       = 0;
    buffer->cursor_read     = 0;
    buffer->cursor_write    = 0;
    buffer->data            = util_calloc (size, type_size);

    return buffer;

}

void* circular_buffer_free (circular_buffer_t* buffer) {

    if (buffer == NULL) return NULL;

    util_free (buffer->data);
    util_free (buffer);
    return NULL;
}

void circular_buffer_print (circular_buffer_t* buffer) {

    printf ("\n");
    printf ("Type size   : %u\n", buffer->type_size);
    printf ("Size        : %u\n", buffer->size);
    printf ("n_read      : %u\n", buffer->n_read);
    printf ("n_written   : %u\n", buffer->n_written);
    printf ("cursor_read : %u\n", buffer->cursor_read);
    printf ("cursor_write: %u\n", buffer->cursor_write);
    printf ("ready       : %u\n", buffer->n_written - buffer->n_read);
/*    int i;

    if (buffer->type_size == 1) {
        char* buf = (char*) buffer->data;
        for (i=0; i<buffer->size; i++) {
            log_print ("[%02d] ", buf[i]);
        }
    }
    else if (buffer->type_size == 2) {
        short int* buf = (short int*) buffer->data;
        for (i=0; i<buffer->size; i++) {
            log_print ("[%02d] ", buf[i]);
        }
    }
    else {
        log_print ("Not implemented\n");
    }
*/



}

unsigned int circular_buffer_nbytes (circular_buffer_t* buffer) {

    if (buffer->n_written >= buffer->n_read)
        return buffer->n_written - buffer->n_read;
    else
        return buffer->size + (buffer->n_written - buffer->n_read);

}

void circular_buffer_write (circular_buffer_t* buffer, void* input, unsigned int size) {

    if (size*buffer->type_size <= (buffer->size - buffer->cursor_write)) {
        memcpy (buffer->data+buffer->cursor_write, input, size * buffer->type_size);
    }
    else {
        memcpy (buffer->data+buffer->cursor_write,
                input,
                (buffer->size - buffer->cursor_write));

        memcpy (buffer->data,
                input+(buffer->size - buffer->cursor_write),
                size*buffer->type_size - (buffer->size - buffer->cursor_write));
    }
    buffer->cursor_write=(buffer->cursor_write+(size*buffer->type_size))%buffer->size;
    buffer->n_written+=size;

}

void circular_buffer_write_value (circular_buffer_t* buffer, void* input, unsigned int size) {

    int i;
    for (i=0; i < size; i++) {
        memcpy (buffer->data+buffer->cursor_write+(i*buffer->type_size), input, buffer->type_size);
        buffer->cursor_write=(buffer->cursor_write+(1*buffer->type_size))%buffer->size;
        buffer->n_written++;
    }

}

unsigned int circular_buffer_read (circular_buffer_t* buffer, void* output, unsigned int size) {

    if (buffer->cursor_write < buffer->cursor_read) {
        buffer->cursor_read = buffer->cursor_read % UINT_MAX;
    }

    if ((buffer->n_written - buffer->n_read) > buffer->size) {
        int times_over  = ((buffer->n_written - buffer->n_read) / buffer->size)-1;
        int over        = (buffer->n_written - buffer->n_read) % buffer->size;
        buffer->n_read += (times_over*buffer->size) + over;
        buffer->cursor_read = buffer->n_read % buffer->size;
    }

    int i;
    for (i=0; i < size && buffer->n_read < buffer->n_written; i++) {
        memcpy (output+(i*buffer->type_size), buffer->data+buffer->cursor_read, buffer->type_size);
        buffer->cursor_read=(buffer->cursor_read+(1*buffer->type_size))%buffer->size;
        buffer->n_read++;
    }
    return i;
}