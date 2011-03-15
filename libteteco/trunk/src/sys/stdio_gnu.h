#ifndef _STDIO_GNU_H_
#define _STDIO_GNU_H_

#include <stdarg.h>
#include <stdio.h>

int vasprintf(char **str, const char *format, va_list ap);
int asprintf(char **str, const char *format, ...);

#endif
