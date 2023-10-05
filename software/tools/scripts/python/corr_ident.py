#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

from loguru import logger

def normalize(data):
    data_min = 0
    data_max = 65535

    return (data - data_min) / (data_max - data_min)

if __name__ == "__main__":
    font = {'size': 16}
    plt.rc('font', **font)

    SAMPLING_HZ = 8000

    x = np.genfromtxt("noise.txt", delimiter='\n')
    x_mean = np.mean(x)
    x_var = np.var(x)
    x = x - x_mean
    rxx = np.correlate(x, x, 'full')[len(x) - 1 : ] 

    y = np.genfromtxt("error.txt", delimiter='\n')
    y_mean = np.mean(y)
    y_var = np.var(y)
    y = y - y_mean
    ryy = np.correlate(y, y, 'full')[len(x) - 1 : ] 
    
    c = np.correlate(y, x, 'full')[len(x) - 1 : ]
    gk = c / np.max(rxx)
    gk = gk[0 : 60]

    logger.info("Estimated coeffs: {}", gk)
    np.savetxt("sec_coeffs.csv", gk, delimiter=',')

    plt.figure(1)
    plt.plot(gk, linewidth=2)
    plt.title("Odpowiedź impulsowa $S(z^{-1})$ - metoda korelacyjna")
    plt.xlabel(r"i")
    plt.ylabel(r"$\dfrac{R_{xy}}{\delta^2}$")
    plt.legend()
    plt.grid()
    plt.show()
