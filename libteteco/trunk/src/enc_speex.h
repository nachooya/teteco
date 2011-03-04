/**
 * @file    enc_speex.h
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


#ifndef __ENC_SPEEX_H__
#define __ENC_SPEEX_H__

#include <stdint.h>
#include <speex/speex.h>
#include <speex/speex_preprocess.h>
#include "speex_jitter_buffer.h"
#include "circular_buffer.h"

typedef enum {
    ENC_SPEEX_MODE_ENCODER,
    ENC_SPEEX_MODE_DECODER
} enc_speex_mode_t;

typedef enum {
    ENC_SPEEX_NB,
    ENC_SPEEX_WB,
    ENC_SPEEX_UWB
} enc_speex_band_t;

typedef struct {
    enc_speex_mode_t     mode;
    void                 *state;
    SpeexPreprocessState *preprocess_state;
    SpeexJitter          jitter;
    /*Holds bits so they can be read and written to by the Speex routines*/
    SpeexBits            bits;
    unsigned int         frame_size;
    uint8_t              frame_number;
    int                  timestamp;
    uint32_t             sample_rate;
    uint32_t             bitrate;
    uint32_t             encoded_frame_size;

    circular_buffer_t    *buffer;

} enc_speex_status_t;


enc_speex_status_t* enc_speex_start      (enc_speex_mode_t mode, enc_speex_band_t band_par, int quality);
int                 enc_speex_stop       (enc_speex_status_t* status);
int                 enc_speex_encode     (enc_speex_status_t* status, int16_t *frame);
int                 enc_speex_put_sample (enc_speex_status_t* status, int8_t* samples, int samples_size);
int                 enc_speex_decode     (enc_speex_status_t* status, int16_t *frame);

#endif