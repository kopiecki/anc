/*
 *  Latency test program
 *
 *     Author: Jaroslav Kysela <perex@perex.cz>
 *
 *     Author of bandpass filter sweep effect:
 *	       Maarten de Boer <mdeboer@iua.upf.es>
 *
 *  This small demo program can be used for measuring latency between
 *  capture and playback. This latency is measured from driver (diff when
 *  playback and capture was started). Scheduler is set to SCHED_RR.
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *USA
 *
 */

#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "dsp/fir.h"
#include "dsp/lms.h"
#include "signal/signal.h"
#include "sound_card/sound_card.h"
#include "utils/circ_fifo.h"
#include "utils/file_helpers.h"

#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW CLOCK_MONOTONIC
#endif

#if defined(__OpenBSD__)
#define sched_getparam(pid, parm)             (-1)
#define sched_setscheduler(pid, policy, parm) (-1)
#endif

typedef struct timespec timestamp_t;

char* sched_policy      = "rr";
char* pdevice           = "hw:0,0";
char* cdevice           = "hw:0,0";
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
int rate                = 22050;
int channels            = 2;
int buffer_size         = 0;    /* auto */
int period_size         = 0;    /* auto */
int latency_min         = 32;   /* in frames / 2 */
int latency_max         = 2048; /* in frames / 2 */
int loop_sec            = 30;   /* seconds */
int block               = 0;    /* block mode */
int use_poll            = 0;
int usleep_val          = 0;
int resample            = 1;
int sys_latency = 0; /* data I/O: use system timings instead driver wakeups */
int pos_dump    = 0; /* dump positions */
int realtime_check = 0;
unsigned long loop_limit;
snd_pcm_uframes_t playback_buffer_size;

snd_output_t* output = NULL;
static float error_buffer[1000 * 1024];
static float x_buffer[1000 * 1024];
static float y_buffer[1000 * 1024];
static float noise_buffer[100 * 1024];
static size_t cnt = 0;
snd_pcm_channel_area_t* areas;

#define FIFO_SIZE (1 << 24u)
static struct circ_fifo x_fifo, fx_fifo;

#define PRIMARY_ORDER 500u
static float primary_w[PRIMARY_ORDER] = {0u};

#define SECONDARY_ORDER 200u
static float secondary_w[] = {
        4.56771913e-04,  4.63606793e-04,  4.52041077e-04,  3.32583419e-04,
        3.09382769e-04,  2.07631724e-04,  1.94435325e-04,  1.44763461e-04,
        9.49165485e-05,  3.74634639e-05,  -1.30312735e-05, -3.68377615e-05,
        -9.22603760e-05, -8.66892913e-05, -1.01447967e-05, 2.32802403e-05,
        5.59759937e-05,  3.13468136e-05,  4.38780009e-06,  2.49550965e-05,
        9.67909391e-07,  -1.16204635e-05, -2.32611702e-05, -6.49796751e-05,
        -1.53802248e-04, -2.38369952e-04, -3.57095045e-04, -4.84456099e-04,
        -5.53861567e-04, -5.90538489e-04, -5.51659546e-04, -5.18174710e-04,
        -5.52874514e-04, -4.91834422e-04, -4.84187264e-04, -5.17894508e-04,
        -5.27521627e-04, -4.83439480e-04, -4.61812608e-04, -4.64610372e-04,
        -4.95088719e-04, -4.09135570e-04, -4.61681542e-04, -4.28474625e-04,
        -4.30755231e-04, -3.46505096e-04, -2.62819559e-04, -1.91820258e-04,
        -1.72096502e-04, -5.22067532e-05, -2.22643865e-05, -8.96362570e-05,
        -1.21020108e-04, -1.76801016e-04, -2.51029390e-04, -2.98547870e-04,
        -3.15004340e-04, -3.43587600e-04, -4.34916326e-04, -4.03757208e-04,
        -4.19569798e-04, -3.54124796e-04, -2.54320978e-04, -3.01387105e-04,
        -2.50356589e-04, -1.71054279e-04, -1.77689536e-04, -6.85920944e-05,
        -6.75366964e-06, 1.06076697e-04,  2.12830038e-04,  2.75128653e-04,
        3.65389055e-04,  3.47047237e-04,  3.66894262e-04,  3.25168221e-04,
        3.13536913e-04,  3.49935408e-04,  2.46842549e-04,  2.24429964e-04,
        1.63058987e-04,  1.59447258e-04,  1.08149957e-04,  8.13551374e-05,
        8.53745079e-05,  3.47789663e-05,  8.28001496e-05,  9.79768614e-05,
        1.61709222e-04,  2.64481771e-04,  3.90117150e-04,  4.63883234e-04,
        5.34971020e-04,  5.83036122e-04,  5.54679259e-04,  5.59876295e-04,
        5.69492882e-04,  5.63364051e-04,  5.66544578e-04,  5.40768776e-04,
        5.88965416e-04,  5.67244730e-04,  5.47999608e-04,  5.28278590e-04,
        5.06621928e-04,  5.49232324e-04,  6.25906178e-04,  5.71286072e-04,
        5.98117280e-04,  6.05538432e-04,  6.01810599e-04,  5.81642762e-04,
        5.70149319e-04,  5.92679749e-04,  5.11667623e-04,  4.00796805e-04,
        3.22767885e-04,  2.94094842e-04,  1.57041971e-04,  7.41909676e-05,
        -1.46569683e-04, -2.83069974e-04, -3.55403481e-04, -3.87742464e-04,
        -3.11089661e-04, -3.27086914e-04, -3.57943403e-04, -3.82810796e-04,
        -3.94255896e-04, -4.17578797e-04, -3.59096751e-04, -3.93702521e-04,
        -3.78771908e-04, -3.74555610e-04, -3.60286717e-04, -3.68094242e-04,
        -3.23303591e-04, -2.50451837e-04, -3.66221163e-04, -4.32513117e-04,
        -4.07120198e-04, -4.04135305e-04, -3.81984427e-04, -2.61673278e-04,
        -2.54378475e-04, -1.91899297e-04, -1.65279212e-04, -1.19328763e-04,
        -9.92399048e-05, -1.38060824e-04, -1.00533916e-04, -9.65447097e-05,
        5.57483216e-05,  3.06718364e-04,  6.98463892e-04,  1.24245847e-03,
        1.87355161e-03,  2.59235833e-03,  3.40990271e-03,  4.12273469e-03,
        4.73473121e-03,  4.96294723e-03,  4.69302268e-03,  4.04753967e-03,
        2.87663889e-03,  1.25961628e-03,  -7.61904086e-04, -3.11342136e-03,
        -5.57510480e-03, -7.93691247e-03, -9.95292134e-03, -1.14373558e-02,
        -1.22091484e-02, -1.22156733e-02, -1.13678749e-02, -9.82768527e-03,
        -7.59528549e-03, -4.92959938e-03, -2.17228963e-03, 3.94142008e-04,
        2.60076278e-03,  4.25331566e-03,  5.30872409e-03,  5.67849600e-03,
        5.20396428e-03,  4.24970653e-03,  2.82797763e-03,  1.22832669e-03,
        -4.77432483e-04, -1.89922629e-03, -2.86773361e-03, -3.42200485e-03,
        -3.45868646e-03, -2.96370587e-03, -2.04805436e-03, -8.05252574e-04,
        6.05538654e-04,  2.09232233e-03,  3.39780801e-03,  4.47262368e-03,
};

