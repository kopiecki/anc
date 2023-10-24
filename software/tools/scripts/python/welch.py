#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt
from psd_wav import compute_and_plot_psd


def read_file(name):
    with open(name) as f:
        return np.array([x for x in f.readlines()]).astype(float)

if __name__ == "__main__":
    font = {'size': 34}
    plt.rc('font', **font)

    off = read_file("data/meas/real/anc_opc.txt")
    off -= np.mean(off)

    on = read_file("data/meas/real/anc_opc_on.txt")
    on -= np.mean(on)

    compute_and_plot_psd(off, 2000, 2**10, _label="Bez ANC")
    compute_and_plot_psd(on, 2000, 2**10, _label="Z ANC")
    plt.legend()
    plt.grid()
    plt.show()

