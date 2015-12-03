
#include "util.h"

#include "anicr_tables.h"
#include "anicr_config.h"

#include "tmp_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <gsl/gsl_errno.h>
#include "gsl/gsl_sf_coupling.h"

#include "tmp_config.h"

typedef struct nlj_hash_item_t
{
  uint64_t _key;
  double   _value;
} nlj_hash_item;

nlj_hash_item *_nlj_items_nn = NULL;
size_t     _num_nlj_items_nn = 0;
nlj_hash_item *_nlj_items_pp = NULL;
size_t     _num_nlj_items_pp = 0;
nlj_hash_item *_nlj_items_np = NULL;
size_t     _num_nlj_items_np = 0;
nlj_hash_item *_nlj_items_pn = NULL;
size_t     _num_nlj_items_pn = 0;

int compare_nlj_item(const void *p1, const void *p2)
{
  uint64_t key1 = ((const nlj_hash_item *) p1)->_key;
  uint64_t key2 = ((const nlj_hash_item *) p2)->_key;

  int k1_nlj_a1, k1_nlj_a2, k1_nlj_c1, k1_nlj_c2;
  int k1_anni_j, k1_crea_j;
  int k1_jtrans;

  k1_nlj_a1 = (key1 >>  0) & 0x7ff;
  k1_nlj_a2 = (key1 >> 11) & 0x7ff;
  k1_nlj_c1 = (key1 >> 22) & 0x7ff;
  k1_nlj_c2 = (key1 >> 33) & 0x7ff;
  k1_anni_j = (key1 >> 44) &  0x7f;
  k1_crea_j = (key1 >> 51) &  0x7f;
  k1_jtrans = (int) (key1 >> 58);

  int k1_a1_l = _table_nlj_states[k1_nlj_a1]._l;
  int k1_a2_l = _table_nlj_states[k1_nlj_a2]._l;
  int k1_c1_l = _table_nlj_states[k1_nlj_c1]._l;
  int k1_c2_l = _table_nlj_states[k1_nlj_c2]._l;

  int k1_a_parity = (k1_a1_l ^ k1_a2_l) & 1;
  int k1_c_parity = (k1_c1_l ^ k1_c2_l) & 1;

  int k2_nlj_a1, k2_nlj_a2, k2_nlj_c1, k2_nlj_c2;
  int k2_anni_j, k2_crea_j;
  int k2_jtrans;

  k2_nlj_a1 = (key2 >>  0) & 0x7ff;
  k2_nlj_a2 = (key2 >> 11) & 0x7ff;
  k2_nlj_c1 = (key2 >> 22) & 0x7ff;
  k2_nlj_c2 = (key2 >> 33) & 0x7ff;
  k2_anni_j = (key2 >> 44) &  0x7f;
  k2_crea_j = (key2 >> 51) &  0x7f;
  k2_jtrans = (int) (key2 >> 58);

  int k2_a1_l = _table_nlj_states[k2_nlj_a1]._l;
  int k2_a2_l = _table_nlj_states[k2_nlj_a2]._l;
  int k2_c1_l = _table_nlj_states[k2_nlj_c1]._l;
  int k2_c2_l = _table_nlj_states[k2_nlj_c2]._l;

  int k2_a_parity = (k2_a1_l ^ k2_a2_l) & 1;
  int k2_c_parity = (k2_c1_l ^ k2_c2_l) & 1;

#define COMPARE_RET_DIFF(x,y) {if ((x) != (y)) return ((x) < (y)) ? -1 : 1; }

  COMPARE_RET_DIFF(k1_jtrans, k2_jtrans);
  COMPARE_RET_DIFF(k1_c_parity, k2_c_parity);
  COMPARE_RET_DIFF(k1_crea_j, k2_crea_j);
  COMPARE_RET_DIFF(k1_nlj_c1, k2_nlj_c1);
  COMPARE_RET_DIFF(k1_nlj_c2, k2_nlj_c2);
  COMPARE_RET_DIFF(k1_a_parity, k2_a_parity);
  COMPARE_RET_DIFF(k1_anni_j, k2_anni_j);
  COMPARE_RET_DIFF(k1_nlj_a1, k2_nlj_a1);
  COMPARE_RET_DIFF(k1_nlj_a2, k2_nlj_a2);

  return 0;
}

