
#include "sp_states.hh"

void sp_states_table(FILE *fid, vect_sp_state &sps)
{
  fprintf (fid,"/********************************************/\n");
  fprintf (fid,"/* Table.  sp_states: %7zd               */\n",
	   sps.size());
  fprintf (fid,"\n");
  fprintf (fid,"sp_state _table_sp_states[] =\n");
  fprintf (fid,"{\n");
  fprintf (fid,"  /*       i   N        n    l    j    m */\n");
  fprintf (fid,"\n");
  
  for (size_t i = 0; i < sps.size(); i++)
    {
      const sp_state &sp = sps[i];

      fprintf (fid,"  /* %7zd %3d */ { %3d, %3d, %3d, %3d },\n",
	       i, 2 * sp._n + sp._l,
	       sp._n, sp._l, sp._j, sp._m);
    }
  
  fprintf (fid,"};\n");
  fprintf (fid,"\n");
}

