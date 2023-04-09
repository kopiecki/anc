#!/usr/bin/python3

import argparse as arg
import numpy as np
import matplotlib.pyplot as plt

from loguru import logger


def parse_arguments() -> arg.Namespace:
    parser: arg.ArgumentParser = arg.ArgumentParser()
    parser.add_argument("--lms", type=str,
                        default="test/functional/output/lms_w.csv")
    parser.add_argument(
        "--e", type=str, default="test/functional/output/lms_e.csv")
    parser.add_argument("--real", type=str,
                        default="test/functional/input/fir_coeffs_14.csv")
    return parser.parse_args()


def read_lms_results(file_name: str) -> np.array:
    w = np.genfromtxt(file_name, delimiter=',', dtype=float)
    return w[:, np.isfinite(w).all(0)]


def read_real_w(file_name: str) -> np.array:
    return np.genfromtxt(file_name, dtype=float)


def read_e(file_name: str) -> np.array:
    return np.genfromtxt(file_name, dtype=float)


def plot_w(real_w: np.array, w: np.array) -> None:
    w = np.matrix.transpose(w)
    for i, (real_coeff, lms_coeff) in enumerate(zip(real_w, w)):
        plt.plot(lms_coeff, label="lms coeff {}".format(i))
        plt.plot(np.ones(lms_coeff.shape[0]) *
                 real_coeff, label="real coeff {}".format(i))
    plt.grid()
    plt.legend()
    plt.show()


if __name__ == "__main__":
    args = parse_arguments()
    w = read_lms_results(args.lms)
    logger.info("Last LMS esimtate = {}", w[-1])
    real_w = read_real_w(args.real)
    logger.info("Real filter coeffs = {}", real_w)

    e = read_e(args.e)

    plt.figure(1)
    plt.plot(e)

    plt.figure(2)
    plot_w(real_w, w)