double findState2(nlj_hash_item *nlj_items, size_t num_nlj_items,int i1,int i2, int j1,int j2, int J1, int J2,int jtrans)
{

  nlj_hash_item *found_item;
  

      uint64_t key =
	(((uint64_t) j1) <<  0) |   //a1
	(((uint64_t) j2) << 11) |   //a2
	(((uint64_t) i1) << 22) |   //c1
	(((uint64_t) i2) << 33) |  //c2
	(((uint64_t) J2) << 44) | //anni_j
	(((uint64_t) J1) << 51) | //crea_j
	(((uint64_t) jtrans) << 58);

      found_item=(nlj_hash_item *) bsearch (&key, nlj_items, num_nlj_items, sizeof (nlj_hash_item), compare_nlj_item); 
      if(found_item!=NULL){

	return (double)found_item->_value;
      }
      else{

	return 0.0;

      }
}



double norm(int na,int la,int ja,int nb,int lb,int jb,int J,int T) //T=0,1
{
  if(na==nb && la==lb && ja==jb)
    {
      if ((J+T)%2==1)
	{
	  return sqrt(2.0);
	}
      else
	{
	  return 0.0;
	}
    }
  else
    {
      return 1.0;
    }
}

void*  readDumpfile(char *filename, size_t *num_nlj_items)
{ 
  nlj_hash_item *nlj_items=0;
 
  int fd = open (filename,O_RDONLY);
  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  full_read (fd, num_nlj_items, sizeof (*num_nlj_items));

  printf ("%zd nlj items.\n", *num_nlj_items);

  size_t sz_nlj_items = sizeof (nlj_hash_item) * *num_nlj_items;
  nlj_items = (nlj_hash_item *) malloc (sz_nlj_items);

  if (!nlj_items)
    {
      fprintf (stderr,
	       "Memory allocation error (%zd bytes).\n", sz_nlj_items);
      exit(1);
    }
 

  full_read (fd, nlj_items, sizeof (nlj_hash_item) * *num_nlj_items);

  close (fd);
 
  qsort (nlj_items, *num_nlj_items, sizeof (nlj_hash_item), compare_nlj_item); 

  return nlj_items;
}

