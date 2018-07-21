#pragma once

//https://misc.flogisoft.com/bash/tip_colors_and_formatting

//Stringyfier for c preproccessor
#define TCOL_STR1(x) #x
#define TCOL_STR(x) TCOL_STR1 (x)


//ANSI text color escape string formatter
#define TCOL(text, foreground, background) \
"\x1B[" TCOL_STR (text) ";3" TCOL_STR (foreground) ";4" TCOL_STR (background) "m"


//Used for text attributes
#define TCOL_NORMAL       0
#define TCOL_BOLD         1
#define TCOL_UNDERSCORE   4
#define TCOL_BLINK        5
#define TCOL_REVERSE      7
#define TCOL_CONCEAL      8


//Used for foreground and background color
#define TCOL_BLACK   0
#define TCOL_RED     1
#define TCOL_GREEN   2
#define TCOL_YELLOW  3
#define TCOL_BLUE    4
#define TCOL_MAGENTA 5
#define TCOL_CYAN    6
#define TCOL_WHITE   7
#define TCOL_DEFAULT 9


//Default text color used in terminals
//#define TCOL_DEFAULT TCOL (TCOL_NORMAL, TCOL_DEFAULT, TCOL_DEFAULT)
#define TCOL_RESET "\x1B[0m"

