
#ifndef __REPL_STATES_HH__
#define __REPL_STATES_HH__

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