int main()
{

  char filename_nn[14]="nlj_out-nn.bin";
  char filename_pp[14]="nlj_out-pp.bin";
  char filename_np[14]="nlj_out-np.bin";
  char filename_pn[14]="nlj_out-pn.bin";

  char filename_p[14]="nlj_out-p.bin";
  char filename_n[14]="nlj_out-n.bin";
 
  size_t num_nlj_2=CFG_NUM_NLJ_STATES*CFG_NUM_NLJ_STATES;
  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;
  
  int num_jtrans=(jtrans_max-jtrans_min)/2+1;
  //  printf("Jtrans Max=%d Min=%d num=%d \n",jtrans_max,jtrans_min,num_jtrans);
  double final_p[num_jtrans][num_nlj_2];
  double final_n[num_jtrans][num_nlj_2];
 
  FILE *fpr=NULL;
  FILE *fn=NULL;

  fpr=fopen(filename_p,"rb");
  fn=fopen(filename_n,"rb");
  int jtrans;
  int ii=0;
  for(ii=0;ii<num_jtrans;ii++){
    if(fpr!=NULL){
      fread(final_p[ii],sizeof(double),num_nlj_2,fpr);
      //  printf("ii=%d\n",ii);
      
    }
    else{
      perror("open");
      exit(1);
    }
    if(fn!=NULL){
      fread(final_n[ii],sizeof(double),num_nlj_2,fn);
    }
    else{
      perror("open");
      exit(1);
    }
  }

  fclose(fpr);
  fclose(fn);
  
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

  FILE *fp;
  fp=fopen("output.txt","w");
  if (fp == NULL) {
  fprintf(stderr, "Can't open output file in!\n");
  exit(1);
  }
  printf("Write trdens-like file: output.txt\n");
  fprintf(fp," OBDME calculation\n");
  fprintf(fp," T \n"); //if diagonal elements.
  fprintf(fp," Wave functions read from anto.egv file");
  fprintf(fp,"\n \n *** Nuclear states ***\n");
  fprintf(fp," Nucleus:\n");
  int A=CFG_NUM_SP_STATES0+CFG_NUM_SP_STATES1;
  
  //check coul! check order!
  int Z=CFG_NUM_SP_STATES0;
  int N=CFG_NUM_SP_STATES1;
  int two_MT=CFG_NUM_SP_STATES0-CFG_NUM_SP_STATES1;
  char parity;
  if(CFG_PARITY_INITIAL ==0){
     parity='+';
  }
  else{
     parity='-';
  }
  float hw=20.0;
  
  int Nhw=2;
  int dim=5;
  int nhme=0;
  int k1max=-1;
  int mxnwd=1;
  int mxsps=32;
  int major=3;
  int iparity=0; 
  double T=(double)CFG_2T_INITIAL/2.0f;
  double energy=-12.5466;
  float ex=0.0;
  int n1max=1;
  int n12max=2;
  int nasps=8;

  fprintf(fp," A=%3d   Z=%3d   N=%3d\n",A,Z,N);
  fprintf(fp," 2*MJ=%3d   2*MT=%3d  parity= %c \n",CFG_2M_INITIAL,two_MT,parity);
  fprintf(fp," hbar Omega=%8.4f   Nhw=%3d   dimension=%8d   nhme=%10d\n",hw,Nhw,dim,nhme);
  fprintf(fp," k1max=%3d   mxnwd=%3d   mxsps=%8d   major=%2d   iparity= %d\n \n",k1max,mxnwd,mxsps,major,iparity);
  
  fprintf(fp," J=%7.4f    T=%7.4f     Energy=%12.4f     Ex=%12.4f\n \n",CFG_2J_INITIAL/2.,T,energy,ex);
  fprintf(fp," N1_max=%4d   N12_max=%4d   Nasps=%4d\n \n",n1max,n12max,nasps);
  fprintf(fp," wave functions of the states #%3d- #%3d used\n \n",1,1);   //Fixed for only gs.
  fprintf(fp," wave functions of the states #%3d- #%3d used\n \n",1,1);   //Fixed for only gs.
  fprintf(fp," number of single-nucleon states =%4d\n",CFG_NUM_NLJ_STATES);

  printf("Write one-body matrix elements\n");

  for (int i=0;i<CFG_NUM_NLJ_STATES; i++)
  {
    fprintf(fp," #%4d  n=%3d  l=%3d  j=%2d/2\n",i+1,_table_nlj_states[i]._n,_table_nlj_states[i]._l,_table_nlj_states[i]._j);
  
  }
  ii=0;
  
  fprintf(fp,"\n\n *** Transition matrix elements for states: ***\n");
  fprintf(fp," #  1 [2*(J,T),Ex]_f= %2d%2d  0.0000   #  1 [2*(J,T),Ex]_i= %2d%2d  0.0000\n",CFG_2J_FINAL,CFG_2T_FINAL,CFG_2J_INITIAL,CFG_2T_INITIAL); //i, excitation energy
   int showJtrans=0;
  for (jtrans = jtrans_min; jtrans <= jtrans_max; jtrans += 2)
   {
     showJtrans=1;
     for( int sp_crea=0;sp_crea<CFG_NUM_NLJ_STATES;sp_crea++){
       for(int sp_anni=0;sp_anni<CFG_NUM_NLJ_STATES;sp_anni++){
	 //       	 if(fabs(final_p[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES])>0.000001||fabs(final_n[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES])>0.000001)
	 int jc=_table_nlj_states[sp_crea]._j;
	 int ja=_table_nlj_states[sp_anni]._j;
	 int lc=_table_nlj_states[sp_crea]._l;
	 int la=_table_nlj_states[sp_anni]._l;

	 if((jc+ja)<jtrans){continue;}
	 if(abs(jc-ja)>jtrans){continue;}
	 if(pow(-1,lc)==pow(-1,la)){   //This needs to be fixed if used for two different states. 
	   
	   if(showJtrans==1){
	     fprintf(fp,"\n Jtrans=%3d\n",jtrans/2);
	     showJtrans=0;
	   }
	   fprintf(fp," a+=%3d    a-=%3d     td(a+,a-): p=%10.6f     n=%10.6f\n",sp_crea+1,sp_anni+1,final_p[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES],final_n[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES]);
	 }
       }
     }
     ii++;
   }
  fprintf(fp," ***************************************************************\n\n");

  //  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  //  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;


  //  int jtrans;

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
  
  fprintf(fp,"  Two-body transition matrix elements\n");
  fprintf(fp,"  ***********************************\n");

#ifdef CFG_SP_PAIRS

  fprintf(fp," number of two-nucleon states =%4d\n",CFG_SP_PAIRS); 
#else
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
  fprintf(fp," number of two-nucleon states =%5d\n",twob1++);
  twob1=0;
#endif
  fprintf(fp," number of two-body Hamiltonian matrix elements = %6d\n",32);  //compute!!
   
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
		fprintf(fp," #%5d  a b J T= %2d %2d  %1d %1d \n",twob1,i1+1,i2+1,Jab,Tab);
	      }
	  }
      }
    }
  }

  fprintf(fp,"\n\n *** Transition matrix elements for states: ***\n");
 //loop over all states. 

  fprintf(fp," #  1 [2*(J,T),Ex]_f= %2d%2d  0.0000   #  1 [2*(J,T),Ex]_i= %2d%2d  0.0000\n",CFG_2J_FINAL,CFG_2T_FINAL,CFG_2J_INITIAL,CFG_2T_INITIAL); //i, excitation energy
 
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
			  double rev1_pp=0.0;
			  double rev2_pp=0.0;
			  double rev3_pp=0.0;
			  printf("%d %d %d %d: %f %f %f \n",i1,i2,j1,j2,value_np,value_nn,value_pp);
			  if(i1!=i2){
			    rev1_np=findState2(_nlj_items_np,_num_nlj_items_np,i2,i1,j1,j2,2*Jab,2*Jcd,jtrans);  
			    printf("i1!=i2: %f ",rev1_np);
			    if(rev1_np&& i2>i1){rev1_np=rev1_np*pow(-1.,-(ji1+ji2)/2+Jab+Tab);
			      printf("Phase: %f \n",pow(-1.,-(ji1+ji2)/2+Jab+Tab));
			    }
			    //      	    rev1_pp=findState2(_nlj_items_pp,_num_nlj_items_pp,i2,i1,j1,j2,2*Jab,2*Jcd,jtrans);  
			    // if(rev1_pp){rev1_pp=rev1_pp*pow(-1.,-(ji1+ji2)/2+Jab+Tab);}
			  }
			  
			  if(j2!=j1){  
			    rev2_np=findState2(_nlj_items_np,_num_nlj_items_np,i1,i2,j2,j1,2*Jab,2*Jcd,jtrans);
			    if(rev2_np&&j2>j1){rev2_np=rev2_np*pow(-1.,-(jj1+jj2)/2+Jcd+Tcd);}
			    // rev2_pp=findState2(_nlj_items_pp,_num_nlj_items_pp,i1,i2,j2,j1,2*Jab,2*Jcd,jtrans);
			    // if(rev2_pp){rev2_pp=rev2_pp*pow(-1.,-(jj1+jj2)/2+Jcd+Tcd);}
			  }
					  
			  if(i1!=i2&&j2!=j1){
			    rev3_np=findState2(_nlj_items_np,_num_nlj_items_np,i2,i1,j2,j1,2*Jab,2*Jcd,jtrans);
			    if(rev3_np&&i2>i1&&j1<j2){rev3_np=rev3_np*pow(-1.,-(ji1+ji2+jj1+jj2)/2+Jab+Jcd+Tab+Tcd); }
			    //rev3_pp=findState2(_nlj_items_pp,_num_nlj_items_pp,i2,i1,j2,j1,2*Jab,2*Jcd,jtrans);
			    // if(rev3_pp){rev3_pp=rev3_pp*pow(-1.,-(ji1+ji2+jj1+jj2)/2+Jab+Jcd+Tab+Tcd); }
			    
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
			  value_pp=(value_pp+rev1_pp+rev2_pp+rev3_pp)*mult*clebsch_pp*Nab*Ncd;
			  value_np=(value_np+rev1_np+rev2_np+rev3_np)*mult*clebsch_np*Nab*Ncd;
				  
			  if(fabs(value_np)<0.000001){value_np=0.0;}
			  if(fabs(value_nn)<0.000001){value_nn=0.0;}
			  if(fabs(value_pp)<0.000001){value_pp=0.0;}

			  if ((fabs(value_np)>0.000001) ||( fabs(value_pp)>0.000001) ||( fabs(value_nn)>0.000001) ){   //roundoff-error?
			    if(showJtrans==1){
			      fprintf (fp,"\n Jtrans= %2d\n", jtrans/2);
			      showJtrans=0;
			    }
#if NP_ORDER
			    
			    fprintf(fp," (a+a+)J=%5d  (a-a-)J=%5d   td: np=%10.6f   pp=%10.6f   nn=%10.6f\n",twob1,twob2, value_np, value_pp,value_nn);//, Jab, Tab, Jcd,Tcd);
#else 
			    fprintf(fp," (a+a+)J=%5d  (a-a-)J=%5d   td: pn=%10.6f   pp=%10.6f   nn=%10.6f\n",twob1,twob2, value_np, value_pp,value_nn);//, Jab, Tab, Jcd,Tcd);
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
  fclose(fp);
  return 0;
}
