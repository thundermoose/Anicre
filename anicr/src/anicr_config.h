
#ifndef __ANICR_CONFIG_H__
#define __ANICR_CONFIG_H__

#include FILENAME_CONFIG_H

#if REVERSED
#define REVNAME(x)  x##_rev
#else
#define REVNAME(x)  x##_forw
#endif

#endif/*__ANICR_CONFIG_H__*/
