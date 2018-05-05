#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#define ASSERT_ACF(A, C, F, ...) if (!(A)) {assert_format (__COUNTER__, __FILE__, __LINE__, __func__, #A, C, #C, F, __VA_ARGS__);}
#define NOTE(C, F, ...) note_format (__COUNTER__, __FILE__, __LINE__, __func__, C, #C, F, __VA_ARGS__)



void assert_format 
(
	int const Counter,
	char const * File_Name, 
	int const Line_Number, 
	char const * Function_Name, 
	char const * Assertion_String,
	int Code,
	char const * Code_Name,
	char const * Format,
	...
);

void note_format 
(
	int const Counter,
	char const * File_Name, 
	int const Line_Number, 
	char const * Function_Name, 
	int Code,
	char const * Code_Name,
	char const * Format,
	...
);

