#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>

int vasprintf(char **str, const char *format, va_list ap);
int asprintf(char **str, const char *format, ...);

#endif
