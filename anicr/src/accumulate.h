#ifndef __ACCUMULATE_H__
#define __ACCUMULATE_H__

#include <stdint.h>

void prepare_accumulate();

void accumulate_add(uint64_t key, double value);

int accumulate_get(uint64_t key, double *value);

void prepare_nlj();

void nlj_add(uint64_t key, double value);

int nlj_get(uint64_t key, double *value);

#endif/*__ACCUMULATE_H__*/
