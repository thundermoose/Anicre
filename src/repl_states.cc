
#include <stdlib.h>
#include <string.h>

#include "error.hh"
#include "colourtext.hh"

#include "repl_states.hh"

repl_states_by_m_N::repl_states_by_m_N(int min_m, int max_m, int max_N,
				       int miss1, int miss2)
{
  _miss1 = miss1;
  _miss2 = miss2;

  _min_m = min_m;
  _rng_m = max_m - min_m + 1;
  _rng_N = max_N + 1;

  _sz_parity = _rng_m * _rng_N;

  size_t sz = 2 * _sz_parity;

  _entries = new vect_int[sz];
}

repl_states_by_m_N::~repl_states_by_m_N()
{
  delete[] _entries;
}

void repl_states_by_m_N::dump() const
{
  for (int parity = 0; parity < 2; parity++)
    {
      printf ("%sp%s=%s%d%s\n",
	      CT_OUT(BOLD_BLUE), CT_OUT(NORM_DEF_COL),
	      CT_OUT(GREEN), parity, CT_OUT(NORM_DEF_COL));
      printf ("%sm%s\\%sN%s  %s",
	      CT_OUT(BOLD_BLUE), CT_OUT(NORM_DEF_COL),
	      CT_OUT(BOLD_BLUE), CT_OUT(NORM_DEF_COL),
	      CT_OUT(GREEN));
      for (int N = 0; N < _rng_N; N++)
	printf ("%4d",N);
      printf ("%s\n",CT_OUT(NORM_DEF_COL));

      for (int m = _min_m; m < _min_m + _rng_m; m += 2)
	{
	  printf ("%s%3d%s: %s",
		  CT_OUT(GREEN),
		  m,
		  CT_OUT(NORM_DEF_COL),
		  CT_OUT(MAGENTA));

	  for (int N = 0; N < _rng_N; N++)
	    {
	      int off = (m - _min_m) * _rng_N + N;
	      vect_int &vect = _entries[parity * _sz_parity + off];
	      size_t num = vect.size();

	      if (num)
		printf ("%4zd", num);
	      else
		printf ("%4s", "-");
	    }

	  printf ("%s\n", CT_OUT(NORM_DEF_COL));
	}
    }

}

void repl_states_by_m_N::write_table(file_output &out) const
{
  size_t sz = _rng_m * _rng_N;

  size_t *offset = new size_t[sz+1];

  out.fprintf("/********************************************/\n");
  out.fprintf("/* Table.  min_m: %3d max_m: %3d max_N: %3d */\n",
	      _min_m, _min_m + _rng_m - 1, _rng_N - 1);
  out.fprintf("\n");
  out.fprintf("state_for_miss_m_N _table_%d_%d_miss[] =\n",
	      _miss1, _miss2);
  out.fprintf("{\n");
  out.fprintf("  /*   m  N [num] */\n");
  out.fprintf("\n");

  size_t totoffset = 0;

  for (int m = _min_m; m < _min_m + _rng_m; m += 2)
    {
      out.fprintf("  /* %3d          */\n", m);

      for (int N = 0; N < _rng_N; N++)
	{
	  int off = (m - _min_m) * _rng_N + N;
	  vect_int &vect = _entries[off];
	  size_t num = vect.size();

	  offset[off] = totoffset;

	  if (!num)
	    continue;

	  out.fprintf("  /*     %2d [%3zd] */", N, num);
	  size_t linesz = 20;

	  for (size_t i = 0; i < num; i++)
	    {
	      char str[32];

	      sprintf (str, " %3d,", vect[i]);
	      size_t itemsz = strlen(str);

	      if (strlen(str) + linesz > 79)
		{
		  //       12345678901234567890
		  out.fprintf("\n                    ");
		  linesz = 20;
		}
	      linesz += itemsz;
	      out.fprintf("%s", str);
	    }

	  out.fprintf("\n");

	  totoffset += num;
	}

      int off = (m - _min_m) * _rng_N + _rng_N;

      offset[off] = totoffset;
    }

  // offset[sz] = totoffset;

  out.fprintf("};\n");
  out.fprintf("\n");


  out.fprintf("\n");
  out.fprintf("index_into_state_for_miss _table_%d_%d_offset[] =\n",
	      _miss1, _miss2);
  out.fprintf("{\n");
  out.fprintf("  /*   m */\n");
  out.fprintf("\n");

  for (int m = _min_m; m < _min_m + _rng_m; m += 2)
    {
      out.fprintf("  /* %3d */", m);

      for (int N = 0; N < _rng_N+1; N++)
	{
	  int off = (m - _min_m) * _rng_N + N;

	  out.fprintf(" %4zd,", offset[off]);
	}
      out.fprintf("\n");
    }
  out.fprintf("  /*     */ %4zd\n", offset[sz]);
  out.fprintf("};\n");
  out.fprintf("\n");

  out.fprintf("info_state_for_miss _table_%d_%d_info =\n",
	      _miss1, _miss2);
  out.fprintf("{\n");
  out.fprintf("  %d, %d, %d,\n", _min_m, _rng_m, _rng_N);
  out.fprintf("  _table_%d_%d_miss,\n", _miss1, _miss2);
  out.fprintf("  _table_%d_%d_offset,\n", _miss1, _miss2);
  out.fprintf("};\n");

  out.fprintf("\n");
  out.fprintf("/********************************************/\n");
  out.fprintf("\n");
}
