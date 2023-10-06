/**
 * @file    test_prng.c
 * @brief   <Provide some short description here>
 */

#include "dsp/prng.h"
#include "signal/signal.h"
#include "utils/file_helpers.h"
#include "utils/logger.h"

#define N_SAMPLES 1024 << 4

static uint16_t randn_buf[N_SAMPLES] = {0u};
static uint16_t xors_buf[N_SAMPLES]  = {0u};

static int generate_noises()
{
        struct signal noise = {
                .type = SIGNAL_TYPE_NOISE,
        };

        signal_initialize_noise();

        for (size_t i = 0u; i < N_SAMPLES; ++i) {
                randn_buf[i] = signal_generate(&noise, NULL, 1);
                xors_buf[i]  = prng_xoroshiropp();
        }

        int error = file_helpers_write_buf(
                "test/functional/output/randn.csv",
                '\n',
                randn_buf,
                N_SAMPLES,
                false);
        if (error < 0)
                return error;

        error = file_helpers_write_buf(
                "test/functional/output/xorshift.csv",
                '\n',
                xors_buf,
                N_SAMPLES,
                false);
        
        return error;
}

int main(void)
{
        LOG_INFO("Running PRNG test");
        int error = generate_noises();
        if (error < 0)
                LOG_ERROR("PRNG test failed");

        return error;
}
