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

#include "calobop.h"
#include "caltbop.h"
//#define M_PI 3.14159265358979323846

#define MASSN 938.9187
#define HBARC 197.326963

int oneb()
{

  char filename_p[14]="nlj_out-p.bin";
  char filename_n[14]="nlj_out-n.bin";
 
  size_t num_nlj_2=CFG_NUM_NLJ_STATES*CFG_NUM_NLJ_STATES;
  int jtrans_min = abs(CFG_2J_INITIAL - CFG_2J_FINAL);
  int jtrans_max = CFG_2J_INITIAL + CFG_2J_FINAL;
  
  int num_jtrans=(jtrans_max-jtrans_min)/2+1;
 
  double final_p[num_jtrans][num_nlj_2];
  double final_n[num_jtrans][num_nlj_2];
 
  FILE *fpr=NULL;
  FILE *fn=NULL;

  fpr=fopen(filename_p,"rb");
  fn=fopen(filename_n,"rb");
  int jtrans;
  int ii=0;
  printf("Reading transition densities\n");
  for(ii=0;ii<num_jtrans;ii++){
    if(fpr!=NULL){
      fread(final_p[ii],sizeof(double),num_nlj_2,fpr);
     
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
 
  FILE *fp;
  fp=fopen("obs.txt","w");
  if (fp == NULL) {
  fprintf(stderr, "Can't open output file in!\n");
  exit(1);
  }
  printf("Writing output to obs.txt\n");
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
  float hw=CFG_HW;
 
  fprintf(fp," A=%3d   Z=%3d   N=%3d\n",A,Z,N);
  fprintf(fp," 2*MJ=%3d   2*MT=%3d  parity= %c \n",CFG_2M_INITIAL,two_MT,parity);
  fprintf(fp," hbar Omega=%8.4f",hw);
 
  ii=0;
  
  int showJtrans=0;
  double Qp=0.0,Qn=0.0;
  double b=computeB(hw);
  int numberStep=100;
  double rmin=0.0;
  double rmax=6.0;
  double r[numberStep];
  double Nr[numberStep];
  double Pr[numberStep];

  double sumPr=0.0;
  double sumNr=0.0;
  for(int i=0; i<numberStep;i++){
    r[i]=(rmax-rmin)/(numberStep-1)*i+rmin;
  }

  gsl_sf_result result;

  ii=0;
  //  printf(" b= %f hw= %f\n",b,hw);
  fprintf(fp,"\n Magnetic moment:\n");
  for (jtrans = jtrans_min; jtrans <= jtrans_max; jtrans += 2)
   {

    for(int i=0; i<numberStep;i++){
      Pr[i]=0.0;
      Nr[i]=0.0;
    }
    Qp=0.0;
    Qn=0.0;
    
    if (jtrans/2%2!=0){
    	ii++;
	    continue;
	    }  //Only for Iparityi==IparityF
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
		   //        printf("\n Jtrans=%3d\n",jtrans/2);
	           showJtrans=0;
	         }

		
	      	 int ret =
		   		   gsl_sf_coupling_3j_e(CFG_2J_INITIAL,jtrans,CFG_2J_FINAL,CFG_2J_INITIAL,0,-CFG_2J_INITIAL,
	    				&result);
    
          if (ret != GSL_SUCCESS)
          {
            fprintf (stderr,"ERR! %d\n", ret);
            exit(1);
          }
	        Qp+=obmeQ(na,la,ja,nb,lb,jb,jtrans/2,b)*final_p[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES];
          Qn+=obmeQ(na,la,ja,nb,lb,jb,jtrans/2,b)*final_n[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES];
          double Rp=obmeSH(la,ja,lb,jb,jtrans/2)*final_p[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES];
          double Rn=obmeSH(la,ja,lb,jb,jtrans/2)*final_n[ii][sp_anni+sp_crea*CFG_NUM_NLJ_STATES];
        
  //        printf("Rfactor: %f %f \n",Rp,Rn);
          if(jtrans==0){   
            
            for(int i=0; i<numberStep;i++){
              Pr[i]+=Rp*radialHO(r[i],b,na,la)*radialHO(r[i],b,nb,lb)*3.54491;
              Nr[i]+=Rn*radialHO(r[i],b,na,la)*radialHO(r[i],b,nb,lb)*3.54491;
              sumPr+=Pr[i];
              sumNr+=Nr[i];
            }
          }
        }

       }  
     }
     ii++;

     fprintf(fp,"B(E) %f p: %f n: %f \n", pow(Qp,2)/(CFG_2J_INITIAL+1),Qp,Qn);

     char filename[15];
     sprintf(filename,"output_r_%d.txt",jtrans/2);
     printf("Writing radial output to: output_r_%d.txt\n",jtrans/2);
     FILE *routput;
     
     routput = fopen(filename, "w");
     for(int i=0; i< numberStep; i++){
       fprintf(routput, "%4.2f  %8.5f %8.5f\n", r[i],Pr[i],Nr[i]);
     }
     fclose(routput);

   }


  fclose(fp);

  return 0;
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
  temp=1./sqrt(4.*M_PI)*(1+pow(-1.,l1+l2+lambda))/2.0*pow(-1,jj2/2.0-0.5+lambda);
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
  
double radialQ(double x,void *p){
  //Parameters to integration of R_aR_b
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

  Q=obmeSH(la,jja,lb,jjb,lambda);// Biedenhar-Rose: *pow(-1,0.5*(la+lb+lambda));


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




/*double rangularint(int K,int mk){
  //Compute \int Y_KK(theta,rho) sin(theta) dtheta drho
  
  double integral=0.0;

  return integral;
}*/
double computeB(double hw){
  double B;
  
  return B=sqrt(pow(HBARC,2)/(MASSN*hw));
}
