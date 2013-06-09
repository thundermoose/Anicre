
#ifdef USE_CURSES

#include "colourtext.hh"

#include <curses.h>
#include <term.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CTR_PART_BOLD      1
#define CTR_PART_UNDERLINE 2
#define CTR_PART_SGR0      3
#define CTR_PART_OP        4
#define CTR_PART_FGCOL(i)  (5+(i))
#define CTR_PART_BGCOL(i)  (13+(i))
#define CTR_MAX_PART 21

const char *_colourtext_escape_part[CTR_MAX_PART];

int _do_colourtext = 0;

/* Some implementations do not have const in the tigetstr argument.
 * Work around associated compiler complaints.
 */

char *tigetstr_wrap(const char *capname)
{
  char *capnametmp = strdup(capname);

  char *ret = tigetstr(capnametmp);

  free(capnametmp);

  return ret;
}

size_t colourtext_init()
{
  /*
  if (force_colour == -1 ||
      isatty(STDOUT_FILENO))
  */

  for (int i = 0; i < CTR_MAX_PART; i++)
    _colourtext_escape_part[i] = NULL;  

  int errret, ret;

  ret = setupterm(NULL, 1, &errret);
  
  if (ret == ERR)
    {
      return colourtext_prepare();
    }

  // printf ("%d %d %d\n",ret,ERR,errret);

  _colourtext_escape_part[CTR_PART_BOLD] = tigetstr_wrap("bold");

  _colourtext_escape_part[CTR_PART_UNDERLINE] = tigetstr_wrap("smul");

  _colourtext_escape_part[CTR_PART_SGR0] = tigetstr_wrap("sgr0");

  _colourtext_escape_part[CTR_PART_OP] = tigetstr_wrap("op");

#ifndef NCURSES_CONST
#define NCURSES_CONST
#endif

  NCURSES_CONST char *setaf = tigetstr_wrap("setaf");

  // printf ("%p %p\n",_escape_bold,setaf);

  fflush(stdout);

  for (int i = 0; i < 8; i++)
    if (setaf)
      _colourtext_escape_part[CTR_PART_FGCOL(i)] = strdup(tparm(setaf,i));

  NCURSES_CONST char *setab = tigetstr_wrap("setab");

  for (int i = 0; i < 8; i++)
    if (setab)
      _colourtext_escape_part[CTR_PART_BGCOL(i)] = strdup(tparm(setab,i));

  // printf ("%p %p\n",_escape_col[0],_escape_col[1]);

  /*
  for (int i = 0; i < CTR_MAX_PART; i++)
    if (_colourtext_escape_part[i])
      printf ("part %d: %d \"%s\"\n",i,
	      strlen(_colourtext_escape_part[i]),
	      escapeashash(_colourtext_escape_part[i]));
  */
  return colourtext_prepare();
}

size_t colourtext_setforce(int force_colour)
{
  _do_colourtext = force_colour;

  return colourtext_prepare();
}

int colourtext_getforce()
{
  return _do_colourtext;
}

colourtext_prepared_item _colourtext_prepared[2][CTR_NUM_REQUEST];

char *escapeashash(const char *text)
{
  char *out = (char *) malloc(strlen(text)+1);
  char *dest = out;

  while (*text != 0)
    {
      if (*text != '\033')
	*(dest++) = *text;
      else
	{
	  *(dest++) = '#';
	}
      text++;
    }

  *dest = 0;

  return out;
}

size_t colourtext_prepare()
{
  static int _prepared = 0;

  size_t maxlen = 0;

  for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < CTR_NUM_REQUEST; j++)
	{
	  if (_prepared && strcmp(_colourtext_prepared[i][j]._str,"") != 0)
	    free(const_cast<char *>(_colourtext_prepared[i][j]._str));
	  _colourtext_prepared[i][j]._str = "";
	  _colourtext_prepared[i][j]._len = 0;
	}

      if (_do_colourtext == -1)
	continue;

      //printf ("prepare %d: %d\n",i,
      //        isatty(i == 0 ? STDOUT_FILENO : STDERR_FILENO));

      if (!isatty(i == 0 ? STDOUT_FILENO : STDERR_FILENO) &&
	  _do_colourtext != 1)
	continue;

