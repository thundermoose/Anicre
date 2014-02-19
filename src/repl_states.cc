
#include <stdlib.h>
#include <string.h>
#include <algorithm>

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
  // Trickery to make even and odd parity tables always equally long
  _rng_N = ((max_N + 1) & ~1) + 1;

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

void repl_states_by_m_N::write_table(file_output &out,
				     bool continuationlist) const
{
  size_t sz = _rng_m * (_rng_N + 1);

  size_t *offset = new size_t[2 * sz + 1];
  size_t *contstart = new size_t[2 * sz]; /* a few too many, does not matter */
  size_t *contend = new size_t[2 * sz];

#define OFFSET(parity,m,N)    offset[(parity)*sz+((m)-_min_m)*(_rng_N+1)+(N)]
#define CONTSTART(parity,m,N) contstart[(parity)*sz+((m)-_min_m)*(_rng_N)+(N)]
#define CONTEND(parity,m,N)   contend  [(parity)*sz+((m)-_min_m)*(_rng_N)+(N)]

  size_t totoffset = 0;

  /* For two and more particles, it often occurs that possible
   * particles for a higher energy has smaller indices than particles
   * for a slightly lower energy.  Even though the sp states are
   * ordered by energy.  This seems to come about due to the ability
   * to make more or less excursions in m.  The effect also seems not
   * to be curable by an more advanced sorting.  There are cases when
   * sp state 0 and 1 are in one order, and then the other.
   *
   * We deal with this by letting the main list only eject entries as
   * far as is safe while keeping ordering.  Each energy then also
   * have a small continuation list.
   */

  vect_int *_continuation = new vect_int[_rng_N];

  out.fprintf("/********************************************/\n");
  out.fprintf("/* Table.  min_m: %3d max_m: %3d max_N: %3d */\n",
	      _min_m, _min_m + _rng_m - 1, _rng_N - 1);
  out.fprintf("\n");
  out.fprintf("state_for_miss_m_N _table_%d_%d_miss[] =\n",
	      _miss1, _miss2);
  out.fprintf("{\n");
  for (int parity = 0; parity < 2; parity++)
    {
      if (parity)
	out.fprintf("\n");
      out.fprintf("  /*   m  N [num] */  /* parity = %d */\n", parity);
      out.fprintf("\n");

      for (int m = _min_m; m < _min_m + _rng_m; m += 2)
	{
	  out.fprintf("  /* %3d          */\n", m);
	  
	  vect_int remain_states[2];
	  size_t prev_used = 0;

	  for (int N = 0; N < _rng_N; N++)
	    {
	      int off = (m - _min_m) * _rng_N + N;
	      vect_int &vect = _entries[parity * _sz_parity + off];
	      size_t num = vect.size();
	      
	      OFFSET(parity,m,N) = totoffset;
	      CONTSTART(parity,m,N) = 0;
	      CONTEND(parity,m,N) = 0;
	      
	      vect_int &remain_this = remain_states[  N & 1 ];
	      vect_int &remain_prev = remain_states[!(N & 1)];

	      num += remain_prev.size() - prev_used;

	      remain_this.resize(num);

	      std::merge (remain_prev.begin() + prev_used, remain_prev.end(),
			  vect.begin(), vect.end(),
			  remain_this.begin());

	      prev_used = 0;

	      if (N && ((N ^ parity) & 1))
		{
		  if (vect.size())
		    FATAL("Internal error excluding useless "
			  "energy steps in tables.");
		  continue;
		}

	      if (!num)
		continue;

	      /* What is the lowest index of any following state.
	       * (any would be next in 2-particle case, two next in
	       * 3-particle case and so on, but lets be lazy...)
	       */

	      int ordered_to = INT_MAX;

	      for (int N_next = N + 1; N_next < _rng_N; N_next++)
		{
		  int off_next = (m - _min_m) * _rng_N + N_next;
		  vect_int &vect_next =
		    _entries[parity * _sz_parity + off_next];
		  size_t num_next = vect_next.size();

		  if (!num_next)
		    continue;

		  if (vect_next[0] < ordered_to)
		    ordered_to = vect_next[0];
		}

	      out.fprintf("  /*     %2d [%3zd] */", N, num);
	      size_t linesz = 20;

	      prev_used = num; /* unless aborted below */

	      for (size_t i = 0; i < num; i++)
		{
		  int sp = remain_this[i];

		  if (sp >= ordered_to)
		    {
		      _continuation[N].resize(num - i);
		      std::copy(remain_this.begin() + i,
				remain_this.end(),
				_continuation[N].begin());
		      prev_used = i;
		      break;
		    }

		  char str[32];
		  
		  sprintf (str, " %3d,", sp);
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
	      
	      totoffset += prev_used;
	    }

	  OFFSET(parity,m,_rng_N) = totoffset;

	  for (int N = 0; N < _rng_N; N++)
	    {
	      vect_int &vect = _continuation[N];
	      size_t num = vect.size();

	      if (!num)
		continue;

	      if (!continuationlist)
		FATAL("Internal error, unexpected continuation items.");

	      out.fprintf("  /* cnt %2d [%3zd] */", N, num);
	      size_t linesz = 20;

	      CONTSTART(parity,m,N) = totoffset;

	      for (size_t i = 0; i < num; i++)
		{
		  int sp = vect[i];

		  char str[32];

		  sprintf (str, " %3d,", sp);
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

	      CONTEND(parity,m,N) = totoffset;

	      _continuation[N].resize(0);
	    }
	 
	}
    }

  offset[2 * sz] = totoffset;

  delete[] _continuation;
      
  // offset[sz] = totoffset;

  out.fprintf("};\n");
  out.fprintf("\n");


  out.fprintf("\n");
  out.fprintf("index_into_state_for_miss _table_%d_%d_offset[] =\n",
	      _miss1, _miss2);
  out.fprintf("{\n");

  for (int parity = 0; parity < 2; parity++)
    {
      if (parity)
        out.fprintf("\n");
      out.fprintf("  /* parity = %d */\n", parity);
      out.fprintf("\n");
      out.fprintf("  /*   m \\ N");
      for (int N = 0; N < _rng_N; N++)
	{
	  if (N && ((N ^ parity) & 1))
	    continue;
	  out.fprintf("%4d  %s", N, continuationlist ? "            " : "");
	}
      out.fprintf("*/\n");
      out.fprintf("\n");
      
      for (int m = _min_m; m < _min_m + _rng_m; m += 2)
	{
	  out.fprintf("  /* %3d */", m);

	  for (int N = 0; N < _rng_N; N++)
	    {
	      if (N && ((N ^ parity) & 1))
		{
		  if (OFFSET(parity,m,N) !=
		      OFFSET(parity,m,N+1))
		    FATAL("Internal error excluding useless "
			  "energy steps in tables.");
		  continue;
		}
	      
	      out.fprintf(" %4zd,", OFFSET(parity,m,N));

	      if (continuationlist)
		{
		  size_t contoffset =
		    CONTSTART(parity,m,N) - OFFSET(parity,m,N+1);
		  size_t contlength =
		    CONTEND(parity,m,N) - CONTSTART(parity,m,N);
		  if (!contlength)
		    contoffset = 0;

		  if (contoffset > ((1 << 18) - 1))
		    FATAL("Error, contoffset will overflow CL item.");
		  if (contlength > ((1 << 14) - 1))
		    FATAL("Error, contlength will overflow CL item.");

		  out.fprintf("CL(%3zd,%3zd),", contoffset, contlength);

		}
	    }
	  out.fprintf(" %s%4zd%s\n",
		      continuationlist ? "" : "/* ",
		      OFFSET(parity,m,_rng_N),
		      continuationlist ? "," : " */");
	}
    }
  out.fprintf("\n");
  if (!continuationlist)
    out.fprintf("  /*     */ %4zd\n", offset[2 * sz]);
  out.fprintf("};\n");
  out.fprintf("\n");

  delete[] offset;
  delete[] contstart;
  delete[] contend;

  int m_stride = continuationlist ? _rng_N + 2 : _rng_N / 2 + 1;
  int parity_stride = ((_rng_m+1) / 2) * m_stride;

  out.fprintf("info_state_for_miss _table_%d_%d_info =\n",
	      _miss1, _miss2);
  out.fprintf("{\n");
  out.fprintf("  %d, %d, %d, %d, %d,\n",
	      _min_m, _rng_m, m_stride,
	      _rng_N, parity_stride);
  out.fprintf("  _table_%d_%d_miss,\n", _miss1, _miss2);
  out.fprintf("  _table_%d_%d_offset,\n", _miss1, _miss2);
  out.fprintf("};\n");

  out.fprintf("\n");
  out.fprintf("/********************************************/\n");
  out.fprintf("\n");
}
