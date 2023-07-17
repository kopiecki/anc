#!/usr/bin/python3

from scipy.fft import fft, fftfreq
import numpy as np
import matplotlib.pyplot as plt
import sys


if __name__ == "__main__":
    T = 1.0 / 10.0

    data = []
    with open(sys.argv[1]) as f:
        for line in f.readlines():
            sample = float(line)
            data.append(sample)

    N = len(data)

    yf = fft(data)
    xf = fftfreq(N, T)[:N//2]

    plt.semilogy(xf[1:N//2], 2.0/N * np.abs(yf[1:N//2]), '-b')
    plt.grid()
    plt.show()
