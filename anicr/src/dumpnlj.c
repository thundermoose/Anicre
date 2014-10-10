
#include "util.h"

#include "anicr_tables.h"
#include "anicr_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <gsl/gsl_errno.h>
#include "gsl/gsl_sf_coupling.h"

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


double findState(nlj_hash_item *nlj_items, size_t num_nlj_items,int i1,int i2, int j1,int j2, int J1, int J2,int jtrans)
{
  uint64_t savedkey=0;
    for (size_t i = 0; i < num_nlj_items; i++)
  //    	       for (size_t i = 0; i <1; i++)
    {
      //   printf("i %d %d\n",(int)i,(int)num_nlj_items);
   
      uint64_t key = nlj_items[i]._key;
      
      int nlj_a1, nlj_a2, nlj_c1, nlj_c2;
      int anni_j, crea_j;
      int key_jtrans;
      
      nlj_a1 = (key >>  0) & 0x7ff;
      nlj_a2 = (key >> 11) & 0x7ff;
      nlj_c1 = (key >> 22) & 0x7ff;
      nlj_c2 = (key >> 33) & 0x7ff;
      anni_j = (key >> 44) &  0x7f;
      crea_j = (key >> 51) &  0x7f;
      key_jtrans = (int) (key >> 58);
      //For nn
      //   printf("TESTING %d %d %d %d %d \n", (int)i,nlj_a1,nlj_a2,nlj_c1,nlj_c2);
      if (key_jtrans == jtrans && nlj_c1==i1 && nlj_c2==i2 && nlj_a1==j1 && nlj_a2==j2 && anni_j==J1 && crea_j==J2)
	{
	  printf("Found state %d %d and %d %d ",i1,i2,j1,j2);
	  if(savedkey!=0){
	    printf("ERROR: More then one key found!");
	    return 0;
	   }
	  savedkey=i;
	}
      //printf("test2\n");
    }
    //  printf("Saved key %d /n",(int)savedkey);
  if(savedkey==0)
    {
    return 0;
    }
  else
    {
    printf("Klart!");
    return (double)nlj_items[savedkey]._value;
    }
  }

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


void*  readDumpfile(char *filename, size_t *num_nlj_items)
{ 
  nlj_hash_item *nlj_items=0;
  printf("Address1: %p\n",(void*)nlj_items);
  int fd = open (filename,O_RDONLY);
  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  full_read (fd, num_nlj_items, sizeof (*num_nlj_items));

  printf ("%zd nlj items.\n", *num_nlj_items);

  size_t sz_nlj_items = sizeof (nlj_hash_item) * *num_nlj_items;
  printf("test %d",(int)sz_nlj_items);
  nlj_items = (nlj_hash_item *) malloc (sz_nlj_items);
  printf("Address1: %p\n",(void*)nlj_items);

  if (!nlj_items)
    {
      fprintf (stderr,
	       "Memory allocation error (%zd bytes).\n", sz_nlj_items);
      exit(1);
    }
 

  full_read (fd, nlj_items, sizeof (nlj_hash_item) * *num_nlj_items);

  close (fd);
 
 qsort (nlj_items, *num_nlj_items, sizeof (nlj_hash_item), compare_nlj_item);
for (int i=0;i<(int)*num_nlj_items;i++){

   printf("test %f",nlj_items[i]._value);
  }
  printf("Address1: %p\n",(void*)nlj_items);

  return nlj_items;
}

int main()
{

  char filename_nn[14]="nlj_out-nn.bin";
  char filename_pp[14]="nlj_out-pp.bin";
  char filename_np[14]="nlj_out-np.bin";

  _nlj_items_nn=readDumpfile(filename_nn,&_num_nlj_items_nn);
  _nlj_items_pp=readDumpfile(filename_pp,&_num_nlj_items_pp);
  _nlj_items_np=readDumpfile(filename_np,&_num_nlj_items_np);

  printf("test %d",(int) _num_nlj_items_nn);
  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;

  int jtrans;

  int mtrans = CFG_2M_INITIAL - CFG_2M_FINAL;
  if (abs(mtrans) > jtrans_max)
    {
      fprintf (stderr, "FIXME: abs(mtrans) > jtrans_max.\n");
      exit(1);
    }
  for (jtrans = jtrans_min; jtrans <= jtrans_max; jtrans += 2)
    {
      printf ("Jtrans=%d\n", jtrans/2);

      double mult;

      {
	gsl_sf_result result;
	
	int ret =
	  gsl_sf_coupling_3j_e(CFG_2J_INITIAL,  jtrans,  CFG_2J_FINAL,
			       CFG_2M_INITIAL, -mtrans, -CFG_2M_FINAL,
			       &result);
	if (ret != GSL_SUCCESS)
	  {
	    fprintf (stderr,"ERR! %d\n", ret);
	    exit(1);
	  }
	int sign = 1 - ((CFG_2J_INITIAL/* - jtrans*/ + CFG_2M_FINAL) & 2);
       
	mult = 1 / (result.val) * sign;
      }
      
      int Tab;
      int Jab;
      int pi1;
    
      int twob1=0;
      int twob2=0;
      printf("num %d",CFG_NUM_NLJ_STATES);
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
		    //		    printf("a,b: %d: %d %d %d    %d: %d %d %d\n",i1+1,ni1,li1,ji1,i2+1,ni2,li2,ji2);
		    //		    printf("1:Two-body state %d: %d %d J=%d T=%d pi=%d \n",twob1,i1+1,i2+1,J1,T1,pi1);
		    twob1++;
		    twob2=0;
		    for(int pi2=1;pi2>=-1;pi2=pi2-2){
		      for(int Jcd=0; Jcd<=CFG_MAX_J;Jcd++){
			for (int Tcd=0;Tcd<=1;Tcd++){
			  for (int j1 = 0; j1<CFG_NUM_NLJ_STATES; j1++)
			    {
			for(int j2 = j1; j2<CFG_NUM_NLJ_STATES; j2++)
			  { 
			    //	    printf("%d %d \n",j1,j2);
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
		    double value_nn=findState(_nlj_items_nn, _num_nlj_items_nn, i1, i2,  j1, j2, Jab,Jcd,jtrans);
		    double value_pp=findState(_nlj_items_pp, _num_nlj_items_pp, i1, i2,  j1, j2, Jab,Jcd,jtrans);
		    double value_np=findState(_nlj_items_np, _num_nlj_items_np, i1, i2,  j1, j2, Jab,Jcd,jtrans);


		  //For nn
		  
		      if (value_np || value_pp || value_nn){
	
			printf("test %d %d %d \n",twob1,twob2,Tab);
		       

			printf ("Create %3d %3d : %2d | Annihilate %3d %3d : %2d = %11.6f %11.6f %11.6f\n",
			    i1+1, i2+1, Jab,
			    j1+1, j2+1, Jcd,
				mult * value_nn, mult*value_pp,mult*value_np);
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

  return 0;
}
