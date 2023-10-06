/**
 * @file    prng.h
 * @brief   <Provide some short description here>
 */

#ifndef PRNG_H_
#define PRNG_H_

#include <stdint.h>

uint16_t prng_xorshift(void);
uint16_t prng_xoroshiropp(void);
uint16_t prng_fib(void);

#endif /* PRNG_H_ */
