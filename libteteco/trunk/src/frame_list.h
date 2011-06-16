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

typedef void(*frame_list_callback_ft) (int, void*);

typedef struct {

    void*    payload;
    uint8_t  size;
    int      timestamp;

} frame_t;

typedef struct {

    frame_t*  frames;
    uint16_t  size;
    uint16_t  num_frames;
    uint16_t  read_cursor;
    uint16_t  write_cursor;

    frame_list_callback_ft callback;
    uint16_t               callback_size;
    void*                  callback_arg;

} frame_list_t;



frame_list_t* frame_list_new            (uint16_t num_frames);
void*         frame_list_free           (frame_list_t* frame_list);
int           frame_list_add_frame      (frame_list_t* frame_list, uint8_t size, int timestamp, void* payload);
int           frame_list_get_frames_raw (frame_list_t* frame_list, uint16_t num_frames, void* output);
int           frame_list_set_callback   (frame_list_t* frame_list, int num_frames, frame_list_callback_ft callback, void* callback_arg);

#endif