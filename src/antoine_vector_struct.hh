
#ifndef __ANTOINE_VECTOR_STRUCT_HH__
#define __ANTOINE_VECTOR_STRUCT_HH__

#include "antoine_struct.hh"

#include <stdint.h>

struct mr_antoine_vector_header_t
{
  // No idea why 60, would have thought 30...
  char text2[60];
};

struct mr_antoine_vector_info1_t
{
  mr_antoine_fon_new_t fon;
  double    ze;
  uint32_t  ncut;
  uint32_t  sht;
};

struct mr_antoine_cut_item_t // ncut
{
  uint32_t x;
};

#endif/*__ANTOINE_VECTOR_STRUCT_HH__*/
