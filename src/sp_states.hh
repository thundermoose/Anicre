#ifndef __SP_STATES_HH__
#define __SP_STATES_HH__

#include "file_output.hh"

#include <stdio.h>

#include <vector>

struct nlj_state
{
public:
  nlj_state(int n, int l, int j)
  {
    _n = n;
    _l = l;
    _j = j;
  }

public:
  int _n;
  int _l;
  int _j;
};

typedef std::vector<nlj_state> vect_nlj_state;

void nlj_states_table(file_output &out, vect_nlj_state &nljs);

struct sp_state
{
public:
  sp_state(int n, int l, int j, int m, int nlj)
  {
    _n = n;
    _l = l;
    _j = j;
    _m = m;

    _nlj = nlj;
  }

public:
  int _n;
  int _l;
  int _j;
  int _m;

  int _nlj;
};

typedef std::vector<sp_state> vect_sp_state;

void sp_states_table(file_output &out, vect_sp_state &sps);

void twob_states_table(file_output &out, vect_nlj_state &nljs);
#endif/*__SP_STATES_HH__*/
