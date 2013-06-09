
#ifndef __MARKCONVBOLD_HH__
#define __MARKCONVBOLD_HH__

// %-conversions in error/warning/info-strings are automatically
// boldified
#define ERR_BOLD    "\033A" // place in string before bold section
#define ERR_ENDBOLD "\033B" // place in string after bold section
#define ERR_RED     "\033C" // place in string before red text
#define ERR_GREEN   "\033D" // place in string before green text
#define ERR_BLUE    "\033E" // place in string before blue text
#define ERR_MAGENTA "\033F" // place in string before magenta text
#define ERR_CYAN    "\033G" // place in string before cyan text
#define ERR_BLACK   "\033H" // place in string before black text
#define ERR_ENDCOL  "\033I" // place in string after coloured section
#define ERR_NOBOLD  "\033J" // place in string before %-conversion, that
                            // should not be boldified

char *markconvbold(const char *fmt);

void markconvbold_output(const char *fmt,int linemarkup);

#endif//__MARKCONVBOLD_HH__
