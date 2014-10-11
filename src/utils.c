#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <glib.h>
#include "utils.h"

int mod(int x, int m)
{
	int r = x % m;
	return (r < 0) ? r + m : r;
}

void die(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "axon: ");
	
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void print_err(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "axon: ");

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
}

char *str_copy(char **dest, const char *src)
{
	char *tmp;
	tmp = g_strdup(src);
	
	if (dest && *dest) {
		g_free(*dest);
		*dest = tmp;
	}

	return tmp;
}
