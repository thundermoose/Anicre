#ifndef __SSE_HELPER_H__
#define __SSE_HELPER_H__

typedef long long int v2di __attribute__ ((vector_size (16)));
typedef double v2df __attribute__ ((vector_size (16)));
typedef int v4si __attribute__ ((vector_size (16)));

typedef union v2dif_t
{
  v2di _i;
  v2df _f;
} v2dif;

typedef union v2d4si_t
{
  v2di _2d;
  v4si _4s;
} v2d4si;

#endif/*__SSE_HELPER_H__*/