#define LMS_STEP 0.0001

static struct signal sine_200hz = {
        .amplitude          = UINT16_MAX,
        .frequency          = 200.0,
        .sampling_frequency = 8000,
        .type               = SIGNAL_TYPE_SINE,
        .phase              = 0,
};

static struct signal white_noise = {
        .amplitude          = UINT16_MAX,
        .frequency          = 200.0,
        .sampling_frequency = 8000,
        .type               = SIGNAL_TYPE_NOISE,
        .phase              = 0,
};

static inline uint16_t to_u16(float sample)
{
        return (12 * UINT8_MAX * sample) + INT16_MAX;
}

static void generate_sine(snd_pcm_uframes_t offset, int count, double* _phase)
{
        unsigned char* samples[channels];
        int steps[channels];
        int format_bits     = snd_pcm_format_width(format);
        unsigned int maxval = (1 << (format_bits - 1)) - 1;
        int bps             = format_bits / 8; /* bytes per sample */
        int phys_bps        = snd_pcm_format_physical_width(format) / 8;

        /* verify and prepare the contents of areas */
        for (int chn = 0; chn < channels; chn++) {
                if ((areas[chn].first % 8) != 0) {
                        printf("areas[%u].first == %u, aborting...\n",
                               chn,
                               areas[chn].first);
                        exit(EXIT_FAILURE);
                }
                samples[chn] = /*(signed short *)*/ (
                        ((unsigned char*)areas[chn].addr)
                        + (areas[chn].first / 8));
                if ((areas[chn].step % 16) != 0) {
                        printf("areas[%u].step == %u, aborting...\n",
                               chn,
                               areas[chn].step);
                        exit(EXIT_FAILURE);
                }
                steps[chn] = areas[chn].step / 8;
                samples[chn] += offset * steps[chn];
        }
        /* fill the channel areas */
        size_t k = 0;
        while (count-- > 0) {
                uint16_t res = signal_generate(&sine_200hz, NULL, 0);
                for (int chn = 0; chn < channels; chn++) {
                        if (chn == 0) {
                                for (int i = 0; i < bps; i++)
                                        *(samples[chn] + i) =
                                                (res >> i * 8) & 0xff;
                                samples[chn] += steps[chn];
                        }
                }
        }
}

static inline float normalize(uint16_t sample)
{
        return 2 * ((float)sample / (float)(UINT16_MAX)) - 1;
}

