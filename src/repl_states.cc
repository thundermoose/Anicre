
#include <stdlib.h>
#include <string.h>

#include "error.hh"
#include "colourtext.hh"

#include "repl_states.hh"

repl_states_by_m_N::repl_states_by_m_N(int min_m, int max_m, int max_N)
{
  _min_m = min_m;
  _rng_m = max_m - min_m + 1;
  _rng_N = max_N + 1;

  size_t sz = _rng_m * _rng_N;

  _entries = (int*) malloc (sz * sizeof (int));

  if (!_entries)
    FATAL("Memory allocation failure (_entries).");

  memset (_entries, 0, sz * sizeof (int));
}

void repl_states_by_m_N::dump()
{
  printf ("%sm%s\\%sN%s  %s",
	  CT_OUT(BOLD_BLUE), CT_OUT(NORM_DEF_COL),
	  CT_OUT(BOLD_BLUE), CT_OUT(NORM_DEF_COL),
	  CT_OUT(GREEN));
  for (int N = 0; N < _rng_N; N++)
    printf ("%4d",N);
  printf ("%s\n",CT_OUT(NORM_DEF_COL));

  for (int m = _min_m; m < _min_m + _rng_m; m++)
    {
      printf ("%s%3d%s: %s",
	      CT_OUT(GREEN),
	      m,
	      CT_OUT(NORM_DEF_COL),
	      CT_OUT(MAGENTA));

      for (int N = 0; N < _rng_N; N++)
	{
	  int off = (m - _min_m) * _rng_N + N;
	  int num = _entries[off];

	  printf ("%4d", num);
	}

      printf ("%s\n", CT_OUT(NORM_DEF_COL));
    }

}
