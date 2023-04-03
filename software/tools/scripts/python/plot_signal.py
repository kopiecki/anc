#!/usr/bin/python3

import argparse as arg
import numpy as np
import matplotlib.pyplot as plt


def parse_arguments() -> arg.Namespace:
    parser: arg.ArgumentParser = arg.ArgumentParser()
    parser.add_argument("--file", type=str)
    parser.add_argument("--fs", type=float)
    return parser.parse_args()


def read_file(file_name: str) -> list[float]:
    data: list[float] = []
    with open(file_name) as file:
        for line in file.readlines():
            sample = float(line)
            data.append(sample)
    return data


def plot(buffer: list[float], sampling_rate: float, name: str) -> None:
    t = np.arange(len(buffer)) / sampling_rate

    plt.plot(t, buffer, label=name)
    plt.legend()
    plt.xlabel("t, s")
    plt.ylabel("y(t)")
    plt.grid()
    plt.show()


if __name__ == "__main__":
    args = parse_arguments()
    buffer: list[float] = read_file(args.file)
    plot(buffer, args.fs, args.file)
