#!/usr/bin/python3

import numpy as np
import scipy.signal as scisig
import matplotlib.pyplot as plt

from fir import FIR
from loguru import logger

def normalize(data):
    data_min = 0
    data_max = 65535

    return (data - data_min) / (data_max - data_min)

if __name__ == "__main__":
    SAMPLING_HZ = 8000

    x = np.genfromtxt("noise.txt", delimiter='\n')
    x_mean = np.mean(x)
    x_var = np.var(x)
    x = x - x_mean
    rxx = np.correlate(x, x, 'full')[len(x) - 1 : ] 


    y = np.genfromtxt("left.txt", delimiter='\n')
    y_mean = np.mean(y)
    y_var = np.var(y)
    y = y - y_mean
    ryy = np.correlate(y, y, 'full')[len(x) - 1 : ] 
    
    c = np.correlate(y, x, 'full')[len(x) - 1 : ]
    gk = c / np.max(rxx)
    gk = gk[ : 400]

    logger.info("Estimated coeffs: {}", gk)
    np.savetxt("sec_coeffs.csv", gk, delimiter='\n')

    plt.figure(1)
    plt.plot(gk)
    plt.grid()
    plt.show()
