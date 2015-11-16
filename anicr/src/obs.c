#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
  int oneb_files_exist=0;
  //Check if p- n-file exist
  if(access("nlj_out-n.bin",F_OK) !=-1){
    printf("nlj_out-n.bin exists \n");
    oneb_files_exist=1;
  }
  else{
    printf("Run ./n_anicr to compute transition data\n");
  }
    if(access("nlj_out-p.bin",F_OK) !=-1){
    printf("nlj_out-p.bin exists \n");

   }
  else{
    printf("Run ./p_anicr to compute transition data\n");
    oneb_files_exist=0;
  }
  //Write out what to compute
  if(oneb_files_exist){
      printf("COMPUTE ONE-BODY OPERATORS\n");
  //Compute observables
      oneb();
  }


  //#########################################
  //#######   TWO-BODY OPERATORS ############
  //#########################################
  // Check if pp, nn and np/np files exist
  // Check if TBME.int exist:
  //   -Write out file information
  //   -Controll if correct number of TBME read
  // Write out what to compute
  int twob_file_exist=0;
  printf("COMPUTE TWO-BODY OPERATORS\n");
  if(access("nlj_out-nn.bin",F_OK) !=-1){
    printf("nlj_out-nn.bin exists \n");
    twob_file_exist=1;
  }
  else{
    printf("Run ./nn_anicr to compute transition data\n");
  }

  if(access("nlj_out-pp.bin",F_OK) !=-1){
    printf("nlj_out-pp.bin exists \n");
  }
  else{
    printf("Run ./pp_anicr to compute transition data\n");
    twob_file_exist=0;
  }
  if(access("nlj_out-pn.bin",F_OK)!=-1 || access("nlj_out-np.bin",F_OK)!=-1){
    printf("nlj_out-pn/np.bin exist\n");
    
  }
  else{
    printf("Run ./np_anicr (pn_anicr) to compute transition data\n");
    twob_file_exist=0;
  }
  if(twob_file_exist){
    if(access("TBME.int",F_OK)!=-1){
      readTBME();
    }
    twob();
  }
  return 0;
}

