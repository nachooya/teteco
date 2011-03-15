/**
 * @file    audio.c
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

#ifdef __WINDOWS__
#include "sys/stdio_gnu.h"
#endif
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "log.h"
#include "util.h"
#include "audio.h"


static int audio_record_callback (const void                      *inputBuffer,
                                  void                            *outputBuffer,
                                  unsigned long                   framesPerBuffer,
                                  const PaStreamCallbackTimeInfo  *timeInfo,
                                  PaStreamCallbackFlags           statusFlags,
                                  void                            *userData );

static int audio_play_callback (const void *inputBuffer,
                                void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData);


PaDeviceIndex audio_get_default_device (audio_device_type_t in_out) {

    PaHostApiIndex api = Pa_HostApiTypeIdToHostApiIndex (paALSA);

    if (api == paHostApiNotFound) {
        if (in_out == AUDIO_DEV_IN)
            return Pa_GetDefaultInputDevice ();
        else if (in_out == AUDIO_DEV_OUT)
            return Pa_GetDefaultOutputDevice ();
        else
            return paNoDevice;
    }

    PaDeviceIndex device;

    for (device = 0; device < Pa_GetHostApiInfo(api)->deviceCount; device++) {

        if (!strcmp ("default", Pa_GetDeviceInfo (Pa_HostApiDeviceIndexToDeviceIndex (api, device))->name)) {
            return Pa_HostApiDeviceIndexToDeviceIndex (api, device);
        }

    }

    if (in_out == AUDIO_DEV_IN)
        return Pa_GetDefaultInputDevice ();
    else if (in_out == AUDIO_DEV_OUT)
        return Pa_GetDefaultOutputDevice ();
    else
        return paNoDevice;

}

void audio_print_devices (audio_device_type_t in_out) {

    char**         devices    = NULL;
    int*           index      = NULL;
    int            n_devices  = 0;

    n_devices = audio_get_devices (in_out, &index, &devices);
    int i;
    for (i=0; i < n_devices; i++) {
        log_print ("[audio]: %s", devices[i]);
        util_free (devices[i]);
    }
    util_free (devices);

}

audio_data_t* audio_init (double sample_rate, unsigned long framesPerBuffer, void* get_frame_par, void *user_data) {

    audio_data_t* audio_data = NULL;

    audio_data = util_malloc (sizeof (audio_data_t));
    if (audio_data == NULL) {
        log_print ("[audio]: Error initializing audio_data: %m");
        return NULL;
    }
    else {
        audio_data->sample_rate     = sample_rate;
        audio_data->framesPerBuffer = framesPerBuffer;
        audio_data->stream          = NULL;
        audio_data->frame_callback  = (get_frame_func) get_frame_par;
        audio_data->data            = user_data;

        log_print ("[audio]: Initialized");

        return audio_data;
    }

}

int audio_end (audio_data_t* audio_data) {

    audio_stop (audio_data);
    util_free (audio_data);

    return 1;

}

int audio_get_devices (audio_device_type_t in_out, int** index, char*** devices) {

    // Defalut device is always the first

    int     numDevices = 0;
    int     numSelectedDevices = 0;
    char**  devices_temp  = NULL;
    int*    index_temp    = NULL;
    const   PaDeviceInfo *deviceInfo = NULL;

    // Check parameters
    if (*devices != NULL || *index != NULL) return -1;

    numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 ) {
        log_print( "[audio]: ERROR: Pa_GetDeviceCount returned %s", Pa_GetErrorText (numDevices) );
        return -1;
    }

    log_print( "[audio]: Number of devices = %d", numDevices );

    // Set default device as first
    int default_dev = audio_get_default_device (in_out);

    if (default_dev != paNoDevice) {
        if ((deviceInfo = Pa_GetDeviceInfo (default_dev)) != NULL) {
            numSelectedDevices++;
            devices_temp = util_realloc (devices_temp, numSelectedDevices*sizeof(char*));
            if (devices_temp == NULL) {
                log_print ("[audio]: Error on realloc: %m");
                return -1;
            }
            else if (asprintf (&devices_temp[numSelectedDevices-1], "Device %d: [%s] %s", default_dev, Pa_GetHostApiInfo( deviceInfo->hostApi )->name, deviceInfo->name) == -1) {
                util_free (devices_temp);
                log_print ("[audio]: Error on asprintf: %m");
                return -1;
            }
            index_temp = realloc (index_temp, numSelectedDevices*sizeof(unsigned int));
            if (index == NULL) {
                log_print ("[audio]: Error on realloc[2]: %m");
                return -1;
            }
            else {
                index_temp[numSelectedDevices-1] = default_dev;
            }
        }
    }

    int i;
    for (i=0; i<numDevices; i++) {

        deviceInfo = Pa_GetDeviceInfo (i);
        if (deviceInfo == NULL) continue;
        if (i == default_dev) continue;

//         log_print( "[audio]: Name                        = %s",    deviceInfo->name );
//         log_print( "[audio]: Host API                    = %s",    Pa_GetHostApiInfo( deviceInfo->hostApi )->name );
//         log_print( "[audio]: Max inputs                  = %d",    deviceInfo->maxInputChannels  );
//         log_print( "[audio]: Max outputs                 = %d",    deviceInfo->maxOutputChannels  );
//         log_print( "[audio]: Default low input latency   = %8.3f", deviceInfo->defaultLowInputLatency  );
//         log_print( "[audio]: Default low output latency  = %8.3f", deviceInfo->defaultLowOutputLatency  );
//         log_print( "[audio]: Default high input latency  = %8.3f", deviceInfo->defaultHighInputLatency  );
//         log_print( "[audio]: Default high output latency = %8.3f", deviceInfo->defaultHighOutputLatency  );
//         log_print( "[audio]: Default sample rate         = %8.2f", deviceInfo->defaultSampleRate );

        if (in_out == AUDIO_DEV_IN) {

            if (deviceInfo->maxInputChannels > 0) {
                numSelectedDevices++;
                devices_temp = util_realloc (devices_temp, numSelectedDevices*sizeof(char*));
                if (devices_temp == NULL) {
                    log_print ("[audio]: Error on realloc: %m");
                    return -1;
                }
                else if (asprintf (&devices_temp[numSelectedDevices-1], "Device %d: [%s] %s", i, Pa_GetHostApiInfo( deviceInfo->hostApi )->name, deviceInfo->name) == -1) {
                    util_free (devices_temp);
                    log_print ("[audio]: Error on aslog_print: %m");
                    return -1;
                }
                index_temp = realloc (index_temp, numSelectedDevices*sizeof(int));
                if (index_temp == NULL) {
                    log_print ("[audio]: Error on realloc[2]: %m");
                    return -1;
                }
                else {
                    index_temp[numSelectedDevices-1] = i;
                }
            }
        }
        else if (in_out == AUDIO_DEV_OUT) {
            if (deviceInfo->maxOutputChannels > 0) {
                numSelectedDevices++;
                devices_temp = util_realloc (devices_temp, numSelectedDevices*sizeof(char*));
                if (devices_temp == NULL) {
                    log_print ("[audio]: Error realloc: %m");
                    return -1;
                }
                else if (asprintf (&devices_temp[numSelectedDevices-1], "Device %d: [%s] %s", i, Pa_GetHostApiInfo( deviceInfo->hostApi )->name, deviceInfo->name) == -1) {
                    util_free (devices_temp);
                    log_print ("[audio]: Error on asprintf: %m");
                    return -1;
                }
                index_temp = realloc (index_temp, numSelectedDevices*sizeof(int));
                if (index_temp == NULL) {
                    log_print ("[audio]: Error on realloc[2]: %m");
                    return -1;
                }
                else {
                    index_temp[numSelectedDevices-1] = i;
                }
            }
        }
        else {
            log_print ("[audio]: Bad AUDIO_DEV_TYPE");
        }
    }

    *devices = devices_temp;
    *index   = index_temp;

    return numSelectedDevices;

}

int audio_record (int device, audio_data_t* audio_data) {

    PaStreamParameters  inputParameters;
    PaError             err = paNoError;
    PaDeviceIndex       deviceIndex = device;

    if (audio_data == NULL) {
        log_print ("[audio]: Error no audio_data");
        return -1;
    }

    if (deviceIndex == -1) {
        #ifdef __LINUX__
        deviceIndex = audio_get_default_device (AUDIO_DEV_IN);
        #else
        deviceIndex = Pa_GetDefaultInputDevice ();
        #endif
        if (deviceIndex == paNoDevice) {
            log_print ("[audio]: Default input device not found");
            return -1;
        }
    }
    log_print ("[audio]: Using device %d: %s:%s", deviceIndex, Pa_GetHostApiInfo(Pa_GetDeviceInfo(deviceIndex)->hostApi)->name, Pa_GetDeviceInfo(deviceIndex)->name);

    inputParameters.device                      = deviceIndex;
    inputParameters.channelCount                = 1;
    inputParameters.sampleFormat                = paInt16;
    inputParameters.suggestedLatency            = Pa_GetDeviceInfo(deviceIndex)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo   = NULL;

    //enc_speex_start     (&encoder, ENC_SPEEX_MODE_ENCODER);

    log_print ("[audio]: Opening stream...");
    err = Pa_OpenStream(&audio_data->stream,
                        &inputParameters,
                        NULL,                           /* &outputParameters, */
                        audio_data->sample_rate,
                        audio_data->framesPerBuffer,    /* frames per buffer */
                        paClipOff,                      /* we won't output out of range samples so don't bother clipping them */
                        audio_record_callback,
                        audio_data);

    if( err != paNoError ) {
        log_print ("[audio]: Error %s", Pa_GetErrorText (err));
        return -1;
    }
    log_print ("[audio]: done!");

    if (audio_data->stream) {
        log_print ("[audio]: Starting stream...");
        err = Pa_StartStream(audio_data->stream);
        if( err != paNoError ) {
            log_print ("[audio]: Error %s", Pa_GetErrorText (err));
            return -1;
        }
        log_print ("[audio]: done!");
        log_print("[audio]: Now recording!!");
        return 1;
    }
    else {
        log_print ("[audio]: Cannot initialize record stream");
        return -1;
    }
}