void anc(char* buffer, int r)
{
        unsigned char* samples[channels];
        int steps[channels];
        int bps = 16 / 8; /* bytes per sample */

        /* verify and prepare the contents of areas */
        for (int chn = 0; chn < channels; chn++) {
                if ((areas[chn].first % 8) != 0) {
                        printf("areas[%u].first == %u, aborting...\n",
                               chn,
                               areas[chn].first);
                        exit(EXIT_FAILURE);
                }
                samples[chn] = /*(signed short *)*/ (
                        ((unsigned char*)areas[chn].addr)
                        + (areas[chn].first / 8));
                if ((areas[chn].step % 16) != 0) {
                        printf("areas[%u].step == %u, aborting...\n",
                               chn,
                               areas[chn].step);
                        exit(EXIT_FAILURE);
                }
                steps[chn] = areas[chn].step / 8;
                samples[chn] += 0 * steps[chn];
        }

        uint16_t* r_samples = (uint16_t*)buffer;

        for (int i = 0; i < r; i++) {
                const uint16_t e = r_samples[i * channels + 0] << 4;
                const uint16_t x = r_samples[i * channels + 1] << 4;

                const float e_n = normalize(e);
                const float x_n = normalize(x);

                error_buffer[cnt] = e_n;
                x_buffer[cnt] = x_n;

                circ_fifo_write(&x_fifo, x_n);

                float y = dsp_fir(&x_fifo, primary_w, PRIMARY_ORDER, 0);
                const uint16_t y_dac = to_u16(y);
                y_buffer[cnt++]        = y_dac;

                for (int chn = 0; chn < channels; chn++) {
                        for (int i = 0; i < bps; i++) {
                                if (chn == 0) {
                                        *(samples[chn] + i) =
                                                (y_dac >> i * 8) & 0xff;
                                }
                        }

                        samples[chn] += steps[chn];
                }

                const float fx =
                        dsp_fir(&x_fifo, secondary_w, SECONDARY_ORDER, 0);
                circ_fifo_write(&fx_fifo, fx);

                if (cnt < 50000) {
                        dsp_lms(primary_w, e_n, LMS_STEP, &fx_fifo, PRIMARY_ORDER, 0);
                }
        }
}

void ident(char* buffer, int r)
{
        unsigned char* samples[channels];
        int steps[channels];
        int bps = 16 / 8; /* bytes per sample */

        /* verify and prepare the contents of areas */
        for (int chn = 0; chn < channels; chn++) {
                if ((areas[chn].first % 8) != 0) {
                        printf("areas[%u].first == %u, aborting...\n",
                               chn,
                               areas[chn].first);
                        exit(EXIT_FAILURE);
                }
                samples[chn] = /*(signed short *)*/ (
                        ((unsigned char*)areas[chn].addr)
                        + (areas[chn].first / 8));
                if ((areas[chn].step % 16) != 0) {
                        printf("areas[%u].step == %u, aborting...\n",
                               chn,
                               areas[chn].step);
                        exit(EXIT_FAILURE);
                }
                steps[chn] = areas[chn].step / 8;
                samples[chn] += 0 * steps[chn];
        }

        uint16_t* r_samples = (uint16_t*)buffer;

        for (int i = 0; i < r; i++) {
                const uint16_t e = r_samples[i * channels + 0] << 4;
                const uint16_t x = r_samples[i * channels + 1] << 4;

                error_buffer[cnt] = normalize(e);
                x_buffer[cnt]     = normalize(x);

                uint16_t y          = signal_generate(&white_noise, NULL, 0);
                noise_buffer[cnt++] = normalize(y);
                for (int chn = 0; chn < channels; chn++) {
                        for (int i = 0; i < bps; i++) {
                                if (chn == 0) {
                                        *(samples[chn] + i) = (y >> i * 8) & 0xff;
                                }
                        }

                        samples[chn] += steps[chn];
                }
        }
}

static inline long long frames_to_micro(size_t frames)
{
        return (long long)((frames * 1000000LL) + (rate / 2)) / rate;
}

void timestamp_now(timestamp_t* tstamp)
{
        if (clock_gettime(CLOCK_MONOTONIC_RAW, tstamp))
                printf("clock_gettime() failed\n");
}

long long timestamp_diff_micro(timestamp_t* tstamp)
{
        timestamp_t now, diff;
        timestamp_now(&now);
        if (tstamp->tv_nsec > now.tv_nsec) {
                diff.tv_sec  = now.tv_sec - tstamp->tv_sec - 1;
                diff.tv_nsec = (now.tv_nsec + 1000000000L) - tstamp->tv_nsec;
        } else {
                diff.tv_sec  = now.tv_sec - tstamp->tv_sec;
                diff.tv_nsec = now.tv_nsec - tstamp->tv_nsec;
        }
        /* microseconds */
        return (diff.tv_sec * 1000000) + ((diff.tv_nsec + 500L) / 1000L);
}

