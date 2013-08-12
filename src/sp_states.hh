#ifndef __SP_STATES_HH__
#define __SP_STATES_HH__

struct sp_state
{
public:
  sp_state(int n, int l, int j, int m)
  {
    _n = n;
    _l = l;
    _j = j;
    _m = m;
  }

public:
  int _n;
  int _l;
  int _j;
  int _m;
};

#endif/*__SP_STATES_HH__*/