int audio_play (int device, audio_data_t* audio_data) {

    PaStreamParameters  outputParameters;
    PaError             err = paNoError;
    PaDeviceIndex       deviceIndex = device;

    if (audio_data == NULL) {
        log_print ("[audio]: Error no audio_data");
        return -1;
    }

    if (deviceIndex == -1) {
        #ifdef __LINUX__
        deviceIndex = audio_get_default_device (AUDIO_DEV_OUT);
        #else
        deviceIndex = Pa_GetDefaultOutputDevice ();
        #endif

        if (deviceIndex == paNoDevice) {
            log_print ("[audio]: Default output device not found %d", deviceIndex);
            log_print ("[audio]: Available devices are:");
            audio_print_devices (AUDIO_DEV_OUT);
            return -1;
        }
    }
    log_print ("[audio]: Using device %d: %s", deviceIndex, Pa_GetDeviceInfo(deviceIndex)->name);

    outputParameters.device = deviceIndex;
    outputParameters.channelCount = 1;
    outputParameters.sampleFormat =  paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo (outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;


    log_print("[audio]: Begin playback...");
    err = Pa_OpenStream(&audio_data->stream,
                         NULL, /* no input */
                         &outputParameters,
                         audio_data->sample_rate,
                         audio_data->framesPerBuffer,
                         paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                         audio_play_callback,
                         audio_data);

    if( err != paNoError ) {
        log_print ("[audio]: Error %s", Pa_GetErrorText (err));
        audio_print_devices (AUDIO_DEV_OUT);
        return -1;
    }
    log_print ("[audio]: done!");


    if (audio_data->stream) {

        err = Pa_StartStream( audio_data->stream );
        if( err != paNoError ) {
            log_print ("[audio]: Error %s", Pa_GetErrorText (err));
            return -1;
        }
        log_print("[audio]: Waiting for playback to finish.");
    }
    else {
        log_print ("[audio]: Cannot initialize play stream");
        return -1;
    }
    return 1;
}

int audio_stop (audio_data_t* audio_data) {

    log_print ("[audio]: Stopping stream");

    if (audio_data == NULL) return -1;

    if (audio_data->stream == NULL) return -1;

    PaError err = Pa_IsStreamActive (audio_data->stream);

    if (err == 1) {
        err = Pa_StopStream (audio_data->stream);
        if (err != paNoError) {
            log_print ("[audio]: Error[1] %s", Pa_GetErrorText (err));
            return -1;
        }
        err = Pa_CloseStream (audio_data->stream);
        if (err != paNoError) {
            log_print ("[audio]: Error[2] %s", Pa_GetErrorText (err));
            return -1;
        }
    }
    else if (err == 0) {
        err = Pa_CloseStream (audio_data->stream);
        if (err != paNoError) {
            log_print ("[audio]: Error[3] %s", Pa_GetErrorText (err));
            return -1;
        }
    }
    else {
        log_print ("[audio]: Error[4] %s", Pa_GetErrorText (err));
        return -1;
    }
    audio_data->stream = NULL;

    return 1;
}




static int audio_record_callback (const void                      *inputBuffer,
                                  void                            *outputBuffer,
                                  unsigned long                   framesPerBuffer,
                                  const PaStreamCallbackTimeInfo  *timeInfo,
                                  PaStreamCallbackFlags           statusFlags,
                                  void                            *userData ) {

    int16_t       *input            = (int16_t*)inputBuffer;
    audio_data_t  *audio_data       = (audio_data_t*)userData;
    int16_t       silence_buffer[framesPerBuffer];

    //TODO do constant
    memset (silence_buffer, 0, framesPerBuffer);


    if (statusFlags & paInputUnderflow) {
        log_print ("[audio]: Buffer underflow");
    }
    if (statusFlags & paInputOverflow) {
        log_print ("[audio]: Buffer overflow");
    }
    if (statusFlags & paPrimingOutput) {
        log_print ("[audio]: Priming output");
    }

    //log_print ("[audio]: recording [%f]", timeInfo->inputBufferAdcTime);
    if(inputBuffer == NULL) {
        log_print ("[audio]: Nothing to write");
        (audio_data->frame_callback) (framesPerBuffer, silence_buffer, audio_data->data);
    }
    else {
        //log_print ("[audio]: Writting %d bytes", framesPerBuffer);
        (audio_data->frame_callback) (framesPerBuffer, input, audio_data->data);
    }

    return paContinue;
}

static int audio_play_callback (const void *inputBuffer,
                                void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData)
{
    int16_t      *output     = (int16_t*) outputBuffer;
    audio_data_t *audio_data = (audio_data_t*) userData;

    if (statusFlags & paOutputUnderflow) {
        log_print ("[audio]: Buffer underflow");
    }
    if (statusFlags & paOutputOverflow) {
        log_print ("[audio]: Buffer overflow");
    }
    if (statusFlags & paPrimingOutput) {
        log_print ("[audio]: Priming output");
    }

    int16_t frames[framesPerBuffer];

    //log_print ("[audio]: playing [%f]", timeInfo->outputBufferDacTime);

    //TODO: avoid doble copy
    int frames_written = (audio_data->frame_callback) (framesPerBuffer, frames, audio_data->data);

    int i;
    for (i=0; i<frames_written; i++) {
        output[i] = frames[i];
    }

    if (frames_written < framesPerBuffer) {
        log_print ("[audio]: Not enough frames to playback, sending silence");
        int i;
        for (i=frames_written; i<framesPerBuffer; i++) output[i] = 0;
    }
    return paContinue;
}
