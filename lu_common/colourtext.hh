
#ifndef __COLOURTEXT_HH__
#define __COLOURTEXT_HH__

#include <stddef.h>

// Since we do not know what background colour the user has in his
// terminal, we cannot use e.g. yellow.  Assuming the background is
// either black or white, the following are readable:

// red, green, (blue), magenta, (cyan)

#define CTR_NONE                0 // used in markconvbold_output
#define CTR_WHITE_BG_RED        1
#define CTR_BLACK_BG_YELLOW     2
#define CTR_YELLOW_BG_BLUE      3
#define CTR_BLUE_BG_YELLOW      4
#define CTR_NORM_DEF_COL        5
#define CTR_DEF_COL             6
#define CTR_NORM                7
#define CTR_BOLD                8
#define CTR_UL                  9
#define CTR_RED                10
#define CTR_GREEN              11
#define CTR_BLUE               12
#define CTR_MAGENTA            13
#define CTR_CYAN               14
#define CTR_BOLD_RED           15
#define CTR_BOLD_GREEN         16
#define CTR_BOLD_BLUE          17 
#define CTR_BOLD_MAGENTA       18
#define CTR_BOLD_CYAN          19
#define CTR_UL_RED             20
#define CTR_UL_GREEN           21
#define CTR_UL_BLUE            22 
#define CTR_UL_MAGENTA         23
#define CTR_UL_CYAN            24
#define CTR_WHITE              25 // not for general use!!!
#define CTR_BLACK              26 // not for general use!!!
#define CTR_NUM_REQUEST        27

#ifdef USE_CURSES

#include <curses.h>
#include <time.h>

size_t colourtext_init();

size_t colourtext_setforce(int force_colour); // -1 = never, 0 = auto, 1 = always

int colourtext_getforce();

// Call whenever stdout or stderr might have been remapped
// (checks isatty status)
size_t colourtext_prepare();

struct colourtext_prepared_item
{
  const char *_str;
  size_t      _len;
};

extern colourtext_prepared_item _colourtext_prepared[2][CTR_NUM_REQUEST];

inline const char *colourtext_get(int fileno,int request) {
  return _colourtext_prepared[fileno][request]._str;
}

inline colourtext_prepared_item *
colourtext_get_prepared(int fileno,int request) {
  return &_colourtext_prepared[fileno][request];
}

#define CT_OUT(request) \
  colourtext_get(0,CTR_##request)
#define CT_ERR(request) \
  colourtext_get(1,CTR_##request)

#define CTP_OUT(request) \
  colourtext_get_prepared(0,CTR_##request)
#define CTP_ERR(request) \
  colourtext_get_prepared(1,CTR_##request)

char *escapeashash(const char *text);  // For debugging

#else

inline size_t colourtext_init() { return 0; }

inline size_t colourtext_setforce(int) { return 0; }

inline int colourtext_getforce() { return 0; }

inline size_t colourtext_prepare() { return 0; }

inline const char *colourtext_get(int,int) { return ""; }

#define CT_OUT(request) ""
#define CT_ERR(request) ""

#define CTP_OUT(request) NULL
#define CTP_ERR(request) NULL

#endif

#endif//__COLOURTEXT_HH__
