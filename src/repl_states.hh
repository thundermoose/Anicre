
#ifndef __REPL_STATES_HH__
#define __REPL_STATES_HH__

#include <limits.h>
#include <assert.h>

#include <vector>

typedef std::vector<int> vect_int;

class repl_states_by_m_N
{
public:
  repl_states_by_m_N(int min_m, int max_m, int max_N);
  virtual ~repl_states_by_m_N();

public:
  int _min_m, _rng_m;
  int _rng_N;

  vect_int *_entries;

public:
  bool has_entry(int m, int N)
  {
    assert(m >= _min_m && (m - _min_m) < _rng_m && N < _rng_N);
    int off = (m - _min_m) * _rng_N + N;
    vect_int &vect = _entries[off];
    return vect.size() != 0;
  }

  int min_N(int m, int i) const
  {
    if (m < _min_m ||
	m - _min_m >= _rng_m)
      return INT_MAX;

    int off = (m - _min_m) * _rng_N;

    for (int N = 0; N < _rng_N; N++)
      {
	vect_int &vect = _entries[off+N];

	/* For us to accept this, we must have a state in the list
	 * with index > i.
	 */

	for (size_t j = 0; j < vect.size(); j++)
	  if (vect[j] > i)
	    return N;
      }

    return INT_MAX;
  }

  void add_entry(int m, int N, int i)
  {
    assert(m >= _min_m && (m - _min_m) < _rng_m && N < _rng_N);
    int off = (m - _min_m) * _rng_N + N;

    vect_int &vect = _entries[off];

    vect.push_back(i);
  }

public:
  void dump() const;


};

#endif/*__REPL_STATES_HH__*/
