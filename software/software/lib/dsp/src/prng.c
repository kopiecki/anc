/**
 * @file    prng.h
 * @brief   <Provide some short description here>
 */

#include "dsp/prng.h"

static uint16_t prng_x = 1u;

uint16_t prng_xorshift(void)
{
        prng_x ^= prng_x << 7;
        prng_x ^= prng_x >> 9;
        prng_x ^= prng_x << 8;

        return prng_x;
}

uint16_t prng_fib(void)
{
        uint16_t bit =
                ((prng_x >> 0) ^ (prng_x >> 1) ^ (prng_x >> 3) ^ (prng_x >> 12))
                & 1u;
        prng_x = (prng_x >> 1) | (bit << 15);

        return prng_x;
}
