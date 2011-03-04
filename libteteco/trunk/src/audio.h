/**
 * @file    audio.h
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

#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdint.h>
#include <portaudio.h>

typedef enum {
    AUDIO_DEV_IN,
    AUDIO_DEV_OUT
} audio_device_type_t;

//  numframes, buffer
typedef int (*get_frame_func) (int, int16_t*, void*);

typedef struct {

    double              sample_rate;
    unsigned long       framesPerBuffer;
    PaStream            *stream;
    void                *data;

    get_frame_func      frame_callback;


} audio_data_t;

audio_data_t*   audio_init            (double sample_rate, unsigned long framesPerBuffer, void* get_frame_par, void* user_data);
int             audio_end             (audio_data_t* audio_data);
int             audio_get_devices     (audio_device_type_t in_out, int **index, char ***devices);
int             audio_record          (int device, audio_data_t* audio_data);
int             audio_play            (int device, audio_data_t* audio_data);
int             audio_stop            (audio_data_t* audio_data);


#endif
