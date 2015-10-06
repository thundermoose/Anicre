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
double radialHO(double r,double b,int n,int l);
double obmeSH(int l1,int jj1,int l2,int jj2,int lambda);
double obmeQ(int na, int la, int jja,int nb, int lb,int jb,int lambda,double b);
double computeB(double hw);
int main()
{

  /*  _nlj_items_nn = NULL;
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
  */
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
  /*  
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
  */
  FILE *fp;
  fp=fopen("obs.txt","w");
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
  double Qp=0.0,Qn=0.0;
  double b=1.0;//computeB(20.0);
  for (jtrans = jtrans_min; jtrans <= jtrans_max; jtrans += 2)
   {

     showJtrans=1;
     for( int sp_crea=0;sp_crea<CFG_NUM_NLJ_STATES;sp_crea++){
       for(int sp_anni=0;sp_anni<CFG_NUM_NLJ_STATES;sp_anni++){
	 //       	 if(fabs(final_p[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES])>0.000001||fabs(final_n[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES])>0.000001)
	       int nb=_table_nlj_states[sp_anni]._n;
	       int na=_table_nlj_states[sp_crea]._n;

	       int ja=_table_nlj_states[sp_crea]._j;
	       int jb=_table_nlj_states[sp_anni]._j;
	       int la=_table_nlj_states[sp_crea]._l;
	       int lb=_table_nlj_states[sp_anni]._l;
	       if((ja+jb)<jtrans){continue;}
	       if(abs(ja-jb)>jtrans){continue;}
	       if(pow(-1,lb)==pow(-1,la)){   //This needs to be fixed if used for two different states. 
	   
	         if(showJtrans==1){
	           fprintf(fp,"\n Jtrans=%3d\n",jtrans/2);
		   printf("\n Jtrans=%3d\n",jtrans/2);

	           showJtrans=0;
	         }
	         printf(" a+=%3d    a-=%3d     td(a+,a-): p=%10.6f     n=%10.6f\n",sp_crea+1,sp_anni+1,final_p[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES],final_n[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES]);
	         printf("Q= %f \n",obmeQ(na,la,ja,nb,lb,jb,jtrans/2,b));
	         Qp+=obmeQ(na,la,ja,nb,lb,jb,jtrans/2,b)*final_p[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES];
		 Qn+=obmeQ(na,la,ja,nb,lb,jb,jtrans/2,b)*final_n[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES];

          }

       }  
     }
     ii++;
     printf(" p: %f n: %f \n", Qp,Qn);
     Qp=0.0;
     Qn=0.0;
   }
  fprintf(fp," ***************************************************************\n\n");

  fclose(fp);

/*
  for(double r=0.0;r<10;r+=0.1){
    printf(" %f  %f \n",r,radialHO(r,1,2,1));
  }

  printf(" %f \n",obmeSH(1,1,1,1,0));
  return 0;
  */
}




double radialHO(double r, double b,int n,int l){
  double L;
  double temp;
  L=gsl_sf_laguerre_n(n,l+0.5,pow(r/b,2));
  temp=sqrt(2*tgamma(n+1)/(pow(b,3)*tgamma(n+l+3./2)))*pow(r/b,l);
  temp*=exp(-0.5*pow(r/b,2));
  temp*=L;
  
  return temp ;
};


double obmeSH(int l1,int jj1,int l2,int jj2,int lambda){
  //Computes the matrix element (l1,1/2,j2||Y_lambda||l2,1/2,j2) according to Suhonen eq. 2.57
  double temp;
  temp=1./sqrt(4.*M_PI)*pow(-1,jj2/2.0-0.5+lambda)*(1+pow(-1.,l1+l2+lambda))/2.0;
  temp*=sqrt(jj1+1.)*sqrt(jj2+1.)*sqrt(2*lambda+1.);
  gsl_sf_result result;
  int ret =
    gsl_sf_coupling_3j_e(jj1,jj2,lambda*2,1,-1,0,
             &result);
    
        if (ret != GSL_SUCCESS)
    {
      fprintf (stderr,"ERR! %d\n", ret);
      exit(1);
    }

  temp*=result.val;
  return temp;
}

struct RR_params{int na; int la;int nb;int lb;double b; int lambda;};
  
double radialQ(double x,void *p){//int na, int la, int nb, int lb,int lambda,double b){
  struct RR_params * params = (struct RR_params *)p;
  int na=(params->na);
  int la=(params->la);
  int nb=(params->nb);
  int lb=(params->lb);
  double b=(params->b);
  int lambda=(params->lambda);
  return radialHO(x,b,na,la)*pow(x,lambda)*radialHO(x,b,nb,lb)*pow(x,2);
}
double obmeQ(int na, int la, int jja,int nb, int lb,int jjb,int lambda,double b){
  //computes the reduced electric transition for single-particles.
  //Doesn't include the charge factor
  double Q;

  Q=obmeSH(la,jja,lb,jjb,lambda);

  
  //Integrate r-dependence
  struct RR_params alpha={na,la,nb,lb,b,lambda};
  gsl_integration_workspace * w = gsl_integration_workspace_alloc(1000);
  double result,error;
  gsl_function F;
  F.function = &radialQ;
  F.params=&alpha;
  int ret =gsl_integration_qag( &F,0,10,1e-5,1e-5,1000,6,w,&result,&error);
  if( error>=0.01){
        printf ("ERR! %d\n", ret);
      exit(1);
  }
  //printf ("result          = % .18f\n", result);
  //  printf ("exact result    = % .18f\n", expected);
  //printf ("estimated error = % .18f\n", error);
  //  printf ("actual error    = % .18f\n", result - expected);
  //printf ("intervals =  %d\n",(int) w->size);
  gsl_integration_workspace_free (w);
  return Q*result;
}

double computeB(double hw){
  double B;
  
  return B=sqrt(pow(HBARC,2)/(MASSN*hw));
}
