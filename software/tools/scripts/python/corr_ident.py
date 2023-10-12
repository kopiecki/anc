#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

from loguru import logger

def normalize(data):
    data_min = 0
    data_max = 65535

    return (data - data_min) / (data_max - data_min)

def ident(x_name, y_name):
    x = np.genfromtxt(x_name, delimiter='\n')
    x_mean = np.mean(x)
    x = x - x_mean
    rxx = np.correlate(x, x, 'full')[len(x) - 1 : ] 

    y = np.genfromtxt(y_name, delimiter='\n')
    y_mean = np.mean(y)
    y = y - y_mean
    ryy = np.correlate(y, y, 'full')[len(x) - 1 : ] 
    
    c = np.correlate(y, x, 'full')[len(x) - 1 : ]
    gk = c / np.max(rxx)

    logger.info("Estimated coeffs: {}", gk)
    np.savetxt("sec_coeffs.csv", gk, delimiter=',')

    return gk[0 : 300]

if __name__ == "__main__":
    font = {'size': 34}
    plt.rc('font', **font)

    SAMPLING_HZ = 8000

    gk_128 = ident("noise_128.txt", "anc_128.txt")
    gk_256 = ident("noise_256.txt", "anc_256.txt")

    plt.figure(1)
    plt.plot(gk_128, linewidth=2, label=r"$n_f = 64$", color = 'b')
    plt.plot(gk_256, linewidth=2, label=r"$n_f = 128$", color = 'r')
    plt.axvline(64, color = 'b', linestyle = '--', linewidth=2, label=r"$i = 64$") 
    plt.axvline(128, color = 'r', linestyle = '--', linewidth=2, label=r"$i = 128$")
    plt.xlabel(r"i")
    plt.ylabel(r"$\hat{s}(i)$")
    plt.legend()
    plt.grid()
    plt.show()
