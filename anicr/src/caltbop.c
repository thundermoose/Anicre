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
#include <stdint.h>

#include "caltbop.h"

#define MASSN 938.9187
#define HBARC 197.326963
typedef struct twob_state{
  uint64_t _key; 
  double _hrel;
  double _trel;
  double _vpp;
  double _vnn;
  double _vpn;
  double _coul;
} twob_state;

struct twob_state *twob_array;
size_t numTBME=0;

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

 readTBME();
  
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
 
  printf("\n\n *** Transition matrix elements for states: ***\n");
 //loop over all states. 

 
//for (jtrans = jtrans_min; jtrans <= jtrans_max; jtrans += 2)
for (jtrans = jtrans_min; jtrans <= 0; jtrans += 2)

 	
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
   	//  for(int itbme=0; itbme<numTBME;itbme++)
      for(pi1=1;pi1>=-1;pi1=pi1-2){
      	//Jab? 
	for(Jab=0; Jab<=CFG_MAX_J;Jab++){
	  for (Tab=0;Tab<=1;Tab++){
	    for (int i1 = 0; i1<CFG_NUM_NLJ_STATES; i1++){
	      for(int i2 = i1; i2<CFG_NUM_NLJ_STATES; i2++){
	//	for(int itbme=0; itbme<numTBME;itbme++){
	//	   	i1=twob_array[itbme].a; //+1??
//		   	i2=twob_array[itbme].b; //+1??
//		   	  	j1=twob_array[itbme].c; //+1??
//		   	  	j2=twob_array[itbme].d; //+1??
//		   	jtrans=twob_array[itbme].a;
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
        
	  if(Jcd==Jab && Tcd==Tab ){
				  //  printf("key: %d %d %d \n", Jcd,(int)getKey(i1,i2,j1,j2,Jcd,0), (int)getKey(i1,i2,j1,j2,Jcd,1));

	    twob_state *p1=NULL;
	    twob_state *p2=NULL;
	    uint64_t key1=getKey(i1,i2,j1,j2,Jcd,1);
	    uint64_t key2=getKey(j1,j2,i1,i2,Jcd,1);
	    
	    p1=bsearch (&key1,twob_array, numTBME, sizeof (twob_state), compare_tbme_item); 
	    p2=bsearch (&key2,twob_array, numTBME, sizeof (twob_state), compare_tbme_item);
	    
	    if(p1!=NULL){// p2!=NULL){
	      printf("%d %d Hit T34=1 P1:%f %d %d %d %d \n",twob1,twob2,p1->_trel,i1,i2,j1,j2);
	    }
	    if(p2!=NULL){				 printf("%d %d Hit T34=1 P2: %f %d %d %d %d \n",twob1,twob2,p2->_trel,i1,i2,j1,j2);}
			      
	    p1=NULL;
	    p2=NULL;
	    key1=getKey(i1,i2,j1,j2,Jcd,0);
	    key2=getKey(j1,j2,i1,i2,Jcd,0);
	    p1=bsearch (&key1,twob_array, numTBME, sizeof (twob_state), compare_tbme_item); 
	    p2=bsearch (&key2,twob_array, numTBME, sizeof (twob_state), compare_tbme_item); 
	    if(p1!=NULL){// p2!=NULL){
	      printf("%d %d Hit T34=0 P1:%f %d %d %d %d \n",twob1,twob2,p1->_trel,i1,i2,j1,j2);
	    }
	    if(p2!=NULL){
	      printf("%d %d Hit T34=0 P2: %f %d %d %d %d \n",twob1,twob2,p2->_trel,i1,i2,j1,j2);}
	  
	  }
	}		 
			}}}//}
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

//  else{printf("test %d \n",twob_array[0].a);}
  return 0;
}
int compare_tbme_item(const void *p1, const void *p2)
{
  uint64_t key1 = ((const twob_state *) p1)->_key;
  uint64_t key2 = ((const twob_state *) p2)->_key;

  int k1_nlj_a1, k1_nlj_a2, k1_nlj_c1, k1_nlj_c2;
  int k1_j23, k1_t23;

  k1_nlj_a1 = (key1 >>  0) & 0x7ff;
  k1_nlj_a2 = (key1 >> 11) & 0x7ff;
  k1_nlj_c1 = (key1 >> 22) & 0x7ff;
  k1_nlj_c2 = (key1 >> 33) & 0x7ff;
  k1_j23 = (key1 >> 44) &  0x7f;
  k1_t23 = (key1 >> 51) &  0x7f;

  int k1_a1_l = _table_nlj_states[k1_nlj_a1]._l;
  int k1_a2_l = _table_nlj_states[k1_nlj_a2]._l;
  int k1_c1_l = _table_nlj_states[k1_nlj_c1]._l;
  int k1_c2_l = _table_nlj_states[k1_nlj_c2]._l;

  int k1_a_parity = (k1_a1_l ^ k1_a2_l) & 1;
  int k1_c_parity = (k1_c1_l ^ k1_c2_l) & 1;

  int k2_nlj_a1, k2_nlj_a2, k2_nlj_c1, k2_nlj_c2;
  int k2_j23, k2_t23;

  k2_nlj_a1 = (key2 >>  0) & 0x7ff;
  k2_nlj_a2 = (key2 >> 11) & 0x7ff;
  k2_nlj_c1 = (key2 >> 22) & 0x7ff;
  k2_nlj_c2 = (key2 >> 33) & 0x7ff;
  k2_j23 = (key2 >> 44) &  0x7f;
  k2_t23 = (key2 >> 51) &  0x7f;

  int k2_a1_l = _table_nlj_states[k2_nlj_a1]._l;
  int k2_a2_l = _table_nlj_states[k2_nlj_a2]._l;
  int k2_c1_l = _table_nlj_states[k2_nlj_c1]._l;
  int k2_c2_l = _table_nlj_states[k2_nlj_c2]._l;

  int k2_a_parity = (k2_a1_l ^ k2_a2_l) & 1;
  int k2_c_parity = (k2_c1_l ^ k2_c2_l) & 1;

#define COMPARE_RET_DIFF(x,y) {if ((x) != (y)) return ((x) < (y)) ? -1 : 1; }

  COMPARE_RET_DIFF(k1_c_parity, k2_c_parity);
  COMPARE_RET_DIFF(k1_j23, k2_j23);
  COMPARE_RET_DIFF(k1_nlj_c1, k2_nlj_c1);
  COMPARE_RET_DIFF(k1_nlj_c2, k2_nlj_c2);
  COMPARE_RET_DIFF(k1_a_parity, k2_a_parity);
  COMPARE_RET_DIFF(k1_t23, k2_t23);
  COMPARE_RET_DIFF(k1_nlj_a1, k2_nlj_a1);
  COMPARE_RET_DIFF(k1_nlj_a2, k2_nlj_a2);

  return 0;
}
int printKey(twob_state p1 ){
	uint64_t key=p1._key;
	int a1,a2,c1,c2,j23,t23;
	a1 = (key >>  0) & 0x7ff;
  	a2 = (key >> 11) & 0x7ff;
  	c1 = (key >> 22) & 0x7ff;
  	c2 = (key >> 33) & 0x7ff;
  	j23 = (key >> 44) &  0x7f;
  	t23 = (key >> 51) &  0x7f;
  	printf("a1: %d, a2: %d, c1: %d, c2: %d 2*J23: %d 2*T23: %d ",a1,a2,c1,c2,j23,t23);
	return 0;
}

