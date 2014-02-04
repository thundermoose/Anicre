
#ifndef __ANICR_TABLES_H__
#define __ANICR_TABLES_H__

typedef struct sp_state_info_t
{
  int _n;
  int _l;
  int _j;
  int _m;
} sp_state_info;

typedef int state_for_miss_m_N;
typedef int index_into_state_for_miss;

typedef struct info_state_for_miss_t
{
  int _m_min;
  int _m_steps;
  int _num_E;

  state_for_miss_m_N        *_miss;
  index_into_state_for_miss *_offset;

} info_state_for_miss;

#endif/*__ANICR_TABLES_H__*/
