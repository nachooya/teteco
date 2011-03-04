/**
 * @file    circular_buffer.h
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


#ifndef __CIRCULAR_BUFER_H__
#define __CIRCULAR_BUFER_H__

typedef struct {
    unsigned int type_size;
    unsigned int size;
    unsigned int n_read;
    unsigned int n_written;
    unsigned int cursor_read;
    unsigned int cursor_write;
    void*        data;
} circular_buffer_t;

circular_buffer_t* circular_buffer_new         (unsigned int size, unsigned int type_size);
void*              circular_buffer_free        (circular_buffer_t* buffer);
void               circular_buffer_write       (circular_buffer_t* buffer, void* input,  unsigned int size);
void               circular_buffer_write_value (circular_buffer_t* buffer, void* input,  unsigned int size);
unsigned int       circular_buffer_read        (circular_buffer_t* buffer, void* output, unsigned int size);
unsigned int       circular_buffer_nbytes      (circular_buffer_t* buffer);
void               circular_buffer_print       (circular_buffer_t* buffer);

#endif