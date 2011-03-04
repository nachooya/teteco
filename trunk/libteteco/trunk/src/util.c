/**
 * @file    util.c
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


#include <time.h>
#include <sys/time.h>

#include "util.h"

void *util_realloc (void* chunk, size_t size) {

    void* tmp = chunk;
    chunk = realloc (chunk, size);
    if (chunk == NULL) {
        free (tmp);
        return NULL;
    }
    else {
        return chunk;
    }
}


int util_now_offset (struct timespec *time, struct timespec *offset) {

#if defined (__MACOSX__) || defined (__WINDOWS__)
    struct timeval now;

    if (0 != gettimeofday (&now, NULL)) {
        return 0;
    }

    time->tv_sec = now.tv_sec + offset->tv_sec;
    time->tv_nsec = now.tv_usec*1000 + offset->tv_nsec;

#else

    struct  timespec  now;
    if (0 != clock_gettime (CLOCK_REALTIME, &now)) {
        return 0;
    }

    time->tv_sec  = now.tv_sec + offset->tv_sec;
    time->tv_nsec = now.tv_nsec + offset->tv_nsec;

#endif

    if (time->tv_nsec >= 1000000000L) {        /* Carry? */
        time->tv_sec++ ;  
        time->tv_nsec = time->tv_nsec - 1000000000L ;
    }

    return 1;

}
