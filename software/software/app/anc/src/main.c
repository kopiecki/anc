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
static uint16_t left_buffer[100 * 1024];
static uint16_t right_buffer[100 * 1024];
static uint16_t noise_buffer[100 * 1024];
static uint16_t error_buffer[100 * 1024];
static uint16_t y_buffer[100 * 1024];
static size_t n_cnt = 0;
static size_t cnt   = 0;
snd_pcm_channel_area_t* areas;

#define FIFO_SIZE (1 << 24u)
static struct circ_fifo x_fifo, fx_fifo;

#define PRIMARY_ORDER 800u
static float primary_w[PRIMARY_ORDER] = {0u};

#define SECONDARY_ORDER 400u
static float secondary_w[] = {
        2.914938882094222916e-03,  3.715823137623394928e-03,
        4.327360718113375769e-03,  4.671737653523082463e-03,
        4.810362608570333334e-03,  4.770711707070958603e-03,
        4.670454604384994407e-03,  4.539953338172769519e-03,
        4.522663516106882609e-03,  4.644513544243297418e-03,
        4.942615955910340496e-03,  5.460386075494620337e-03,
        6.134386722524723563e-03,  6.808735212357986018e-03,
        7.416122665774929797e-03,  7.834899418786971451e-03,
        8.018091964602023017e-03,  7.855745753904576703e-03,
        7.317008904325348825e-03,  6.492420958140133688e-03,
        5.427303001526346399e-03,  4.232523210101102502e-03,
        3.009663824828076161e-03,  1.852171072396077093e-03,
        9.025738159251474801e-04,  1.901871879322820143e-04,
        -2.572196681714410734e-04, -4.120479522416343066e-04,
        -3.616023762506436357e-04, -1.918762356154239039e-04,
        -1.066242957749070631e-05, 5.842793640585547946e-05,
        -8.722506697833152416e-05, -5.080974144613520632e-04,
        -1.227641937791400760e-03, -2.220272816496955248e-03,
        -3.367782937337345916e-03, -4.609541769768488520e-03,
        -5.817430651698407215e-03, -6.814858343399680246e-03,
        -7.436213195116287387e-03, -7.587109902006702704e-03,
        -7.200324960286656750e-03, -6.314989602668651114e-03,
        -4.979662030011092720e-03, -3.349729104311265863e-03,
        -1.553014583611846914e-03, 2.415610721342018779e-04,
        1.820558489975292876e-03,  2.996949899836503541e-03,
        3.700291725228286362e-03,  3.827781750541458996e-03,
        3.341544868580517084e-03,  2.344766943499818936e-03,
        9.203393747475222377e-04,  -7.763225877672553024e-04,
        -2.546091768166472164e-03, -4.294764333406424391e-03,
        -5.777007456536437796e-03, -6.898748945191612164e-03,
        -7.568700539535844207e-03, -7.806377967305393531e-03,
        -7.642844542653941947e-03, -7.136760119963440029e-03,
        -6.340034312758555633e-03, -5.403983311232603282e-03,
        -4.421655618672344287e-03, -3.417405869747909639e-03,
        -2.457291958371846628e-03, -1.501549311469918676e-03,
        -5.707375745707020864e-04, 4.349192935948340195e-04,
        1.517758103847093596e-03,  2.700919520453975899e-03,
        3.931261526647176520e-03,  5.198096050519077777e-03,
        6.415759520663023158e-03,  7.470689851435514950e-03,
        8.230644149950359922e-03,  8.624320666318107884e-03,
        8.578715852388407032e-03,  8.108275215610574391e-03,
        7.192614238929032761e-03,  5.911850021359226631e-03,
        4.371864231275241625e-03,  2.749862201929702613e-03,
        1.229532962405456304e-03,  -1.415891417577625269e-04,
        -1.224322492648278853e-03, -1.977395589836965575e-03,
        -2.407403093574349458e-03, -2.566899791411872325e-03,
        -2.558448880981809264e-03, -2.477534498022228478e-03,
        -2.385655059523906617e-03, -2.374545240316559228e-03,
        -2.446596368767150163e-03, -2.576774253539979801e-03,
        -2.751483207488895737e-03, -2.838220602173403828e-03,
        -2.844472165904681719e-03, -2.677174452520025404e-03,
        -2.353938037452000132e-03, -1.840169580234161072e-03,
        -1.161394062360157024e-03, -4.357579954120387612e-04,
        2.171847300271854172e-04,  7.422018407733878189e-04,
        1.104598162401328968e-03,  1.207608362999634680e-03,
        1.138408480101861953e-03,  8.863257007954609230e-04,
        5.749055933775646526e-04,  2.669230152736823489e-04,
        8.123382992731553709e-05,  4.944437589807318287e-05,
        2.110373236904299207e-04,  5.745257638461137399e-04,
        1.114539357441021120e-03,  1.787779391578993814e-03,
        2.407554326650369197e-03,  2.945665121051682656e-03,
        3.365041849547196085e-03,  3.598438339590819685e-03,
        3.637020882083355933e-03,  3.528120538126048970e-03,
        3.318937293762493600e-03,  3.012950673860773088e-03,
        2.698916210556152797e-03,  2.362248981737688058e-03,
        2.121842139368454797e-03,  1.875607405825379216e-03,
        1.742156140604896779e-03,  1.622429112784042841e-03,
        1.484303271324621375e-03,  1.294783106323162385e-03,
        9.933335755657282141e-04,  5.866932741458507706e-04,
        4.432710147091021733e-05,  -6.805262798457567932e-04,
        -1.488172616665982446e-03, -2.359249420740982747e-03,
        -3.090502243255376525e-03, -3.512106788673871084e-03,
        -3.304606236442738949e-03, -2.140642239254194907e-03,
        4.477425097436063713e-04,  4.912958588537250440e-03,
        1.156407716726623978e-02,  2.046225776888834988e-02,
        3.130573841185904649e-02,  4.333282243407067524e-02,
        5.526730340798053503e-02,  6.543598368715899949e-02,
        7.205029428138375047e-02,  7.339321898423190049e-02,
        6.810397996382364094e-02,  5.547098022582643007e-02,
        3.564265521240432555e-02,  9.610467248874961579e-03,
        -2.078922864809406440e-02, -5.299610019274976491e-02,
        -8.415741685447740428e-02, -1.114747997724667150e-01,
        -1.324068288002094307e-01, -1.451717484221316246e-01,
        -1.489018373214323621e-01, -1.436938127074700866e-01,
        -1.305373479607084830e-01, -1.111818898730845134e-01,
        -8.789558950123060876e-02, -6.307125226333176737e-02,
        -3.904313390031250580e-02, -1.764312129624793016e-02,
        -1.768643231180081995e-04, 1.275502844553794840e-02,
        2.112704210213377926e-02,  2.546751098745806893e-02,
        2.668264192068773091e-02,  2.579796767910061059e-02,
        2.384974582766219431e-02,  2.170165964131975586e-02,
        2.007108386123923269e-02,  1.931841251775536644e-02,
        1.959553808585058968e-02,  2.080169568208907124e-02,
        2.267750254026229886e-02,  2.490919459753148171e-02,
        2.715458511305311046e-02,  2.913712837069788383e-02,
        3.073356501049032824e-02,  3.187241527842984978e-02,
        3.265482469988944397e-02,  3.316909795109985321e-02,
        3.363086394155104242e-02,  3.418035405579292890e-02,
        3.500646551318613992e-02,  3.615870100245689722e-02,
        3.769702411546481391e-02,  3.944442846096053029e-02,
        4.126119212746123116e-02,  4.294106041063314172e-02,
        4.429373113220382713e-02,  4.506674106404553320e-02,
        4.513483977959924232e-02,  4.434659957272920477e-02,
        4.279706609101327497e-02,  4.047718133794842715e-02,
        3.760406453158639944e-02,  3.432193478844292739e-02,
        3.088581810891382132e-02,  2.750793241685902182e-02,
        2.431009586460869745e-02,  2.144768902303988839e-02,
        1.902222006711043814e-02,  1.696859126798110420e-02,
        1.532054048850703225e-02,  1.389492381282379611e-02,
        1.258525767101377306e-02,  1.122869672425629017e-02,
        9.765869775168320097e-03,  8.091243737409664280e-03,
        6.111485375699135698e-03,  3.815492870133588635e-03,
        1.281168647316901317e-03,  -1.439220580901167832e-03,
        -4.307666629462179354e-03, -7.234078114227882918e-03,
        -1.011000036453963725e-02, -1.291329846247012310e-02,
        -1.560209346772041018e-02, -1.820638579060120457e-02,
        -2.073479133674616462e-02, -2.320877213376584119e-02,
        -2.572027736488778638e-02, -2.823617208633888431e-02,
        -3.086241401294769968e-02, -3.356340615443018788e-02,
        -3.629042778457809010e-02, -3.888808642440692853e-02,
        -4.124087190423512267e-02, -4.314497191305534385e-02,
        -4.445319773251394518e-02, -4.505156369431821684e-02,
        -4.475946410575018519e-02, -4.363515042145981598e-02,
        -4.164390219288387901e-02, -3.891802326195446804e-02,
        -3.568139440852233224e-02, -3.210360031996939423e-02,
        -2.843024429562453562e-02, -2.489783101377075758e-02,
        -2.166742206441211321e-02, -1.887789710045447225e-02,
        -1.657929385032422959e-02, -1.480656162699258045e-02,
        -1.344715351218248554e-02, -1.245796566804198771e-02,
        -1.172350937919002488e-02, -1.121173739357218466e-02,
        -1.083364569666616338e-02, -1.056781011752230999e-02,
        -1.040894864859570265e-02, -1.045716554870826376e-02,
        -1.069454925987985833e-02, -1.110770196368574513e-02,
        -1.169787043400973350e-02, -1.237238574358014327e-02,
        -1.300682047042306388e-02, -1.342248666297130195e-02,
        -1.346595732570065457e-02, -1.295166448565293962e-02,
        -1.182348386293650792e-02, -1.003872547367998767e-02,
        -7.634190289069536436e-03, -4.819045545012305351e-03,
        -1.694979215970168502e-03, 1.530665618963632365e-03,
        4.651414331209085748e-03,  7.529008660832720370e-03,
        1.001821612534582739e-02,  1.207943812636109483e-02,
        1.368519982628378130e-02,  1.487869201332618109e-02,
        1.578776689420451768e-02,  1.644191373221411123e-02,
        1.695990678693924322e-02,  1.744136151958234141e-02,
        1.796344462998204425e-02,  1.844785668000494677e-02,
        1.892691700479884331e-02,  1.934487397579335438e-02,
        1.963299038652705636e-02,  1.970651635943145713e-02,
        1.951588160757100274e-02,  1.904620004781604897e-02,
        1.825678232812177545e-02,  1.715819533094918234e-02,
        1.583348007711939814e-02,  1.437802178900759488e-02,
        1.286337005473303999e-02,  1.142254767824970284e-02,
        1.017917769422364317e-02,  9.224741488401307088e-03,
        8.648463201250934651e-03,  8.469640288364039746e-03,
        8.725027165272968704e-03,  9.364957944369181955e-03,
        1.031609987980112997e-02,  1.150047249963357006e-02,
        1.282629085724397086e-02,  1.409600186062713936e-02,
        1.511851622255340130e-02,  1.581625629446247600e-02,
        1.603201414006034942e-02,  1.574290262365915516e-02,
        1.482820475529459010e-02,  1.338066606982587017e-02,
        1.144099951646790694e-02,  9.202950854521172244e-03,
        6.877368655056218492e-03,  4.627881872268368134e-03,
        2.654068749594490524e-03,  1.172062874541005559e-03,
        2.713749871313545837e-04,  1.072963181579849208e-06,
        3.457727430427621888e-04,  1.163117234138649628e-03,
        2.263922295209670320e-03,  3.454480607908114263e-03,
        4.538157968937537104e-03,  5.338645579397176440e-03,
        5.786080166015296865e-03,  5.746092222755115955e-03,
        5.309431701946002065e-03,  4.585235637819767493e-03,
        3.659278822002064689e-03,  2.674631471772625970e-03,
        1.830444354664042790e-03,  1.188050863791830988e-03,
        7.853180427700940172e-04,  6.326415827465659307e-04,
        6.554789639883571578e-04,  7.001770889146975057e-04,
        6.647587534233799934e-04,  4.338386317835994242e-04,
        -9.520595781341113849e-05, -9.485538296468783073e-04,
        -2.121691117966754622e-03, -3.539198058339327203e-03,
        -5.038837176740283003e-03, -6.481080527229465989e-03,
        -7.701187348731851519e-03, -8.472072654490915219e-03,
        -8.737094084600429897e-03, -8.405849168069069038e-03,
        -7.574825768153999918e-03, -6.309288944670942735e-03,
        -4.777977510943207107e-03, -3.174103732210608285e-03,
        -1.793339911798157941e-03, -7.711531427764772807e-04,
        -3.732076516602798487e-04, -6.614456433719257018e-04,
        -1.705933749601289432e-03, -3.378984781460494212e-03,
        -5.503854872993887981e-03, -7.785613713160920331e-03,
        -9.948357854179855006e-03, -1.174324478273404589e-02,
        -1.289941544458985193e-02, -1.327723564249786228e-02,
        -1.278411358079332809e-02, -1.143001342286383698e-02,
        -9.358269919708759460e-03, -6.806029196535597590e-03,
        -4.029816620083220315e-03, -1.303877371602895534e-03,
        1.058969984562241947e-03,  2.846672743526366088e-03,
        3.945655617564419911e-03,  4.247695609555023760e-03,
        3.800341725431445432e-03,  2.682512634401868889e-03,
        1.166032420817603426e-03,  -6.095372260351582033e-04,
        -2.393648553037880638e-03, -4.008008781141099257e-03,
        -5.297508853403846973e-03, -6.148983467793849023e-03,
        -6.560876772487310438e-03, -6.548317781138575377e-03,
        -6.128869255566734772e-03, -5.469141366031418129e-03,
        -4.670940750958064124e-03, -3.893589151446425530e-03,
        -3.259662976551329361e-03, -2.890835101463240173e-03,
        -2.763180569415941111e-03, -2.923347927118011454e-03,
};

