/**
 * @file    log.c
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
#include <stdarg.h>
#include <stdlib.h>

#include "log.h"

log_callback_ft log_callback;

void log_set_callback (void* log_callback_ref) {

    log_callback = log_callback_ref;

}

void log_print (char* fmt, ...) {

    va_list args;
    va_start (args,fmt);

    char* entry = NULL;

    //vfprintf (stderr, fmt, args);

    vasprintf(&entry, fmt, args);

    if (log_callback != NULL) {
        (log_callback) (entry);
    }
    else {
        fprintf (stderr, "LOG: %s\n", entry);
    }
    #ifdef __WINDOWS__
    fflush (stderr);
    fflush (stdout);
    #endif



    free (entry);

    va_end (args);
}
