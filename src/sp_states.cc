
#include "sp_states.hh"

void sp_states_table(file_output &out, vect_sp_state &sps)
{
  out.fprintf("/********************************************/\n");
  out.fprintf("/* Table.  sp_states: %7zd               */\n",
	      sps.size());
  out.fprintf("\n");
  out.fprintf("sp_state_info _table_sp_states[] =\n");
  out.fprintf("{\n");
  out.fprintf("  /*       i   N        n    l    j    m */\n");
  out.fprintf("\n");
  
  for (size_t i = 0; i < sps.size(); i++)
    {
      const sp_state &sp = sps[i];

      out.fprintf("  /* %7zd %3d */ { %3d, %3d, %3d, %3d },\n",
		  i, 2 * sp._n + sp._l,
		  sp._n, sp._l, sp._j, sp._m);
    }
  
  out.fprintf("};\n");
  out.fprintf("\n");
}