int setparams_stream(snd_pcm_t* handle, snd_pcm_hw_params_t* params, const char* id)
{
        int err;
        unsigned int rrate;

        err = snd_pcm_hw_params_any(handle, params);
        if (err < 0) {
                printf("Broken configuration for %s PCM: no configurations "
                       "available: %s\n",
                       snd_strerror(err),
                       id);
                return err;
        }
        err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
        if (err < 0) {
                printf("Resample setup failed for %s (val %i): %s\n",
                       id,
                       resample,
                       snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_set_access(
                handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        if (err < 0) {
                printf("Access type not available for %s: %s\n",
                       id,
                       snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_set_format(handle, params, format);
        if (err < 0) {
                printf("Sample format not available for %s: %s\n",
                       id,
                       snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_set_channels(handle, params, channels);
        if (err < 0) {
                printf("Channels count (%i) not available for %s: %s\n",
                       channels,
                       id,
                       snd_strerror(err));
                return err;
        }
        rrate = rate;
        err   = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
        if (err < 0) {
                printf("Rate %iHz not available for %s: %s\n",
                       rate,
                       id,
                       snd_strerror(err));
                return err;
        }
        if ((int)rrate != rate) {
                printf("Rate doesn't match (requested %iHz, get %iHz)\n",
                       rate,
                       err);
                return -EINVAL;
        }
        /* we do not want driver wakeups */
        if (sys_latency > 0
            && snd_pcm_hw_params_can_disable_period_wakeup(params)) {
                err = snd_pcm_hw_params_set_period_wakeup(handle, params, 0);
                if (err < 0) {
                        printf("Cannot disable period wakeups for %s\n", id);
                        return err;
                }
        }
        return 0;
}

int setparams_bufsize(
        snd_pcm_t* handle,
        snd_pcm_hw_params_t* params,
        snd_pcm_hw_params_t* tparams,
        snd_pcm_uframes_t bufsize,
        const char* id)
{
        int err;
        snd_pcm_uframes_t periodsize;

        snd_pcm_hw_params_copy(params, tparams);
        periodsize = bufsize * 2;
        err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &periodsize);
        if (err < 0) {
                printf("Unable to set buffer size %li for %s: %s\n",
                       bufsize * 2,
                       id,
                       snd_strerror(err));
                return err;
        }
        if (period_size > 0)
                periodsize = period_size;
        else
                periodsize /= 2;
        err = snd_pcm_hw_params_set_period_size_near(
                handle, params, &periodsize, 0);
        if (err < 0) {
                printf("Unable to set period size %li for %s: %s\n",
                       periodsize,
                       id,
                       snd_strerror(err));
                return err;
        }
        return 0;
}

int setparams_set(
        snd_pcm_t* handle,
        snd_pcm_hw_params_t* params,
        snd_pcm_sw_params_t* swparams,
        const char* id)
{
        int err;
        snd_pcm_uframes_t val;

        err = snd_pcm_hw_params(handle, params);
        if (err < 0) {
                printf("Unable to set hw params for %s: %s\n",
                       id,
                       snd_strerror(err));
                return err;
        }
        err = snd_pcm_sw_params_current(handle, swparams);
        if (err < 0) {
                printf("Unable to determine current swparams for %s: %s\n",
                       id,
                       snd_strerror(err));
                return err;
        }
        err = snd_pcm_sw_params_set_start_threshold(handle, swparams, 0x7fffffff);
        if (err < 0) {
                printf("Unable to set start threshold mode for %s: %s\n",
                       id,
                       snd_strerror(err));
                return err;
        }
        if (!block)
                val = 4;
        else
                snd_pcm_hw_params_get_period_size(params, &val, NULL);
        err = snd_pcm_sw_params_set_avail_min(handle, swparams, val);
        if (err < 0) {
                printf("Unable to set avail min for %s: %s\n",
                       id,
                       snd_strerror(err));
                return err;
        }
        err = snd_pcm_sw_params(handle, swparams);
        if (err < 0) {
                printf("Unable to set sw params for %s: %s\n",
                       id,
                       snd_strerror(err));
                return err;
        }
        return 0;
}

int setparams(snd_pcm_t* phandle, snd_pcm_t* chandle, int* bufsize)
{
        int err, last_bufsize = *bufsize;
        snd_pcm_hw_params_t *pt_params, *ct_params; /* templates with rate,
                                                       format and channels */
        snd_pcm_hw_params_t *p_params, *c_params;
        snd_pcm_sw_params_t *p_swparams, *c_swparams;
        snd_pcm_uframes_t p_size, c_size, p_psize, c_psize;
        unsigned int p_time, c_time;
        unsigned int val;

        snd_pcm_hw_params_alloca(&p_params);
        snd_pcm_hw_params_alloca(&c_params);
        snd_pcm_hw_params_alloca(&pt_params);
        snd_pcm_hw_params_alloca(&ct_params);
        snd_pcm_sw_params_alloca(&p_swparams);
        snd_pcm_sw_params_alloca(&c_swparams);
        if ((err = setparams_stream(phandle, pt_params, "playback")) < 0) {
                printf("Unable to set parameters for playback stream: %s\n",
                       snd_strerror(err));
                exit(0);
        }
        if ((err = setparams_stream(chandle, ct_params, "capture")) < 0) {
                printf("Unable to set parameters for playback stream: %s\n",
                       snd_strerror(err));
                exit(0);
        }

        if (buffer_size > 0) {
                *bufsize = buffer_size;
                goto __set_it;
        }

__again:
        if (buffer_size > 0)
                return -1;
        if (last_bufsize == *bufsize)
                *bufsize += 4;
        last_bufsize = *bufsize;
        if (*bufsize > latency_max)
                return -1;
__set_it:
        if ((err = setparams_bufsize(
                     phandle, p_params, pt_params, *bufsize, "playback"))
            < 0) {
                printf("Unable to set sw parameters for playback stream: %s\n",
                       snd_strerror(err));
                exit(0);
        }
        if ((err = setparams_bufsize(
                     chandle, c_params, ct_params, *bufsize, "capture"))
            < 0) {
                printf("Unable to set sw parameters for playback stream: %s\n",
                       snd_strerror(err));
                exit(0);
        }

        snd_pcm_hw_params_get_period_size(p_params, &p_psize, NULL);
        if (p_psize > (unsigned int)*bufsize)
                *bufsize = p_psize;
        snd_pcm_hw_params_get_period_size(c_params, &c_psize, NULL);
        if (c_psize > (unsigned int)*bufsize)
                *bufsize = c_psize;
        snd_pcm_hw_params_get_period_time(p_params, &p_time, NULL);
        snd_pcm_hw_params_get_period_time(c_params, &c_time, NULL);

        snd_pcm_hw_params_get_buffer_size(p_params, &p_size);
        playback_buffer_size = p_size;
        if (p_psize * 2 < p_size) {
                snd_pcm_hw_params_get_periods_min(p_params, &val, NULL);
                if (val > 2) {
                        printf("playback device does not support 2 periods per "
                               "buffer\n");
                        exit(0);
                }
                goto __again;
        }
        snd_pcm_hw_params_get_buffer_size(c_params, &c_size);
        if (c_psize * 2 < c_size) {
                snd_pcm_hw_params_get_periods_min(c_params, &val, NULL);
                if (val > 2) {
                        printf("capture device does not support 2 periods per "
                               "buffer\n");
                        exit(0);
                }
                goto __again;
        }
        if ((err = setparams_set(phandle, p_params, p_swparams, "playback")) < 0) {
                printf("Unable to set sw parameters for playback stream: %s\n",
                       snd_strerror(err));
                exit(0);
        }
        if ((err = setparams_set(chandle, c_params, c_swparams, "capture")) < 0) {
                printf("Unable to set sw parameters for playback stream: %s\n",
                       snd_strerror(err));
                exit(0);
        }

        if ((err = snd_pcm_prepare(phandle)) < 0) {
                printf("Prepare error: %s\n", snd_strerror(err));
                exit(0);
        }

        snd_pcm_dump(phandle, output);
        snd_pcm_dump(chandle, output);
        fflush(stdout);
        return 0;
}

void showstat(snd_pcm_t* handle, size_t frames)
{
        int err;
        snd_pcm_status_t* status;

        snd_pcm_status_alloca(&status);
        if ((err = snd_pcm_status(handle, status)) < 0) {
                printf("Stream status error: %s\n", snd_strerror(err));
                exit(0);
        }
        printf("*** frames = %li ***\n", (long)frames);
        snd_pcm_status_dump(status, output);
}

void showlatency(size_t latency)
{
        double d;
        latency *= 2;
        d = (double)latency / (double)rate;
        printf("Trying latency %li frames, %.3fus, %.6fms (%.4fHz)\n",
               (long)latency,
               d * 1000000,
               d * 1000,
               (double)1 / d);
}

void showinmax(size_t in_max)
{
        double d;

        printf("Maximum read: %li frames\n", (long)in_max);
        d = (double)in_max / (double)rate;
        printf("Maximum read latency: %.3fus, %.6fms (%.4fHz)\n",
               d * 1000000,
               d * 1000,
               (double)1 / d);
}

void gettimestamp(snd_pcm_t* handle, snd_timestamp_t* timestamp)
{
        int err;
        snd_pcm_status_t* status;

        snd_pcm_status_alloca(&status);
        if ((err = snd_pcm_status(handle, status)) < 0) {
                printf("Stream status error: %s\n", snd_strerror(err));
                exit(0);
        }
        snd_pcm_status_get_trigger_tstamp(status, timestamp);
}

void setscheduler(void)
{
        struct sched_param sched_param;
        int policy          = SCHED_RR;
        const char* spolicy = "Round Robin";

        if (strcasecmp(sched_policy, "fifo") == 0) {
                policy  = SCHED_FIFO;
                spolicy = "FIFO";
        } else if (strcasecmp(sched_policy, "other") == 0) {
                policy  = SCHED_OTHER;
                spolicy = "OTHER";
        }
        if (sched_getparam(0, &sched_param) < 0) {
                printf("Scheduler getparam failed...\n");
                return;
        }
        sched_param.sched_priority = sched_get_priority_max(policy);
        if (!sched_setscheduler(0, policy, &sched_param)) {
                printf("Scheduler set to %s with priority %i...\n",
                       spolicy,
                       sched_param.sched_priority);
                fflush(stdout);
                return;
        }
        printf("!!!Scheduler set to %s with priority %i FAILED!!!\n",
               spolicy,
               sched_param.sched_priority);
}

long timediff(snd_timestamp_t t1, snd_timestamp_t t2)
{
        signed long l;

        t1.tv_sec -= t2.tv_sec;
        l = (signed long)t1.tv_usec - (signed long)t2.tv_usec;
        if (l < 0) {
                t1.tv_sec--;
                l = 1000000 + l;
                l %= 1000000;
        }
        return (t1.tv_sec * 1000000) + l;
}

long readbuf(snd_pcm_t* handle, char* buf, long len, size_t* frames, size_t* max)
{
        long r;

        if (!block) {
                do {
                        r = snd_pcm_readi(handle, buf, len);
                } while (r == -EAGAIN);
                if (r > 0) {
                        *frames += r;
                        if ((long)*max < r)
                                *max = r;
                }
                // printf("read = %li\n", r);
        } else {
                int frame_bytes =
                        (snd_pcm_format_physical_width(format) / 8) * channels;
                do {
                        r = snd_pcm_readi(handle, buf, len);
                        if (r > 0) {
                                buf += r * frame_bytes;
                                len -= r;
                                *frames += r;
                                if ((long)*max < r)
                                        *max = r;
                        }
                        // printf("r = %li, len = %li\n", r, len);
                } while (r >= 1 && len > 0);
        }
        // showstat(handle, 0);
        return r;
}

long writebuf(snd_pcm_t* handle, char* buf, long len, size_t* frames)
{
        long r;
        int frame_bytes = (snd_pcm_format_physical_width(format) / 8) * channels;

        while (len > 0) {
                r = snd_pcm_writei(handle, buf, len);
                if (r == -EAGAIN)
                        continue;
                // printf("write = %li\n", r);
                if (r < 0)
                        return r;
                // showstat(handle, 0);
                buf += r * frame_bytes;
                len -= r;
                *frames += r;
        }
        return 0;
}

#define FILTERSWEEP_LFO_CENTER 2000.
#define FILTERSWEEP_LFO_DEPTH  1800.
#define FILTERSWEEP_LFO_FREQ   0.2
#define FILTER_BANDWIDTH       50

/* filter the sweep variables */
float lfo, dlfo, fs, fc, BW, C, D, a0, a1, a2, b1, b2, *x[3], *y[3];

static ssize_t get_avail(snd_pcm_t* pcm)
{
        ssize_t avail;

        while (1) {
                avail = snd_pcm_avail(pcm);
                if (avail == -EAGAIN)
                        continue;
                break;
        }
        return avail;
}

void help(void)
{
        int k;
        printf("Usage: latency [OPTION]... [FILE]...\n"
               "-h,--help      help\n"
               "-P,--pdevice   playback device\n"
               "-C,--cdevice   capture device\n"
               "-m,--min       minimum latency in frames\n"
               "-M,--max       maximum latency in frames\n"
               "-U,--updates   I/O updates in milliseconds (0 = off)\n"
               "-F,--frames    frames to transfer\n"
               "-f,--format    sample format\n"
               "-c,--channels  channels\n"
               "-r,--rate      rate\n"
               "-B,--buffer    buffer size in frames\n"
               "-E,--period    period size in frames\n"
               "-s,--seconds   duration of test in seconds\n"
               "-b,--block     block mode\n"
               "-p,--poll      use poll (wait for event - reduces CPU usage)\n"
               "-y,--usleep    sleep for the specified amount of microseconds "
               "between\n"
               "               stream updates (default 0 - off)\n"
               "-e,--effect    apply an effect (bandpass filter sweep)\n"
               "-x,--posdump   dump buffer positions\n"
               "-X,--realtime  do a realtime check (buffering)\n"
               "-O,--policy    set scheduler policy (RR, FIFO or OTHER)\n");
        printf("Recognized sample formats are:");
        for (k = 0; k < SND_PCM_FORMAT_LAST; ++k) {
                const char* s = snd_pcm_format_name(k);
                if (s)
                        printf(" %s", s);
        }
        printf("\n\n");
        printf("Tip #1 (usable latency with large periods, non-blocking mode, "
               "good CPU usage,\n"
               "        superb xrun prevention):\n"
               "  latency -m 8192 -M 8192 -t 1 -p\n"
               "Tip #2 (superb latency, non-blocking mode, but heavy CPU "
               "usage):\n"
               "  latency -m 128 -M 128\n");
}

int main(int argc, char* argv[])
{
        struct option long_option[] = {
                {"help", 0, NULL, 'h'},     {"pdevice", 1, NULL, 'P'},
                {"cdevice", 1, NULL, 'C'},  {"min", 1, NULL, 'm'},
                {"max", 1, NULL, 'M'},      {"updates", 1, NULL, 'U'},
                {"frames", 1, NULL, 'F'},   {"format", 1, NULL, 'f'},
                {"channels", 1, NULL, 'c'}, {"rate", 1, NULL, 'r'},
                {"buffer", 1, NULL, 'B'},   {"period", 1, NULL, 'E'},
                {"seconds", 1, NULL, 's'},  {"block", 0, NULL, 'b'},
                {"poll", 0, NULL, 'p'},     {"usleep", 1, NULL, 'y'},
                {"effect", 0, NULL, 'e'},   {"posdump", 0, NULL, 'x'},
                {"realtime", 0, NULL, 'X'}, {"policy", 1, NULL, 'O'},
                {NULL, 0, NULL, 0},
        };
        snd_pcm_t *phandle, *chandle;
        char* buffer;
        int err, latency, morehelp;
        int ok;
        snd_timestamp_t p_tstamp, c_tstamp;
        ssize_t r, cap_avail, cap_avail_max, pbk_fill, pbk_fill_min;
        size_t frames_in, frames_out, in_max;
        timestamp_t tstamp_start;
        int effect = 0;
        morehelp   = 0;

        // file_helpers_read_buf("sec_coeffs.txt", secondary_w);

        while (1) {
                int c;
                if ((c = getopt_long(
                             argc,
                             argv,
                             "hP:C:m:M:U:F:f:c:r:B:E:s:y:O:bpenxX",
                             long_option,
                             NULL))
                    < 0)
                        break;
                switch (c) {
                        case 'h':
                                morehelp++;
                                break;
                        case 'P':
                                pdevice = strdup(optarg);
                                break;
                        case 'C':
                                cdevice = strdup(optarg);
                                break;
                        case 'm':
                                err         = atoi(optarg) / 2;
                                latency_min = err >= 4 ? err : 4;
                                if (latency_max < latency_min)
                                        latency_max = latency_min;
                                break;
                        case 'M':
                                err = atoi(optarg) / 2;
                                latency_max =
                                        latency_min > err ? latency_min : err;
                                break;
                        case 'U':
                                err         = atoi(optarg);
                                sys_latency = err <= 0 ? 0 : err;
                                break;
                        case 'f':
                                format = snd_pcm_format_value(optarg);
                                if (format == SND_PCM_FORMAT_UNKNOWN) {
                                        printf("Unknown format, setting to "
                                               "default S16_LE\n");
                                        format = SND_PCM_FORMAT_S16_LE;
                                }
                                break;
                        case 'c':
                                err      = atoi(optarg);
                                channels = err >= 1 && err < 1024 ? err : 1;
                                break;
                        case 'r':
                                err  = atoi(optarg);
                                rate = err >= 4000 && err < 200000 ? err : 44100;
                                break;
                        case 'B':
                                err         = atoi(optarg);
                                buffer_size = err >= 32 && err < 200000 ? err : 0;
                                break;
                        case 'E':
                                err         = atoi(optarg);
                                period_size = err >= 32 && err < 200000 ? err : 0;
                                break;
                        case 's':
                                err      = atoi(optarg);
                                loop_sec = err >= 1 && err <= 100000 ? err : 30;
                                break;
                        case 'b':
                                block = 1;
                                break;
                        case 'p':
                                use_poll = 1;
                                break;
                        case 'y':
                                usleep_val = atoi(optarg);
                                break;
                        case 'e':
                                effect = 1;
                                break;
                        case 'n':
                                resample = 0;
                                break;
                        case 'x':
                                pos_dump = 1;
                                break;
                        case 'X':
                                realtime_check = 1;
                                break;
                        case 'O':
                                sched_policy = optarg;
                                break;
                }
        }

        if (morehelp) {
                help();
                return 0;
        }
        err = snd_output_stdio_attach(&output, stdout, 0);
        if (err < 0) {
                printf("Output failed: %s\n", snd_strerror(err));
                return 0;
        }

        loop_limit = loop_sec * rate;
        latency    = latency_min - 4;
        buffer     = malloc(
                (latency_max * 2 * snd_pcm_format_physical_width(format) / 8)
                * channels);

        /* I/O updates based on a system timer */
        if (sys_latency > 0) {
                block    = 0;
                use_poll = 0;
        }

        setscheduler();

        printf("Playback device is %s\n", pdevice);
        printf("Capture device is %s\n", cdevice);
        printf("Parameters are %iHz, %s, %i channels, %s mode, use poll %s\n",
               rate,
               snd_pcm_format_name(format),
               channels,
               block ? "blocking" : "non-blocking",
               use_poll ? "yes" : "no");
        printf("Loop limit is %lu frames, minimum latency = %i, maximum "
               "latency = %i",
               loop_limit,
               latency_min * 2,
               latency_max * 2);
        if (sys_latency > 0)
                printf(", I/O updates %ims", sys_latency);
        else if (!block && !use_poll)
                printf(", I/O usleep %ius", usleep_val);
        printf("\n");

        if ((err = snd_pcm_open(
                     &phandle,
                     pdevice,
                     SND_PCM_STREAM_PLAYBACK,
                     block ? 0 : SND_PCM_NONBLOCK))
            < 0) {
                printf("Playback open error: %s\n", snd_strerror(err));
                return 0;
        }
        if ((err = snd_pcm_open(
                     &chandle,
                     cdevice,
                     SND_PCM_STREAM_CAPTURE,
                     block ? 0 : SND_PCM_NONBLOCK))
            < 0) {
                printf("Record open error: %s\n", snd_strerror(err));
                return 0;
        }

        /* initialize the filter sweep variables */
        if (effect) {
                fs = (float)rate;
                BW = FILTER_BANDWIDTH;

                lfo  = 0;
                dlfo = 2. * M_PI * FILTERSWEEP_LFO_FREQ / fs;

                x[0] = (float*)malloc(channels * sizeof(float));
                x[1] = (float*)malloc(channels * sizeof(float));
                x[2] = (float*)malloc(channels * sizeof(float));
                y[0] = (float*)malloc(channels * sizeof(float));
                y[1] = (float*)malloc(channels * sizeof(float));
                y[2] = (float*)malloc(channels * sizeof(float));
        }

        cap_avail_max = 0;
        pbk_fill_min  = latency * 2;

        areas = calloc(channels, sizeof(snd_pcm_channel_area_t));
        if (areas == NULL) {
                printf("No enough memory\n");
                exit(EXIT_FAILURE);
        }
        for (int chn = 0; chn < channels; chn++) {
                areas[chn].addr  = buffer;
                areas[chn].first = chn * snd_pcm_format_physical_width(format);
                areas[chn].step =
                        channels * snd_pcm_format_physical_width(format);
        }

        int error = circ_fifo_construct(&x_fifo, NULL, FIFO_SIZE);
        if (error < 0)
                return error;

        error = circ_fifo_construct(&fx_fifo, NULL, FIFO_SIZE);
        if (error < 0)
                return error;

        while (1) {
                frames_in = frames_out = 0;
                if (setparams(phandle, chandle, &latency) < 0) {
                        printf("Cannot set params!");
                        break;
                }
                showlatency(latency);
                if ((err = snd_pcm_link(chandle, phandle)) < 0) {
                        printf("Streams link error: %s\n", snd_strerror(err));
                        exit(0);
                }
                if (snd_pcm_format_set_silence(format, buffer, latency * channels)
                    < 0) {
                        fprintf(stderr, "silence error\n");
                        break;
                }

                anc(buffer, latency);
                if (writebuf(phandle, buffer, latency, &frames_out) < 0) {
                        fprintf(stderr, "write error\n");
                        break;
                }

                anc(buffer, latency);
                if (writebuf(phandle, buffer, latency, &frames_out) < 0) {
                        fprintf(stderr, "write error\n");
                        break;
                }

                if (realtime_check)
                        timestamp_now(&tstamp_start);
                if ((err = snd_pcm_start(chandle)) < 0) {
                        printf("Go error: %s\n", snd_strerror(err));
                        exit(0);
                }
                if (realtime_check)
                        printf("[%lldus] Stream start\n",
                               timestamp_diff_micro(&tstamp_start));
                gettimestamp(phandle, &p_tstamp);
                gettimestamp(chandle, &c_tstamp);
#if 0
		printf("Playback:\n");
		showstat(phandle, frames_out);
		printf("Capture:\n");
		showstat(chandle, frames_in);
#endif

                ok     = 1;
                in_max = 0;
                while (ok && frames_in < loop_limit) {
                        cap_avail = latency;
                        if (sys_latency > 0) {
                                poll(NULL, 0, sys_latency);
                                cap_avail = get_avail(chandle);
                                if (cap_avail < 0) {
                                        printf("Avail failed: %s\n",
                                               snd_strerror(cap_avail));
                                        ok = 0;
                                        break;
                                }
                        } else if (use_poll) {
                                /* use poll to wait for next event */
                                snd_pcm_wait(chandle, 1000);
                        } else if (!block && usleep_val > 0) {
                                usleep(usleep_val);
                        }
                        if (pos_dump || realtime_check) {
                                if (sys_latency <= 0)
                                        cap_avail = get_avail(chandle);
                                pbk_fill = get_avail(phandle);
                                if (pbk_fill >= 0)
                                        pbk_fill =
                                                playback_buffer_size - pbk_fill;
                                if (cap_avail > cap_avail_max)
                                        cap_avail_max = cap_avail;
                                if (pbk_fill >= 0 && pbk_fill < pbk_fill_min)
                                        pbk_fill_min = pbk_fill;
                                if (realtime_check) {
                                        long long diff = timestamp_diff_micro(
                                                &tstamp_start);
                                        long long cap_pos = frames_to_micro(
                                                frames_in + cap_avail);
                                        long long pbk_pos = frames_to_micro(
                                                frames_out - pbk_fill);
                                        printf("[%lldus] POS: p=%zd (min=%zd, "
                                               "rt=%lldus) c=%zd (max=%zd, "
                                               "rt=%lldus)\n",
                                               diff,
                                               pbk_fill,
                                               pbk_fill_min,
                                               pbk_pos - diff,
                                               cap_avail,
                                               cap_avail_max,
                                               cap_pos - diff);
                                } else if (pos_dump) {
                                        printf("POS: p=%zd (min=%zd), c=%zd "
                                               "(max=%zd)\n",
                                               pbk_fill,
                                               pbk_fill_min,
                                               cap_avail,
                                               cap_avail_max);
                                }
                        }
                        if ((r = readbuf(
                                     chandle, buffer, cap_avail, &frames_in, &in_max))
                            < 0)
                                ok = 0;
                        else {
                                if (effect)
                                        anc(buffer, r);
                                if (writebuf(phandle, buffer, r, &frames_out) < 0)
                                        ok = 0;
                        }
                }
                if (ok)
                        printf("Success\n");
                else
                        printf("Failure\n");
                printf("Playback:\n");
                showstat(phandle, frames_out);
                printf("Capture:\n");
                showstat(chandle, frames_in);
                showinmax(in_max);
                if (p_tstamp.tv_sec == c_tstamp.tv_sec
                    && p_tstamp.tv_usec == c_tstamp.tv_usec)
                        printf("Hardware sync\n");
                if (realtime_check) {
                        long long diff  = timestamp_diff_micro(&tstamp_start);
                        long long mtime = frames_to_micro(frames_in);
                        printf("Elapsed real time: %lldus\n", diff);
                        printf("Elapsed device time: %lldus\n", mtime);
                        printf("Test time diff (device - real): %lldus\n",
                               mtime - diff);
                }
                snd_pcm_drop(chandle);
                snd_pcm_nonblock(phandle, 0);
                snd_pcm_drain(phandle);
                snd_pcm_nonblock(phandle, !block ? 1 : 0);
                if (ok) {
#if 1
                        printf("Playback time = %li.%i, Record time = %li.%i, "
                               "diff = %li\n",
                               (long)p_tstamp.tv_sec,
                               (int)p_tstamp.tv_usec,
                               (long)c_tstamp.tv_sec,
                               (int)c_tstamp.tv_usec,
                               timediff(p_tstamp, c_tstamp));
#endif
                        break;
                }
                snd_pcm_unlink(chandle);
                snd_pcm_hw_free(phandle);
                snd_pcm_hw_free(chandle);
        }

        snd_pcm_close(phandle);
        snd_pcm_close(chandle);
        snd_output_close(output);
        snd_config_update_free_global();
        free(buffer);
        free(pdevice);
        free(cdevice);

        file_helpers_write_buf("error.txt", '\n', error_buffer, cnt, false);
        file_helpers_write_buf("noise.txt", '\n', noise_buffer, cnt, false);
        file_helpers_write_buf("y.txt", '\n', y_buffer, cnt, false);
        file_helpers_write_buf("x.txt", '\n', x_buffer, cnt, false);

        return 0;
}
