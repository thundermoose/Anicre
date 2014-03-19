#ifndef __SP_PAIR_USE_HH__
#define __SP_PAIR_USE_HH__

#include <stdlib.h>
#include <assert.h>

#define BITSONE_CONTAINER_TYPE    unsigned long
#define BITSONE_CONTAINER_BITS    (sizeof(BITSONE_CONTAINER_TYPE)*8)

class sp_pair_use
{

public:
  size_t _n1, _n2;
  size_t _sz_line_n2;

  BITSONE_CONTAINER_TYPE *_used;

public:
  void alloc(size_t n1, size_t n2);

public:
  void add(unsigned int i1, unsigned int i2)
  {
    assert (i1 < _n1);
    assert (i2 < _n2);

    BITSONE_CONTAINER_TYPE mask2 =
      ((BITSONE_CONTAINER_TYPE) 1) <<
      (i2 % (BITSONE_CONTAINER_BITS));
    size_t offset2 = i2 / (BITSONE_CONTAINER_BITS);

    _used[i1 * _sz_line_n2 + offset2] |= mask2;
  }

};

#endif/*__SP_PAIR_USE_HH__*/
