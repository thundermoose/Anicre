
#ifndef __ANICR_TABLES_H__
#define __ANICR_TABLES_H__

#include <stdint.h>

/**********************************************************************/

typedef struct nlj_state_info_t
{
  int _n;
  int _l;
  int _j;
} nlj_state_info;

typedef struct sp_state_info_t
{
  int _n;
  int _l;
  int _j;
  int _m;

  int _nlj;
} sp_state_info;

typedef uint32_t state_for_miss_m_N;
typedef int index_into_state_for_miss;

typedef struct info_state_for_miss_t
{
  int _m_min;
  int _m_steps;
  int _m_stride;
  int _num_E;
  int _parity_stride;

  state_for_miss_m_N        *_miss;
  index_into_state_for_miss *_offset;

} info_state_for_miss;

typedef struct mp_pack_info_t
{
  int _word;
  int _shift;
  uint64_t _mask;
} mp_pack_info;

/**********************************************************************/

extern nlj_state_info _table_nlj_states[];
extern sp_state_info  _table_sp_states[];

extern info_state_for_miss _table_1_0_info;
extern info_state_for_miss _table_2_0_info;
extern info_state_for_miss _table_3_0_info;

extern mp_pack_info _mp_pack_info[];
extern mp_pack_info _mp_pack_info[];

/**********************************************************************/

#endif/*__ANICR_TABLES_H__*/
