 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <limits.h>
 #include <errno.h>
 #include <stdarg.h>
 
 #include "stdio.h"
 
 #define	INIT_SZ	128
 
 /* VARARGS2 */
 int
 vasprintf(char **str, const char *format, va_list ap)
 {
 	char string[INIT_SZ];
 	char *newstr;
 	int ret;
 	size_t len;
 
 	*str = NULL;
 	ret = vsnprintf(string, INIT_SZ, format, ap);
 	if (ret < 0)	/* retain the value of errno from vsnprintf() */
 		return (-1);
 	if (ret < INIT_SZ) {
 		len = ret + 1;
 		if ((newstr = malloc(len)) == NULL)
 			return (-1);	/* retain errno from malloc() */
 		(void) strncpy(newstr, string, len);
 		*str = newstr;
 		return (ret);
 	}
 	/*
 	 * We will perform this loop more than once only if some other
 	 * thread modifies one of the vasprintf() arguments after our
 	 * previous call to vsnprintf().
 	 */
 	for (;;) {
 		if (ret == INT_MAX) {	/* Bad length */
 			errno = ENOMEM;
 			return (-1);
 		}
 		len = ret + 1;
 		if ((newstr = malloc(len)) == NULL)
 			return (-1);	/* retain errno from malloc() */
 		ret = vsnprintf(newstr, len, format, ap);
 		if (ret < 0) {		/* retain errno from vsnprintf() */
 			free(newstr);
 			return (-1);
 		}
 		if (ret < len) {
 			*str = newstr;
 			return (ret);
 		}
 		free(newstr);
 	}
 }
 
 int
 asprintf(char **str, const char *format, ...)
 {
 	va_list ap;
 	int ret;

 	*str = NULL;
 	va_start(ap, format);
 	ret = vasprintf(str, format, ap);
 	va_end(ap);
 
 	return (ret);
}