#define MAX_PARTS 4

      const char _colourtext_item_parts[CTR_NUM_REQUEST][MAX_PARTS] = {
	{ 0, },                                 // CTR_NONE
	{ CTR_PART_FGCOL(COLOR_WHITE), 
	  CTR_PART_BGCOL(COLOR_RED), 0 },       // CTR_WHITE_BG_RED
	{ CTR_PART_FGCOL(COLOR_BLACK), 
	  CTR_PART_BGCOL(COLOR_YELLOW), 0, 0 }, // CTR_BLACK_BG_YELLOW
	{ CTR_PART_FGCOL(COLOR_YELLOW), 
	  CTR_PART_BGCOL(COLOR_BLUE), 0, 0 },   // CTR_YELLOW_BG_BLUE
	{ CTR_PART_FGCOL(COLOR_BLUE), 
	  CTR_PART_BGCOL(COLOR_YELLOW), 0, 0 },   // CTR_BLUE_BG_YELLOW
 	{ CTR_PART_SGR0, CTR_PART_OP, 0, 0 },   // CTR_NORM_DEF_COL
	{ CTR_PART_OP, 0, 0, 0 },               // CTR_DEF_COL
	{ CTR_PART_SGR0, 0, 0, 0 },             // CTR_NORM
	{ CTR_PART_BOLD, 0, 0, 0 },             // CTR_BOLD
	{ CTR_PART_UNDERLINE, 0, 0, 0 },        // CTR_UL
	{ CTR_PART_FGCOL(COLOR_RED),     0, 0, 0 }, // CTR_RED
	{ CTR_PART_FGCOL(COLOR_GREEN),   0, 0, 0 }, // CTR_GREEN
	{ CTR_PART_FGCOL(COLOR_BLUE),    0, 0, 0 }, // CTR_BLUE
	{ CTR_PART_FGCOL(COLOR_MAGENTA), 0, 0, 0 }, // CTR_MAGENTA
	{ CTR_PART_FGCOL(COLOR_CYAN),    0, 0, 0 }, // CTR_CYAN
	{ CTR_PART_BOLD, 
	  CTR_PART_FGCOL(COLOR_RED),     0, 0 }, // CTR_BOLD_RED
	{ CTR_PART_BOLD, 
	  CTR_PART_FGCOL(COLOR_GREEN),   0, 0 }, // CTR_BOLD_GREEN
	{ CTR_PART_BOLD, 
	  CTR_PART_FGCOL(COLOR_BLUE),    0, 0 }, // CTR_BOLD_BLUE
	{ CTR_PART_BOLD, 
	  CTR_PART_FGCOL(COLOR_MAGENTA), 0, 0 }, // CTR_BOLD_MAGENTA
	{ CTR_PART_BOLD, 
	  CTR_PART_FGCOL(COLOR_CYAN),    0, 0 }, // CTR_BOLD_CYAN
	{ CTR_PART_UNDERLINE, 
	  CTR_PART_FGCOL(COLOR_RED),     0, 0 }, // CTR_UL_RED
	{ CTR_PART_UNDERLINE, 
	  CTR_PART_FGCOL(COLOR_GREEN),   0, 0 }, // CTR_UL_GREEN
	{ CTR_PART_UNDERLINE, 
	  CTR_PART_FGCOL(COLOR_BLUE),    0, 0 }, // CTR_UL_BLUE
	{ CTR_PART_UNDERLINE, 
	  CTR_PART_FGCOL(COLOR_MAGENTA), 0, 0 }, // CTR_UL_MAGENTA
	{ CTR_PART_UNDERLINE, 
	  CTR_PART_FGCOL(COLOR_CYAN),    0, 0 }, // CTR_UL_CYAN
	{ CTR_PART_FGCOL(COLOR_WHITE),   0, 0, 0 }, // CTR_WHITE
	{ CTR_PART_FGCOL(COLOR_BLACK),   0, 0, 0 }, // CTR_BLACK
      };

      for (int j = 0; j < CTR_NUM_REQUEST; j++)
	{
	  char prepare[256];

	  prepare[0] = 0;

	  for (int k = 0; k < MAX_PARTS; k++)
	    if (_colourtext_item_parts[j][k])
	      strcat(prepare,
		     _colourtext_escape_part[(int) _colourtext_item_parts[j][k]]);

	  _colourtext_prepared[i][j]._str = strdup(prepare);
	  _colourtext_prepared[i][j]._len = strlen(prepare);

	  if (_colourtext_prepared[i][j]._len > maxlen)
	    maxlen = _colourtext_prepared[i][j]._len;	    
	  /*
	  printf ("prepare %d/%d: %d \"%s\"\n",i,j,
	  	  strlen(_colourtext_prepared[i][j]),
		  escapeashash(_colourtext_prepared[i][j]));
	  */
	}
    }

  _prepared = 1;

  return maxlen;
}

// Since we do not know what background colour the user has in his
// terminal, we cannot use e.g. yellow.  Assuming the background is
// either black or white, the following are readable:

// red, green, (blue), magenta, (cyan)

#endif/*USE_CURSES*/
