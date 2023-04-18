#!/usr/bin/python3

import numpy as np
import scipy.signal as scisig
import matplotlib.pyplot as plt

from fir import FIR
from loguru import logger

if __name__ == "__main__":
    SAMPLING_HZ = 4000
    COEFFS_LEN = 140

    model = FIR(sampling_hz=SAMPLING_HZ, stopband_ripple_db=30,
                cutoff_hz=200, trans_band_width_hz=1000)

    x = np.genfromtxt("input.csv", delimiter='\n')
    y = np.genfromtxt("capture.csv", delimiter='\n')

    x = x[SAMPLING_HZ :]
    y = y[SAMPLING_HZ :]

    rxx = scisig.correlate(x, x)
    ryy = scisig.correlate(y, y)

    c = scisig.correlate(y, x)
    lags = scisig.correlation_lags(len(x), len(y))
    gk = c / np.max(rxx)
    gk = gk[lags]
    gk = gk[: COEFFS_LEN]

    logger.info("Estimated coeffs: {}", gk)
    np.savetxt("sec_coeffs.csv", gk, delimiter='\n')

    plt.figure(1)
    plt.plot(gk)
    plt.grid()
    plt.show()
