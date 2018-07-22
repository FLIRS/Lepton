#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "tcol.h"

//https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
#ifdef _WIN32
#define __RELATIVE_FILE__ (strrchr("\\" __FILE__, '\\') + 1)
#else
#define __RELATIVE_FILE__ (strrchr("/" __FILE__, '/') + 1)
#endif

//https://gcc.gnu.org/onlinedocs/gcc/Variadic-Macros.html
#define ASSERT(A)               if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (0), NULL, NULL                );}
#define ASSERT_F(A, F, ...)     if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (0), NULL,  (F), ## __VA_ARGS__);}
#define ASSERT_C(A, C)          if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (C),   #C, NULL                );}
#define ASSERT_CF(A, C, F, ...) if (!(A)) {assert_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #A, (C),   #C,  (F), ## __VA_ARGS__);}

#define TRACE(F)            trace_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, (0), NULL, NULL                )
#define TRACE_F(F, ...)     trace_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, (0), NULL,  (F), ## __VA_ARGS__)
#define TRACE_CF(C, F, ...) trace_format (__COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, (C),   #C,  (F), ## __VA_ARGS__)


__attribute__ ((__unused__))
static void assert_format 
(
	int id, 
	char const * file, 
	int line, 
	char const * fn, 
	char const * exp, 
	int code,
	char const * scode,
	char const * fmt, 
	...
)
{
	va_list list;
	va_start (list, fmt);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_RED, TCOL_DEFAULT) "ASSERT %04i" TCOL_RESET " ", id);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%s" TCOL_RESET ":", file);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLUE, TCOL_DEFAULT) "%04i" TCOL_RESET " in ", line);
	fprintf (stderr, TCOL (TCOL_NORMAL, TCOL_YELLOW , TCOL_DEFAULT) "%s" TCOL_RESET " () ", fn);
	fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLACK, TCOL_RED) "[%s]" TCOL_RESET " ", exp);
	if (scode)
	{
		fprintf (stderr, TCOL (TCOL_BOLD, TCOL_BLACK, TCOL_BLUE) "[%i %s]" TCOL_RESET " ", code, scode);
	}
	fprintf (stderr, "[%04i:" TCOL (TCOL_BOLD, TCOL_RED , TCOL_DEFAULT) "%s" TCOL_RESET "]: ", errno, strerror (errno));
	vfprintf (stderr, fmt, list);
	fprintf (stderr, "\n");
	va_end (list);
	abort ();
}



__attribute__ ((__unused__))
static void trace_format 
(
	int id, 
	char const * file, 
	int line, 
	char const * fn, 
	int code,
	char const * scode,
	char const * fmt, 
	...
)
{
	va_list list;
	va_start (list, fmt);
	fprintf	
	(
		stderr,
		"TRACE %04i [%s:%04i in %s ()] [%s %i]: ",
		id, 
		file, 
		line, 
		fn,
		scode,
		code
	);
	vfprintf (stderr, fmt, list);
	fprintf (stderr, "\n");
	va_end (list);
}
