/**
 * @file    enc_speex.c
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


#include <stdio.h>
#include <math.h>
#include <string.h>

#include "log.h"
#include "util.h"
#include "enc_speex.h"

enc_speex_status_t* enc_speex_start (enc_speex_mode_t mode, enc_speex_band_t band_par, int quality) {

    enc_speex_status_t* status = util_malloc (sizeof(enc_speex_status_t));

    memset (status, 0, sizeof (enc_speex_status_t));

    const SpeexMode *band;
    status->mode = mode;

    if (status == NULL) {
        log_print ("[speex]: Error in malloc");
        return NULL;
    }

    if (band_par == ENC_SPEEX_NB) {
        band = &speex_nb_mode;
    }
    else if (band_par == ENC_SPEEX_WB) {
        band = &speex_wb_mode;
    }
    else if (band_par == ENC_SPEEX_UWB) {
        band = &speex_uwb_mode;
    }
    else {
        log_print ("[speex]: Bad band");
        util_free (status);
        return NULL;
    }

    if (mode == ENC_SPEEX_MODE_ENCODER) {
        /*Create a new encoder state in narrowband mode*/
        status->state = speex_encoder_init (band);
        if (status->state == NULL) {
            log_print ("[speex]: Error initializing encoder");
            util_free (status);
            return NULL;
        }
        /*Set the quality to 8 (15 kbps)*/
        int tmp=quality;
        if (speex_encoder_ctl (status->state, SPEEX_SET_QUALITY, &tmp) != 0) {
            log_print ("[speex]: Error setting quality");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        if (speex_encoder_ctl (status->state, SPEEX_GET_FRAME_SIZE, &status->frame_size) != 0) {
            log_print ("[speex]: Error setting getting frame size");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        if (speex_encoder_ctl(status->state, SPEEX_GET_BITRATE, &status->bitrate) != 0) {
            log_print ("[speex]: Error setting getting bitrate");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        if (speex_encoder_ctl(status->state, SPEEX_GET_SAMPLING_RATE, &status->sample_rate) != 0) {
            log_print ("[speex]: Error setting getting sample rate");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        tmp = 1;
        if (speex_encoder_ctl(status->state, SPEEX_SET_VAD, &tmp) != 0) {
            log_print ("[speex]: Error setting VAD");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }

        if (speex_encoder_ctl(status->state, SPEEX_SET_DTX, &tmp) != 0) {
            log_print ("[speex]: Error setting DTX");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
//         tmp = 5
//         if (speex_encoder_ctl(status->state, SPEEX_SET_PLC_TUNING, tmp) != 0) {
//             log_print ("[speex]: Error setting getting sample rate\n");
//             speex_encoder_destroy (status->state);
//             util_free (status);
//             return NULL;
//         }
        status->preprocess_state = speex_preprocess_state_init (status->frame_size, status->sample_rate);
        if (status->preprocess_state == NULL) {
            log_print ("[speex]: Error initializing preprocessor");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        tmp = 1;
        if (speex_preprocess_ctl (status->preprocess_state, SPEEX_PREPROCESS_SET_DENOISE , &tmp) != 0) {
            log_print ("[speex]: Error setting denoisse");
            speex_preprocess_state_destroy (status->preprocess_state);
            speex_encoder_destroy (status->state);
            util_free (status);
        }
        if (speex_preprocess_ctl (status->preprocess_state, SPEEX_PREPROCESS_SET_VAD , &tmp) != 0) {
            log_print ("[speex]: Error setting VAD");
            speex_preprocess_state_destroy (status->preprocess_state);
            speex_encoder_destroy (status->state);
            util_free (status);
        }
        if (speex_preprocess_ctl (status->preprocess_state, SPEEX_PREPROCESS_SET_AGC , &tmp) != 0) {
            log_print ("[speex]: Error setting AGC");
            speex_preprocess_state_destroy (status->preprocess_state);
            speex_encoder_destroy (status->state);
            util_free (status);
        }

        int BUFFER_SECONDS = 1;
        int FRAME_SIZE  = status->frame_size;
        int SAMPLE_RATE = status->sample_rate;
        int ENC_SIZE    = status->encoded_frame_size+5;
        log_print ("[speex]: Frame size: %d\nSample Rate = %d\nEncodec Frame size = %d", FRAME_SIZE, SAMPLE_RATE, ENC_SIZE);
        log_print ("[speex]: Buffer size: %u bytes for 10 seconds", (SAMPLE_RATE/FRAME_SIZE)*ENC_SIZE*BUFFER_SECONDS);

    }
    else if (mode == ENC_SPEEX_MODE_DECODER ) {
        status->state = speex_decoder_init (band);
        if (status->state == NULL) {
            log_print ("[speex]: Error initializing encoder");
            util_free (status);
            return NULL;
        }
        if (speex_decoder_ctl (status->state, SPEEX_GET_FRAME_SIZE, &status->frame_size) != 0) {
            log_print ("[speex]: Error setting getting frame size");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        if (speex_decoder_ctl(status->state, SPEEX_GET_BITRATE, &status->bitrate) != 0) {
            log_print ("[speex]: Error setting getting bitrate");
            speex_decoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        if (speex_decoder_ctl(status->state, SPEEX_GET_SAMPLING_RATE, &status->sample_rate) != 0) {
            log_print ("[speex]: Error setting getting sample rate");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        int tmp=1;
        /*Set the perceptual enhancement on*/
        if (speex_decoder_ctl(status->state, SPEEX_SET_ENH, &tmp) != 0) {
            log_print ("[speex]: Error setting perceptual enhancement");
            speex_encoder_destroy (status->state);
            util_free (status);
            return NULL;
        }
        status->preprocess_state = NULL;
        speex_jitter_init (&status->jitter, status->state, status->sample_rate);
//         if (status->jitter == NULL) {
//             log_print ("[speex]: Error setting perceptual enhancement\n");
//             speex_encoder_destroy (status->state);
//             util_free (status);
//             return NULL;
//         }


    }
    else {
        log_print ("[speex]: Bad speex mode");
        util_free (status);
        return NULL;
    }

    //if (status->frame_size == 0) status->frame_size = 160;
    /*Initialization of the structure that holds the bits*/
    speex_bits_init(&status->bits);
    status->frame_number = 0;
    status->timestamp = 0;

    status->encoded_frame_size = (int)ceil((float)(status->bitrate/(status->sample_rate/status->frame_size))/8.0f);

    log_print ("[speex]: Frames per buffer: %d", status->frame_size);
    log_print ("[speex]: Bitrate: %d [%d]", status->bitrate, status->bitrate/8);
    log_print ("[speex]: Sample: %d", status->sample_rate);
    log_print ("[speex]: Frame calculation: %d bytes", status->encoded_frame_size);

    return status;

}

int enc_speex_stop (enc_speex_status_t* status) {

    if (status == NULL) {
        log_print ("[speex]: Status is NULL");
        return -1;
    }

    if (status->mode == ENC_SPEEX_MODE_ENCODER) {
        /*Destroy the encoder state*/
        speex_preprocess_state_destroy (status->preprocess_state);
        speex_encoder_destroy (status->state);
    }
    else {
        speex_jitter_destroy (&status->jitter);
        speex_decoder_destroy (status->state);
    }
    /*Destroy the bit-packing struct*/
    speex_bits_destroy(&status->bits);
    util_free (status);

    return 1;

}

int enc_speex_encode (enc_speex_status_t* status, int16_t *frame, frame_list_t* frame_list) {

    uint8_t nbBytes = 0;

    //log_print ("  encoding...  ");

    speex_preprocess_run (status->preprocess_state, frame);

    speex_bits_reset(&status->bits);
    status->timestamp += status->frame_size;
    /*Encode the frame: if 0 no need to be transmitted*/
    if (speex_encode_int (status->state, frame, &status->bits) == 0) {
        return 0;
    }

    nbBytes = speex_bits_nbytes (&status->bits);

    uint8_t cbits[nbBytes];

    memset (cbits, 0, nbBytes);

    /*Copy the bits to an array of char that can be written*/
    nbBytes = speex_bits_write(&status->bits, (char*)cbits, nbBytes);
    //TODO: avoid double copy
    //log_print ("nbBytes: %d\n", nbBytes);
    //circular_buffer_write (buffer_out, &status->frame_number, 1);
    status->frame_number++;

    frame_list_add_frame (frame_list, nbBytes, status->timestamp, cbits);
    //log_print ("in %u [%u] bytes ", nbBytes, nbBytes+1+4);



   return nbBytes;
}

int enc_speex_decode (enc_speex_status_t* status, int16_t *frame) {

    speex_jitter_get (&status->jitter, frame, NULL);

    return status->frame_size;
}

int enc_speex_put_sample (enc_speex_status_t* status, int8_t* samples, int samples_size) {

    int cursor = 0;
    int timestamp = 0;
    uint8_t size = 0;

    while (cursor < samples_size) {
        memcpy (&timestamp, &samples[cursor], 4);
        cursor+=4;
        memcpy (&size, &samples[cursor], 1);
        cursor+=1;
        int8_t sample[size];
        memcpy (&sample, &samples[cursor], size);
        //log_print ("size:%u timestamp:%d\n", size, timestamp);
        speex_jitter_put (&status->jitter, (char*)sample, size, timestamp);
        cursor+=size;
    }

    return 1;

}