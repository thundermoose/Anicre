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

#include "caltbop.h"

#define MASSN 938.9187
#define HBARC 197.326963

int twob()
{
  printf("COMPUTE TWO-BODY OPERATORS\n");
  return 0;
}

int readTBME(){

  printf("Read TBME.int\n");

  return 0;
}