#define LMS_STEP 1

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
        return (sample + 1) * INT16_MAX;
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

                error_buffer[r] = e;

                circ_fifo_write(&x_fifo, normalize(x));

                uint16_t y = to_u16(dsp_fir(&x_fifo, primary_w, PRIMARY_ORDER, 0));
                for (int chn = 0; chn < channels; chn++) {
                        for (int i = 0; i < bps; i++) {
                                if (chn == 0) {
                                        *(samples[chn] + i) = (y >> i * 8) & 0xff;
                                }
                        }

                        samples[chn] += steps[chn];
                }

                const float fx =
                        dsp_fir(&x_fifo, secondary_w, SECONDARY_ORDER, 0);
                circ_fifo_write(&fx_fifo, fx);
        }

        for (int i = 0; i < r; i++) {
                dsp_lms(primary_w,
                        normalize(error_buffer[i]),
                        LMS_STEP,
                        &fx_fifo,
                        PRIMARY_ORDER,
                        0);
        }
}

void ident(char* buffer, int r)
{
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
                if (writebuf(phandle, buffer, latency, &frames_out) < 0) {
                        fprintf(stderr, "write error\n");
                        break;
                }
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

        file_helpers_write_buf("left.txt", '\n', left_buffer, cnt, false);
        file_helpers_write_buf("right.txt", '\n', right_buffer, cnt, false);
        file_helpers_write_buf("noise.txt", '\n', noise_buffer, n_cnt, false);

        return 0;
}
