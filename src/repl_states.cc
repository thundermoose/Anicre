
#include <stdlib.h>
#include <string.h>

#include "error.hh"
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

