#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import ctypes

from psd_wav import compute_and_plot_psd


def u16(x: int):
    return ctypes.c_uint16(x)


class fib(object):
    def __init__(self, seed: int):
        self.__seed = seed

    def __call__(self):
        bit = (self.__seed ^ (self.__seed >> 1) ^ (
            self.__seed >> 3) ^ (self.__seed >> 12)) & 1
        self.__seed = (self.__seed >> 1) | (bit << 15)
        return u16(self.__seed)


class xorshift(object):
    def __init__(self, seed: int):
        self.__seed = seed

    def __call__(self):
        self.__seed ^= self.__seed << 7
        self.__seed ^= self.__seed >> 9
        self.__seed ^= self.__seed << 8
        return u16(self.__seed)


if __name__ == "__main__":
    prng = fib(1)
    x = np.array([])
    for i in range(1024):
        x = np.append(x, prng())

    mean = np.mean(x)
    var = np.var(x)
    x -= mean
    sxx = np.correlate(x, x, 'full')[len(x) - 1:]
    sxx /= np.max(sxx)

    plt.subplot(2, 1, 1)
    compute_and_plot_psd(x, 8e3)

    plt.subplot(2, 1, 2)
    plt.plot(sxx, linewidth=2)
    plt.xlabel(r"$\tau$")
    plt.ylabel(r"$S_{xx}(\tau)$")
    plt.grid()

    plt.show()
