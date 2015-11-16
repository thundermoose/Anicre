#include <stdio.h>
#include <stdlib.h>

#include "calobop.h"
#include "caltbop.h"
//#define M_PI 3.14159265358979323846

/*nlj_hash_item *_nlj_items_nn = NULL;
size_t     _num_nlj_items_nn = 0;
nlj_hash_item *_nlj_items_pp = NULL;
size_t     _num_nlj_items_pp = 0;
nlj_hash_item *_nlj_items_np = NULL;
size_t     _num_nlj_items_np = 0;
nlj_hash_item *_nlj_items_pn = NULL;
size_t     _num_nlj_items_pn = 0;
*/

#define MASSN 938.9187
#define HBARC 197.326963

int main()
{

  //Check if p- n-file exist
  //Write out what to compute
  
  printf("COMPUTE ONE-BODY OPERATORS\n");
  //Compute observables
  oneb();
  // Check if pp, nn and np/np files exist
  // Check if TBME.int exist:
  //   -Write out file information
  //   -Controll if correct number of TBME read
  // Write out what to compute
  printf("COMPUTE TWO-BODY OPERATORS\n");
  twob();
  return 0;
}

