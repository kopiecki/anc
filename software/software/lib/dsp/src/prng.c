/**
 * @file    prng.h
 * @brief   <Provide some short description here>
 */

#include "dsp/prng.h"

#define N_STATES 2u

static const uint16_t prng_a     = 13;
static const uint16_t prng_b     = 5;
static const uint16_t prng_c     = 10;
static const uint16_t prng_d     = 9;
static uint16_t prng_x           = 0xDEAD;
static uint16_t prng_s[N_STATES] = {1, 0};

static inline uint16_t prng_rol(uint16_t x, uint16_t k)
{
        return (x << k) | (x >> (16 - k));
}

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

uint16_t prng_xoroshiropp(void)
{
        uint16_t y = prng_rol(prng_s[0] + prng_s[1], prng_d) + prng_s[0];

        prng_s[1] ^= prng_s[0];
        prng_s[0] = prng_rol(prng_s[0], prng_a) ^ prng_s[1] ^ (prng_s[1] << prng_b);
        prng_s[1] = prng_rol(prng_s[1], prng_c);

        return y;
}
