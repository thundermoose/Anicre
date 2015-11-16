#include "util.h"

#include "anicr_tables.h"
#include "anicr_config.h"
#include <math.h>
#include <gsl/gsl_errno.h>
#include "gsl/gsl_sf_coupling.h"
#include "gsl/gsl_sf_laguerre.h"
#include "gsl/gsl_integration.h"
#include "nlj.h"
#include <stdio.h>
#include <stdlib.h>

#include "caltbop.h"

#define MASSN 938.9187
#define HBARC 197.326963

int twob()
{
  printf("COMPUTE TWO-BODY OPERATORS\n");
  _nlj_items_nn = NULL;                                                                          
  _num_nlj_items_nn = 0;                                                                                 
  _nlj_items_pp = NULL;                                                                              
  _num_nlj_items_pp = 0;                                                                                 
  _nlj_items_np = NULL;                                                                              
  _num_nlj_items_np = 0;                                                                                 
  _nlj_items_pn = NULL;                                                                              
  _num_nlj_items_pn = 0;                                                                                 
  char filename_nn[14]="nlj_out-nn.bin";
  char filename_pp[14]="nlj_out-pp.bin";
  char filename_np[14]="nlj_out-np.bin";
  char filename_pn[14]="nlj_out-pn.bin";

  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;
  
  //  int num_jtrans=(jtrans_max-jtrans_min)/2+1;
  //  printf("Jtrans Max=%d Min=%d num=%d \n",jtrans_max,jtrans_min,num_jtrans);
 

  _nlj_items_nn=readDumpfile(filename_nn,&_num_nlj_items_nn);
  printf("Read NN-file\n");
  _nlj_items_pp=readDumpfile(filename_pp,&_num_nlj_items_pp);
  printf("Read PP-file\n");
#if NP_ORDER
  printf("NP order\n");
  (void)filename_pn;
  _nlj_items_np=readDumpfile(filename_np,&_num_nlj_items_np);

#else
  printf("PN order\n");
  (void)filename_np;
  _nlj_items_np=readDumpfile(filename_pn,&_num_nlj_items_np);
#endif
  printf("Read NP-file\n");

 
  
  int jtrans;
  int showJtrans;
  int mtrans = CFG_2M_INITIAL - CFG_2M_FINAL;
  if (abs(mtrans) > jtrans_max)
    {
      fprintf (stderr, "FIXME: abs(mtrans) > jtrans_max.\n");
      exit(1);
    }
  int Tab;
  int Jab;
  int pi1;
  int twob1=0;
  printf("Write two-body matrix elements.\n");
  
  printf("  Two-body transition matrix elements\n");
  printf("  ***********************************\n");

  printf(" number of two-nucleon states =%4d\n",CFG_SP_PAIRS); 
   for(pi1=1;pi1>=-1;pi1=pi1-2){
    for(Jab=0; Jab<=CFG_MAX_J;Jab++){
      for (Tab=0;Tab<=1;Tab++){
	for (int i1 = 0; i1<CFG_NUM_NLJ_STATES; i1++)
	  {
	    for(int i2 = i1; i2<CFG_NUM_NLJ_STATES; i2++)
	      { 
		int li1=_table_nlj_states[i1]._l;
		int li2=_table_nlj_states[i2]._l;
		int ni1=_table_nlj_states[i1]._n;
		int ni2=_table_nlj_states[i2]._n;
		int ji1=_table_nlj_states[i1]._j;
		int ji2=_table_nlj_states[i2]._j;
		if(ji1+ji2<2*Jab){continue;}
		if(abs(ji1-ji2)>2*Jab){continue;}
		if(pow(-1,(li1+li2))!=pi1){continue;}
		if(i1==i2 && pow(-1,(Jab+Tab))!=-1){continue;}
		if(2*ni1+li1+2*ni2+li2>CFG_MAX_SUM_E){continue;}
		
		twob1++;
	      }
	  }
      }
    }
  }
  printf(" number of two-nucleon states =%5d\n",twob1++);
  twob1=0;
   
  for(pi1=1;pi1>=-1;pi1=pi1-2){
    for(Jab=0; Jab<=CFG_MAX_J;Jab++){
      for (Tab=0;Tab<=1;Tab++){
	for (int i1 = 0; i1<CFG_NUM_NLJ_STATES; i1++)
	  {
	    for(int i2 = i1; i2<CFG_NUM_NLJ_STATES; i2++)
	      { 
		int li1=_table_nlj_states[i1]._l;
		int li2=_table_nlj_states[i2]._l;
		int ni1=_table_nlj_states[i1]._n;
		int ni2=_table_nlj_states[i2]._n;
		int ji1=_table_nlj_states[i1]._j;
		int ji2=_table_nlj_states[i2]._j;
		if(ji1+ji2<2*Jab){continue;}
		if(abs(ji1-ji2)>2*Jab){continue;}
		if(pow(-1,(li1+li2))!=pi1){continue;}
		if(i1==i2 && pow(-1,(Jab+Tab))!=-1){continue;}
		if(2*ni1+li1+2*ni2+li2>CFG_MAX_SUM_E){continue;}
		
		twob1++;
		printf(" #%5d  a b J T= %2d %2d  %1d %1d \n",twob1,i1+1,i2+1,Jab,Tab);
	      }
	  }
      }
    }
  }

  printf("\n\n *** Transition matrix elements for states: ***\n");
 //loop over all states. 

 
 for (jtrans = jtrans_min; jtrans <= jtrans_max; jtrans += 2)
    {
      showJtrans=1;
      
      double mult;
      {
	gsl_sf_result result;
	
	int ret =
	  gsl_sf_coupling_3j_e(CFG_2J_INITIAL,  jtrans,  CFG_2J_FINAL,
			       CFG_2M_INITIAL,  mtrans, -CFG_2M_FINAL,
			       &result);
	if (ret != GSL_SUCCESS)
	  {
	    fprintf (stderr,"ERR! %d\n", ret);
	     exit(1); 
	  }
	int sign = 1 - ((CFG_2J_INITIAL - jtrans + CFG_2M_FINAL) & 2);
       
	mult = 1.0 / (result.val) * sign; 
	if(result.val==0.0){continue;}
      }
      twob1=0;
      int twob2=0;
   
      for(pi1=1;pi1>=-1;pi1=pi1-2){
	for(Jab=0; Jab<=CFG_MAX_J;Jab++){
	  for (Tab=0;Tab<=1;Tab++){
	    for (int i1 = 0; i1<CFG_NUM_NLJ_STATES; i1++){
	      for(int i2 = i1; i2<CFG_NUM_NLJ_STATES; i2++){
		int li1=_table_nlj_states[i1]._l;
		int li2=_table_nlj_states[i2]._l;
		int ni1=_table_nlj_states[i1]._n;
		int ni2=_table_nlj_states[i2]._n;
		int ji1=_table_nlj_states[i1]._j;
		int ji2=_table_nlj_states[i2]._j;
		if(ji1+ji2<2*Jab){continue;}
		if(abs(ji1-ji2)>2*Jab){continue;}
		if(pow(-1,(li1+li2))!=pi1){continue;}
		if(i1==i2 && pow(-1,(Jab+Tab))!=-1){continue;}
		if(2*ni1+li1+2*ni2+li2>CFG_MAX_SUM_E){continue;}
		
		twob1++;
		twob2=0;
		for(int pi2=1;pi2>=-1;pi2=pi2-2){
		  for(int Jcd=0; Jcd<=CFG_MAX_J;Jcd++){
		    for (int Tcd=0;Tcd<=1;Tcd++){
		      for (int j1 = 0; j1<CFG_NUM_NLJ_STATES; j1++){
			for(int j2 = j1; j2<CFG_NUM_NLJ_STATES; j2++) {
			  int lj1=_table_nlj_states[j1]._l;
			  int lj2=_table_nlj_states[j2]._l;
			  int nj1=_table_nlj_states[j1]._n;
			  int nj2=_table_nlj_states[j2]._n;
			  int jj1=_table_nlj_states[j1]._j;
			  int jj2=_table_nlj_states[j2]._j;
			  if(jj1+jj2<2*Jcd){continue;}
			  if(abs(jj1-jj2)>2*Jcd){continue;}
			  if(pow(-1,(lj1+lj2))!=pi2){continue;}
			  if(j1==j2 && pow(-1,(Jcd+Tcd))!=-1){continue;}
			  if(2*nj1+lj1+2*nj2+lj2>CFG_MAX_SUM_E){continue;}
			  
			  twob2++;
			  double value_nn=findState2(_nlj_items_nn, _num_nlj_items_nn, i1, i2,  j1, j2, 2*Jab,2*Jcd,jtrans);
			  double value_pp=findState2(_nlj_items_pp, _num_nlj_items_pp, i1, i2,  j1, j2, 2*Jab,2*Jcd,jtrans);
			  double value_np=findState2(_nlj_items_np, _num_nlj_items_np, i1, i2,  j1, j2, 2*Jab,2*Jcd,jtrans);  
			  double rev1_np=0.0;
			  double rev2_np=0.0;
			  double rev3_np=0.0;
			  
			  if(i1!=i2){
			    rev1_np=findState2(_nlj_items_np,_num_nlj_items_np,i2,i1,j1,j2,2*Jab,2*Jcd,jtrans);  
			    if(rev1_np!=0.0){rev1_np=rev1_np*pow(-1.,-(ji1+ji2)/2+Jab+Tab-1);}
			  }
			  
			  if(j2!=j1){  
			    rev2_np=findState2(_nlj_items_np,_num_nlj_items_np,i1,i2,j2,j1,2*Jab,2*Jcd,jtrans);
			    if(rev2_np!=0.0){rev2_np=rev2_np*pow(-1.,-(jj1+jj2)/2+Jcd+Tcd-1);}
			  }
					  
			  if(i1!=i2&&j2!=j1){
			    rev3_np=findState2(_nlj_items_np,_num_nlj_items_np,i2,i1,j2,j1,2*Jab,2*Jcd,jtrans);
			    if(rev3_np!=0.0){rev3_np=rev3_np*pow(-1.,-(ji1+ji2+jj1+jj2)/2+Jab+Jcd+Tab+Tcd); }
			    
			  }
		   
			  double clebsch_nn=0.0;
			  double clebsch_pp=0.0;
			  double clebsch_np=0.0;
			  
			  if(Tab==1 && Tcd==1){
			    clebsch_nn=1.0;
			    clebsch_pp=1.0;
			  }
			  else{
			    clebsch_nn=0.0;
			    clebsch_pp=0.0;
			  }
					 
			  clebsch_np=gsl_sf_coupling_3j(1,1,2*Tab,1,-1,0)*gsl_sf_coupling_3j(1,1,2*Tcd,1,-1,0)*sqrt(2*Tab+1)*sqrt(2*Tcd+1);
				       
			  double Nab=norm(ni1,li1,ji1,ni2,li2,ji2,Jab,Tab);
			  double Ncd=norm(nj1,lj1,jj1,nj2,lj2,jj2,Jcd,Tcd);
			 
			  value_nn=value_nn*mult*clebsch_nn*Nab*Ncd;
			  value_pp=value_pp*mult*clebsch_pp*Nab*Ncd;
			  value_np=(value_np+rev1_np+rev2_np+rev3_np)*mult*clebsch_np*Nab*Ncd;
				  

			  if ((fabs(value_np)>0.000001) ||( fabs(value_pp)>0.000001) ||( fabs(value_nn)>0.000001) ){   //roundoff-error?
			    if(showJtrans==1){
			      printf ("\n Jtrans= %2d\n", jtrans/2);
			      showJtrans=0;
			    }
#if NP_ORDER
			    
			    printf(" (a+a+)J=%5d  (a-a-)J=%5d   td: np=%10.6f   pp=%10.6f   nn=%10.6f\n",twob1,twob2, value_np, value_pp,value_nn);//, Jab, Tab, Jcd,Tcd);
#else 
			    printf(" (a+a+)J=%5d  (a-a-)J=%5d   td: pn=%10.6f   pp=%10.6f   nn=%10.6f\n",twob1,twob2, value_np, value_pp,value_nn);//, Jab, Tab, Jcd,Tcd);
#endif
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }

  free (_nlj_items_nn);
  free (_nlj_items_pp);
  free (_nlj_items_np);

  return 0;
}

int readTBME(){

  printf("Read TBME.int\n");

  return 0;
}
