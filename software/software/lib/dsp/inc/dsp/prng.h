/**
 * @file    prng.h
 * @brief   Collection of LFSR PRNG header.
 */

#ifndef PRNG_H_
#define PRNG_H_

#include <stdint.h>

/**
 * @brief Generate pseudorandom sample with xorshift PRNG
 * 
 * @return uint16_t     Pseudorandom sample
 */
uint16_t prng_xorshift(void);

/**
 * @brief Generate pseudorandom sample with xoroshiropp PRNG
 * 
 * @return uint16_t     Pseudorandom sample
 */
uint16_t prng_xoroshiropp(void);

/**
 * @brief Generate pseudorandom sample with Fibonacci LFSR PRNG
 *
 * @return uint16_t     Pseudorandom sample
 */
uint16_t prng_fib(void);

#endif /* PRNG_H_ */
