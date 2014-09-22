
#include "sp_states.hh"
#include <cmath>
void nlj_states_table(file_output &out, vect_nlj_state &nljs)
{
  out.fprintf("/********************************************/\n");
  out.fprintf("/* Table.  nlj_states: %7zd               */\n",
	      nljs.size());
  out.fprintf("\n");
  out.fprintf("nlj_state_info _table_nlj_states[] =\n");
  out.fprintf("{\n");
  out.fprintf("  /*       i   N        n    l   2j */\n");
  out.fprintf("\n");
  
  for (size_t i = 0; i < nljs.size(); i++)
    {
      const nlj_state &nlj = nljs[i];

      out.fprintf("  /* %7zd %3d */ { %3d, %3d, %3d },\n",
		  i, 2 * nlj._n + nlj._l,
		  nlj._n, nlj._l, nlj._j);
    }
  
  out.fprintf("};\n");
  out.fprintf("\n");
}

void sp_states_table(file_output &out, vect_sp_state &sps)
{
  out.fprintf("/********************************************/\n");
  out.fprintf("/* Table.  sp_states: %7zd               */\n",
	      sps.size());
  out.fprintf("\n");
  out.fprintf("sp_state_info _table_sp_states[] =\n");
  out.fprintf("{\n");
  out.fprintf("  /*       i   N        n    l   2j   2m  nlj */\n");
  out.fprintf("\n");
  
  for (size_t i = 0; i < sps.size(); i++)
    {
      const sp_state &sp = sps[i];

      out.fprintf("  /* %7zd %3d */ { %3d, %3d, %3d, %3d, %3d },\n",
		  i, 2 * sp._n + sp._l,
		  sp._n, sp._l, sp._j, sp._m, sp._nlj);
    }
  
  out.fprintf("};\n");
  out.fprintf("\n");
}

void twob_states_table(file_output &out, vect_nlj_state &nljs)
{
    int Jmax=2;
    int Jmin=0;
    out.fprintf("/********************************************/\n");
    //  out.fprintf("/* Table.  nlj_states: %7zd               */\n",
      //      nljs.size());
      out.fprintf("\n");
  // out.fprintf("nlj_state_info _table_nlj_states[] =\n");
      out.fprintf("{\n");
  //  out.fprintf("  /*       i   N        n    l   2j */\n");
      out.fprintf("TWO_BODY\n");
    int ind=0;
    for (int pi=-1; pi<=1; pi=pi+2){
        out.fprintf("%2d",pi);
        for (int J=Jmin; J<=Jmax; J++)
          {
	      out.fprintf("J %3d \n",J);
	      for (int T=-1;T<=1;T++)
	        {
	    for (size_t i = 0; i < nljs.size(); i++)
	        {
		    for (size_t jj=i; jj <nljs.size(); jj++)
		     
		        {
		    const nlj_state &nlj1 = nljs[i];
		    const nlj_state &nlj2 = nljs[jj];
		    if(pow(-1,(nlj1._l+nlj2._l))==pi){
		        if((nlj1._j+nlj2._j)>J){continue;}
		        //energy
			  //symmetry i==jj 
			  if(i==jj && pow(-1,(J+T))==-1)
			    {
			      continue;}
		        out.fprintf("  {%3d, %3lu, %3lu, %3d, %3d, %3d },\n",
				           ind,i,jj, J,T,pi);
		        ind++;
		      }
		          }
		  }
	          }
	    }
      }
   out.fprintf("};\n");
         out.fprintf("\n");
  }
