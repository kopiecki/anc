#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
import ctypes

from psd_wav import compute_and_plot_psd
from random import seed, randint

def read_file(name):
    with open(name) as f:
        return np.array([int(x) for x in f.readlines()]).astype('uint16')

def xcorr(x):
    mean = np.mean(x)
    x = x - mean
    sxx = np.correlate(x, x, 'full')[len(x) - 1:]
    sxx /= np.max(sxx)

    plt.plot(sxx, linewidth=2)
    plt.xlabel(r"$\tau$")
    plt.ylabel(r"$S_{xx}(\tau)$")
    plt.grid()


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


class rand(object):
    def __init__(self):
        seed(1)

    def __call__(self):
        return u16(randint(0, 32767))

def test():
    prng = rand()

    x = np.array([])
    for _ in range(1024):
        x = np.append(x, prng())

    plt.subplot(2, 1, 1)
    compute_and_plot_psd(x, 8e3)
    plt.subplot(2, 1, 2)
    xcorr(x)

    plt.show()


if __name__ == "__main__":
    r, xor = read_file("randn.csv"), read_file("xorshift.csv")

    plt.subplot(2, 1, 1)
    compute_and_plot_psd(r, 1, 2**8)
    compute_and_plot_psd(xor, 1, 2**8)
    plt.subplot(2, 1, 2)
    xcorr(r)
    xcorr(xor)
    plt.show()

