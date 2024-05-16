import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import ast
from cycler import cycler
import matplotlib
import cmcrameri.cm as cmc


cmap = cmc.bamako


def plot_umd(path, alpha=0.3, evals_per_point=1):
    trfm = lambda x: np.array(ast.literal_eval(x))
    df = pd.read_csv(path, quotechar="\"", converters={1: trfm})

    values = np.vstack(df["distribution"].values)

    # values[ values == 0 ] = 1e-5 # to avoid numerical errors

    n = values.shape[1]
    values = values.reshape((-1, n-1, n))

    values = values / values.sum(axis=2, keepdims=True)  # numbers to probabilities

    a = values[1:]
    b = values[:-1]
    diff = np.abs(a - b)

    diff = diff.reshape((-1, n*(n-1)))

    x = np.arange(diff.shape[0], dtype=np.float64)
    x *= evals_per_point

    for i in range(n-1):
        c = cmap(i/(n-1))
        plt.plot(x, diff[:, (i*n):(i*n+n)], alpha=alpha, color=c) # color="gray")


def plot_pl(path, alpha=0.3, evals_per_point=1):
    trfm = lambda x: np.array(ast.literal_eval(x))
    df = pd.read_csv(path, quotechar="\"", converters={1: trfm})

    values = np.vstack(df["distribution"].values)
    values[values > 1] = 1

    a = values[1:]
    b = values[:-1]
    diff = np.abs(a - b)
    # diff = values

    x = np.arange(diff.shape[0], dtype=np.float64)
    x *= evals_per_point

    for i in range(diff.shape[1]):
        c = cmap(i/diff.shape[1])
        plt.plot(x, diff[:, i], alpha=alpha, color=c)


def plot_gauss(path, alpha=0.3, evals_per_point=1):
    trfm = lambda x: np.array(ast.literal_eval(x))
    df = pd.read_csv(path, quotechar="\"", converters={1: trfm})

    values = np.vstack(df["distribution"].values)

    a = values[1:]
    b = values[:-1]
    # diff = np.abs(a - b)
    diff = a - b
    # diff = values

    x = np.arange(1, diff.shape[0]+1, dtype=np.float64)
    x *= evals_per_point

    n = diff.shape[1]
    for i in range(n):
        print(i)
        c = cmap(i/n)
        plt.plot(x, diff[:, i], alpha=alpha, color=c)


if __name__ == "__main__":
    import sys
    from glob import glob

    algorithm = sys.argv[1]

    plt.figure(figsize=(10, 7))
    matplotlib.rcParams.update({'font.size': 22}) # increase font size

    if algorithm == "gs": # N-bee75eec
        # N-bee75eec
        epi = 100
        for path in glob("results/gs_*.csv"):
            plot_pl(path, evals_per_point=epi*10)

    elif algorithm == "pleda": # N-bee75eec
        epi = 50*10
        for path in glob("results/pleda_*.csv"):
            plot_pl(path, evals_per_point=epi*10)

    elif algorithm == "nnco-lop": # N-bee75eec
        epi = 32*64
        for path in glob("results/convergency_lop_*.csv"):
            plot_pl(path, evals_per_point=epi*10)

    elif algorithm == "rk-eda": # tai50_5_8.fsp
        epi = 50*100
        for path in glob("results/rk-eda_*.csv"):
            plot_gauss(path, evals_per_point=epi*10)
            # break

    elif algorithm == "umda": # tai50_5_8.fsp
        epi = 100*50
        for path in glob("results/umda_*.csv"):
            plot_umd(path, evals_per_point=epi)

    elif algorithm == "nnco-pfsp": # tai50_5_8.fsp
        epi = 32*64 # evals per iter
        n = 50
        for path in glob("results/convergency_pfsp_*.csv"):
            plot_umd(path, evals_per_point=epi*10)

    plt.ylabel("Absolute difference")
    plt.xlabel("Number of evaluations")
    plt.gca().spines[['right', 'top']].set_visible(False)
    # plt.colorbar(matplotlib.cm.ScalarMappable(norm=matplotlib.colors.Normalize(0, 50), cmap=cmap), ax=plt.gca())
    plt.tight_layout()
    plt.savefig(f"distrib_{algorithm}.png")
    plt.show()
