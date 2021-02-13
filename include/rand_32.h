/* author - nocferno */

#ifndef __RAND_32_H__
#define __RAND_32_h__

#include <sys/random.h>

#include <stdbool.h>
#include <stdint.h>

/* 32-bit random number generator */
extern uint32_t get_rand_32(uint32_t, uint32_t);

#endif

