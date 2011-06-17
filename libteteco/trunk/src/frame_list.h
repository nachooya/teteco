/**
 * @file    frame_list.h
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

#ifndef __FRAME_LIST_H__
#define __FRAME_LIST_H__

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

typedef void(*frame_list_callback_ft) (int, void*);

typedef struct {

    void*    payload;
    uint8_t  size;
    bool     dirty;

} frame_t;

typedef struct {

    frame_t*  frames;
    uint16_t  size;
    uint32_t  read_cursor;
    uint32_t  write_cursor;
    uint16_t  notify_frames;
    uint16_t  notify_flag;
    int       pipe_fds[2];

} frame_list_t;



frame_list_t* frame_list_new              (uint16_t num_frames, uint16_t frame_size, uint16_t notify_frames);
int           frame_list_get_read_pipe_fd (frame_list_t* frame_list);
void*         frame_list_free             (frame_list_t* frame_list);
int           frame_list_add_frame        (frame_list_t* frame_list, uint8_t size, int16_t* payload);
uint8_t       frame_list_get_frame        (frame_list_t* frame_list, int16_t* output);

#endif