uint64_t getKey(int a1,int a2,int c1,int c2,int J23,int T23)
{
	uint64_t  key= 
	(((uint64_t) a1) <<  0) |   //a1
	(((uint64_t) a2) << 11) |   //a2
	(((uint64_t) c1) << 22) |   //c1
	(((uint64_t) c2) << 33) |  //c2
	(((uint64_t) J23) << 44) | //anni_j
	(((uint64_t) T23) << 51);

	return key;

}
int readTBME(){

  printf("Read TBME.int\n");
  FILE *fd;
  int a1,a2,c1,c2,J23,T23;
  float f[6];
  fd=fopen("TBME.int","r");
  if(fd==NULL){
      perror("Error while opening the file.\n");
      exit(EXIT_FAILURE);
  }
  

  fscanf(fd,"%zd",&numTBME);
  twob_array=malloc(sizeof(twob_state)*numTBME);
  
  for(int i=0;i<(int)numTBME; i++){
    fscanf(fd,"%d %d %d %d %d %d %f %f %f %f %f %f",&a1,&a2,&c1,&c2,&J23,&T23,&f[0],&f[1],&f[2],&f[3],&f[4],&f[5]);
    uint64_t key= 
	(((uint64_t) a1-1) <<  0) |   //a1
	(((uint64_t) a2-1) << 11) |   //a2
	(((uint64_t) c1-1) << 22) |   //c1
	(((uint64_t) c2-1) << 33) |  //c2
	(((uint64_t) J23) << 44) | //anni_j
	(((uint64_t) T23) << 51);/* | //crea_j
	(((uint64_t) jtrans) << 58); */

    twob_array[i]._key=key;
    twob_array[i]._trel=f[0];
    twob_array[i]._hrel=f[1];
    twob_array[i]._coul=f[2];
    twob_array[i]._vpn=f[3];
    twob_array[i]._vpp=f[4];
    twob_array[i]._vnn=f[5];
    
  }
  int err=fclose(fd);
  if(err!=0){
    perror("Error while closing file");
  }
  else{printf("Success!\n");}
  

  printf("%zu\n",numTBME);

  for (int i=0;i<(int)numTBME;i++){
  	printKey(twob_array[i]);

	printf("%llu %f %f %f %f \n",(unsigned long long int)twob_array[i]._key,twob_array[i]._hrel,twob_array[i]._trel,twob_array[i]._coul,twob_array[i]._vpn);
  }
    qsort (twob_array, numTBME, sizeof (twob_state), compare_tbme_item); 
  printf("\n After sorting: \n");
  for (int i=0;i<(int)numTBME;i++){
  	printKey(twob_array[i]);
	printf(" %llu %f %f %f %f \n",(unsigned long long int)twob_array[i]._key,twob_array[i]._hrel,twob_array[i]._trel,twob_array[i]._coul,twob_array[i]._vpn);
  }
  return 0;
}
