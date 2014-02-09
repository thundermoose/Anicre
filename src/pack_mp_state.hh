
#ifndef __PACK_MP_STATE_HH__
#define __PACK_MP_STATE_HH__

struct pms_info
{
  char _bits;
  char _shift;
  char _word;
};

template<typename pack_T>
class pack_mp_state
{
public:
  pack_mp_state();
  virtual ~pack_mp_state();

public:
  pms_info *_items;

  int _words;
  int _len[2]; // A0, A1

public:
  void setup_pack(int A0, int A1,int *max_index0, int *max_index1);

};

#endif//__PACK_MP_STATE_HH__
