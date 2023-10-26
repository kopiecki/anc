#!/usr/bin/python3

import argparse as arg
import numpy as np
import matplotlib.pyplot as plt


def parse_arguments() -> arg.Namespace:
    parser: arg.ArgumentParser = arg.ArgumentParser()
    parser.add_argument("--file", type=str, nargs='*')
    parser.add_argument("--fs", type=float)
    return parser.parse_args()

def read_file(name):
    with open(name) as f:
        return np.array([float(x) - 0.005 for x in f.readlines()]).astype(float)

def plot(buffer, name, sampling_rate) -> None:
    font = {'size': 34}
    plt.rc('font', **font)

    t = np.arange(len(buffer)) / sampling_rate
    plt.plot(t, buffer, label=name, linewidth=2)

    plt.xlabel("t, s")
    plt.xlim([0, 2.0])
    plt.ylabel("e(t)")
    plt.grid()
    plt.show()

if __name__ == "__main__":
    args = parse_arguments()
    data = read_file(args.file[0])

    plot(data, args.file, args.fs)
