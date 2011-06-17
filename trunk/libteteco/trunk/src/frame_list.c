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

#ifdef __WINDOWS__
    #include <fcntl.h>
    #include <io.h>
    #define pipe(f) _pipe(f, 1000, _O_BINARY)
#else
    #include <unistd.h>
#endif

#include "log.h"
#include "frame_list.h"

frame_list_t* frame_list_new (uint16_t num_frames, uint16_t frame_size, uint16_t notify_frames) {

    log_print ("[frame_list]: num_frames: %d, frame_size. %d, notify_frames: %d", num_frames, frame_size, notify_frames);

    frame_list_t* frame_list  = malloc (sizeof(frame_list_t));

    frame_list->frames        = malloc (sizeof(frame_t)*num_frames);
    frame_list->size          = num_frames;
    frame_list->read_cursor   = 0;
    frame_list->write_cursor  = 0;
    frame_list->notify_frames = notify_frames;
    frame_list->notify_flag   = 0;

    pipe (frame_list->pipe_fds);

    int i;
    for (i = 0; i < num_frames; i++) {
        frame_list->frames[i].payload = malloc (frame_size*2);
    }

    return frame_list;

}

void* frame_list_free (frame_list_t* frame_list) {

    if (frame_list == NULL) return NULL;

    close (frame_list->pipe_fds[0]);
    close (frame_list->pipe_fds[1]);

    int i;
    for (i = 0; i < frame_list->size; i++) {
        free (frame_list->frames[i].payload);
    }

    free (frame_list->frames);
    free (frame_list);

    return NULL;

}

int frame_list_add_frame (frame_list_t* frame_list, uint8_t size, int16_t* payload) {

    //log_print ("[frame_list]: Adding frame size: %d\n", size);

    int pos = frame_list->write_cursor % frame_list->size;

    if (size == 0) {
        frame_list->frames[pos].payload = NULL;
    }
    else {
        memcpy (frame_list->frames[pos].payload, payload, size*sizeof(int16_t));
    }
    frame_list->frames[pos].dirty     = false;
    frame_list->frames[pos].size      = size;

    frame_list->write_cursor++;

    int num_frames = frame_list->write_cursor - frame_list->read_cursor;

    frame_list->notify_flag = (frame_list->notify_flag + 1) % frame_list->notify_frames;

    if (frame_list->notify_flag == 0) {
        write (frame_list->pipe_fds[1],  &num_frames, sizeof(uint16_t));
    }

    return 1;

}

uint8_t frame_list_get_frame (frame_list_t* frame_list, int16_t* output) {

    uint8_t total_size = 0;

    int num_frames = frame_list->write_cursor - frame_list->read_cursor;

    if (num_frames > frame_list->size) {
        log_print ("[frame_list]: Buffer OVERFLOW!!!\n");
    }

    int pos = frame_list->read_cursor % frame_list->size;

    if (num_frames > 0 && !frame_list->frames[pos].dirty) {

        memcpy (frame_list->frames[pos].payload, output, frame_list->frames[pos].size);
        total_size = frame_list->frames[pos].size;
        frame_list->frames[pos].dirty = true;

        frame_list->read_cursor++;
    }

    //log_print ("[frame_list]: get frames (exit) -> get=%d left=%d", num_frames, frame_list->num_frames);

    return total_size;
}

int frame_list_get_read_pipe_fd (frame_list_t* frame_list) {

    return frame_list->pipe_fds[0];

}
