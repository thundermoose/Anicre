#include "sp_pair_use.hh"
#include "error.hh"

void sp_pair_use::alloc(size_t n1, size_t n2)
{
  size_t sz_line_n2 =
    (n2 + BITSONE_CONTAINER_BITS-1) / BITSONE_CONTAINER_BITS;

  size_t sz_line_n1 =
    n1 * sz_line_n2;

  _used = (BITSONE_CONTAINER_TYPE *)
    malloc (sizeof (BITSONE_CONTAINER_TYPE) * sz_line_n1);

  if (!_used)
    FATAL("Memory allocation error (sp_pair_use::_used, %zd bytes).",
	  sizeof (BITSONE_CONTAINER_TYPE) * sz_line_n1);

  _n1 = n1;
  _n2 = n2;
  _sz_line_n2 = sz_line_n2;
}
