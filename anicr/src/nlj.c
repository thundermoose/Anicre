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
#include <stdint.h>
#include "nlj.h"

#include <gsl/gsl_errno.h>
#include "gsl/gsl_sf_coupling.h"

nlj_hash_item *_nlj_items_nn;
size_t     _num_nlj_items_nn;
nlj_hash_item *_nlj_items_pp;
size_t     _num_nlj_items_pp;
nlj_hash_item *_nlj_items_np;
size_t     _num_nlj_items_np;
nlj_hash_item *_nlj_items_pn;
size_t     _num_nlj_items_pn;

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

  //  printf ("%zd nlj items.\n", *num_nlj_items);

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
retval computeres(int i1, int i2,int j1,int j2,int Jab,int Jcd,int jtrans,int Tab,int Tcd,double mult)
{
  double value_nn=findState2(_nlj_items_nn, _num_nlj_items_nn, i1, i2,  j1, j2, 2*Jab,2*Jcd,jtrans);
  double value_pp=findState2(_nlj_items_pp, _num_nlj_items_pp, i1, i2,  j1, j2, 2*Jab,2*Jcd,jtrans);
  double value_np=findState2(_nlj_items_np, _num_nlj_items_np, i1, i2,  j1, j2, 2*Jab,2*Jcd,jtrans);  

  
  // printf("%d %d %d %d: %f %f %f \n",i1,i2,j1,j2,value_np,value_nn,value_pp);
  double rev1_np=0.0;
  double rev2_np=0.0;
  double rev3_np=0.0;
  int li1=_table_nlj_states[i1]._l;
  int li2=_table_nlj_states[i2]._l;
  int ni1=_table_nlj_states[i1]._n;
  int ni2=_table_nlj_states[i2]._n;
  int ji1=_table_nlj_states[i1]._j;
  int ji2=_table_nlj_states[i2]._j;

  int lj1=_table_nlj_states[j1]._l;
  int lj2=_table_nlj_states[j2]._l;
  int nj1=_table_nlj_states[j1]._n;
  int nj2=_table_nlj_states[j2]._n;
  int jj1=_table_nlj_states[j1]._j;
  int jj2=_table_nlj_states[j2]._j;

  if(i1!=i2){
    rev1_np=findState2(_nlj_items_np,_num_nlj_items_np,i2,i1,j1,j2,2*Jab,2*Jcd,jtrans);
    
    //			    printf("i1!=i2: %f ",rev1_np);
    if(rev1_np&& i2>i1){rev1_np=rev1_np*pow(-1.,-(ji1+ji2)/2+Jab+Tab);
      //     printf("Phase: %f \n",pow(-1.,-(ji1+ji2)/2+Jab+Tab));
    }
    
  }
  
  if(j2!=j1){  
    rev2_np=findState2(_nlj_items_np,_num_nlj_items_np,i1,i2,j2,j1,2*Jab,2*Jcd,jtrans);
    if(rev2_np&&j2>j1){rev2_np=rev2_np*pow(-1.,-(jj1+jj2)/2+Jcd+Tcd);}
    
  }
  
  if(i1!=i2&&j2!=j1){
    rev3_np=findState2(_nlj_items_np,_num_nlj_items_np,i2,i1,j2,j1,2*Jab,2*Jcd,jtrans);
    if(rev3_np&&i2>i1&&j1<j2){rev3_np=rev3_np*pow(-1.,-(ji1+ji2+jj1+jj2)/2+Jab+Jcd+Tab+Tcd); }
    
    
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

  retval ret;
  ret._nn=value_nn*mult*clebsch_nn*Nab*Ncd;
  ret._pp=(value_pp)*mult*clebsch_pp*Nab*Ncd;
  ret._np=(value_np+rev1_np+rev2_np+rev3_np)*mult*clebsch_np*Nab*Ncd;
  return ret;
}
