#include "assert_extended.h"

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
)
{
	va_list Ap;
	va_start (Ap, Format);
	fprintf	
	(
		stderr,
		"=== Assertion (%i) ===\n%s:%i: %s: Assertion `%s' failed.\n",
		Counter, 
		File_Name, 
		Line_Number, 
		Function_Name, 
		Assertion_String
	);
	fprintf (stderr, "Code:  %s (%i)\n", Code_Name, Code);
	fprintf (stderr, "errno: %s (%i)\n", strerror (errno), errno);
	vfprintf (stderr, Format, Ap);
	fprintf (stderr, "%s", "\n\n");
	va_end (Ap);
	abort ();
}


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
)
{
	va_list Ap;
	va_start (Ap, Format);
	fprintf	
	(
		stderr,
		"=== Note (%i) ===\n%s:%i: %s:\n",
		Counter, 
		File_Name, 
		Line_Number, 
		Function_Name
	);
	fprintf (stderr, "Code:  %s (%i)\n", Code_Name, Code);
	fprintf (stderr, "errno: %s (%i)\n", strerror (errno), errno);
	vfprintf (stderr, Format, Ap);
	fprintf (stderr, "%s", "\n\n");
	va_end (Ap);
}

