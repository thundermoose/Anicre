
#include "util.h"

#include "anicr_tables.h"
#include "anicr_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include <gsl/gsl_errno.h>
#include "gsl/gsl_sf_coupling.h"

typedef struct nlj_hash_item_t
{
  uint64_t _key;
  double   _value;
} nlj_hash_item;

nlj_hash_item *_nlj_items = NULL;
size_t     _num_nlj_items = 0;

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

#define COMPARE_RET_DIFF(x,y) {if (x != y) return (x < y) ? -1 : 1; }

  COMPARE_RET_DIFF(k1_jtrans, k2_jtrans);
  COMPARE_RET_DIFF(k1_nlj_c1, k2_nlj_c1);
  COMPARE_RET_DIFF(k1_nlj_c2, k2_nlj_c2);
  COMPARE_RET_DIFF(k1_crea_j, k2_crea_j);
  COMPARE_RET_DIFF(k1_nlj_a1, k2_nlj_a1);
  COMPARE_RET_DIFF(k1_nlj_a2, k2_nlj_a2);
  COMPARE_RET_DIFF(k1_anni_j, k2_anni_j);

  return 0;
}

int main()
{


  int fd = open ("nlj_out.bin",O_RDONLY);
  
  if (fd == -1)
    {
      perror("open");
      exit(1);
    }

  full_read (fd, &_num_nlj_items, sizeof (_num_nlj_items));

  printf ("%zd nlj items.\n", _num_nlj_items);

  size_t sz_nlj_items = sizeof (nlj_hash_item) * _num_nlj_items;

  _nlj_items = (nlj_hash_item *) malloc (sz_nlj_items);

  if (!_nlj_items)
    {
      fprintf (stderr,
	       "Memory allocation error (%zd bytes).\n", sz_nlj_items);
      exit(1);
    }
 

  full_read (fd, _nlj_items, sizeof (nlj_hash_item) * _num_nlj_items);

  close (fd);

  qsort (_nlj_items, _num_nlj_items, sizeof (nlj_hash_item), compare_nlj_item);

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

      size_t i;
      
      for (i = 0; i < _num_nlj_items; i++)
	{
	  uint64_t key = _nlj_items[i]._key;
	  
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

	  if (key_jtrans == jtrans)
	    {	  
	      double value = _nlj_items[i]._value;

	      if (value)
		printf ("Create %3d %3d : %2d | Annihilate %3d %3d : %2d = %11.6f\n",
			nlj_c1+1, nlj_c2+1, crea_j,
			nlj_a1+1, nlj_a2+1, anni_j,
			mult * value);
	    }
	  
	}
    }

  free (_nlj_items);

  return 0;
}
