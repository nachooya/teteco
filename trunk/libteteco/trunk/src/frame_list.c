/**
 * @file    frame_list.c
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
#include "frame_list.h"

frame_list_t* frame_list_new (uint16_t num_frames) {

    frame_list_t* frame_list = malloc (sizeof(frame_list_t));

    frame_list->frames        = malloc (sizeof(frame_t)*num_frames);
    frame_list->size          = num_frames;
    frame_list->num_frames    = 0;
    frame_list->read_cursor   = 0;
    frame_list->write_cursor  = 0;
    frame_list->callback      = NULL;
    frame_list->callback_size = 0;
    frame_list->callback_arg  = NULL;

    return frame_list;

}

void* frame_list_free (frame_list_t* frame_list) {

    free (frame_list->frames);
    free (frame_list);

    return NULL;

}

int frame_list_add_frame (frame_list_t* frame_list, uint8_t size, int timestamp, void* payload) {

    if (size == 0) return 1;

    frame_list->frames[frame_list->write_cursor].payload   = malloc (size);
    memcpy (frame_list->frames[frame_list->write_cursor].payload, payload, size);
    frame_list->frames[frame_list->write_cursor].size      = size;
    frame_list->frames[frame_list->write_cursor].timestamp = timestamp;

    frame_list->num_frames =   (frame_list->num_frames   + 1 ) % frame_list->size;
    frame_list->write_cursor = (frame_list->write_cursor + 1 ) % frame_list->size;

    if (frame_list->callback != NULL && frame_list->num_frames >= frame_list->callback_size) {
        (frame_list->callback) (frame_list->num_frames, frame_list->callback_arg);
    }

    return 1;

}

int frame_list_get_frames_raw (frame_list_t* frame_list, uint16_t num_frames, void* output) {

    int i;
    int total_size = 0;

    log_print ("[frame_list]: get frames (start) -> get=%d left=%d", num_frames, frame_list->num_frames);

    for (i=0; i < num_frames && frame_list->num_frames > 0; i++) {


        memcpy (output+total_size, &frame_list->frames[frame_list->read_cursor].timestamp, sizeof(int));
        total_size += sizeof (int);

        memcpy (output+total_size, &frame_list->frames[frame_list->read_cursor].size, sizeof (uint8_t));
        total_size += sizeof (uint8_t);

        memcpy (output+total_size,
                frame_list->frames[frame_list->read_cursor].payload,
                frame_list->frames[frame_list->read_cursor].size);
        total_size += frame_list->frames[frame_list->read_cursor].size;

        free (frame_list->frames[frame_list->read_cursor].payload);

        frame_list->num_frames--;
        frame_list->read_cursor = (frame_list->read_cursor + 1 ) % frame_list->size;
    }

    log_print ("[frame_list]: get frames (exit) -> get=%d left=%d", num_frames, frame_list->num_frames);

    return total_size;
}

int frame_list_set_callback (frame_list_t* frame_list, int num_frames, frame_list_callback_ft callback, void* callback_arg) {

    frame_list->callback_size = num_frames;
    frame_list->callback      = callback;
    frame_list->callback_arg  = callback_arg;

    return 1;

}
