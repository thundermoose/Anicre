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
