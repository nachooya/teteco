/**
 * @file    util.h
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


#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdlib.h>
#include <sys/time.h>

#define util_free(a) free(a);a=NULL
#define util_malloc(a) malloc (a)
#define util_calloc(a,b) calloc (a, b)

#ifdef __WINDOWS__
#include <pthread.h>
// struct timespec {
	// time_t tv_sec;
	// long   tv_nsec;
// };

#endif

void*   util_realloc    (void* chunk, size_t size);
int     util_now_offset (struct timespec *time, struct timespec *offset);

#endif