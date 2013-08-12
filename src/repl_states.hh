
#ifndef __REPL_STATES_HH__
#define __REPL_STATES_HH__

#include <limits.h>

class repl_states_by_m_N
{
public:
  repl_states_by_m_N(int min_m, int max_m, int max_N);

public:
  int _min_m, _rng_m;
  int _rng_N;

  int *_entries;

public:
  bool has_entry(int m, int N)
  {
    assert(m >= _min_m && (m - _min_m) < _rng_m && N < _rng_N);
    int off = (m - _min_m) * _rng_N + N;
    return _entries[off] != 0;
  }

  int min_N(int m)
  {
    if (m < _min_m ||
	m - _min_m >= _rng_m)
      return INT_MAX;

    int off = (m - _min_m) * _rng_N;

    for (int N = 0; N < _rng_N; N++)
      if (_entries[off+N])
	return N;

    return INT_MAX;
  }

  void add_entry(int m, int N)
  {
    assert(m >= _min_m && (m - _min_m) < _rng_m && N < _rng_N);
    int off = (m - _min_m) * _rng_N + N;
    _entries[off]++;
  }

public:
  void dump();


};

#endif/*__REPL_STATES_HH__*/
