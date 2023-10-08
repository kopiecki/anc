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
    n = len(x)
    mean = np.mean(x)
    x = x - mean
    sxx = np.correlate(x, x, 'full')[n - 1:]
    sxx /= np.max(sxx)

    plt.plot(sxx, linewidth=4)
    plt.xlabel(r"$\tau$")
    plt.ylabel(r"$S_{xx}(\tau)$")
    plt.xticks([0, (n - 1) / 2, n - 1])
    plt.yticks([0, 0.25, 0.5, 0.75, 1])
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
    for _ in range(4096):
        x = np.append(x, prng())

    plt.subplot(2, 1, 1)
    compute_and_plot_psd(x, 8e3)
    
    plt.subplot(2, 1, 2)
    xcorr(x)

    plt.show()


if __name__ == "__main__":
    font = {'size'   : 34}
    plt.rc('font', **font)

    xor = read_file("xorshift.csv")
    mean = np.mean(xor)
    xor = xor - mean

    plt.subplot(2, 1, 1)
    compute_and_plot_psd(xor, 2, 2**9)
    plt.subplot(2, 1, 2)
    xcorr(xor)
    plt.show()

