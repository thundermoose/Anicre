
#ifndef __ANICR_CONFIG_H__
#define __ANICR_CONFIG_H__

#include FILENAME_CONFIG_H

#if REVERSED
#define CFG_PACK_WORDS     CFG_PACK_WORDS_REV
#define CFG_HASH_MP_PAD64  CFG_HASH_MP_PAD64_REV
#else
#define CFG_PACK_WORDS     CFG_PACK_WORDS_FORW
#define CFG_HASH_MP_PAD64  CFG_HASH_MP_PAD64_FORW
#endif

#if REVERSED
#define REVNAME(x)  x##_rev
#else
#define REVNAME(x)  x##_forw
#endif

#endif/*__ANICR_CONFIG_H__*/
