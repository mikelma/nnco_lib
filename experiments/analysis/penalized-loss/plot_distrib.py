import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import ast
from cycler import cycler
import matplotlib
import cmcrameri.cm as cmc

color_a = cmc.lipari(0.7)
color_b = cmc.lipari(0.1)

def plot_umd(path, alpha=0.5, evals_per_point=1):
    trfm = lambda x: np.array(ast.literal_eval(x))
    df = pd.read_csv(path, quotechar="\"", converters={1: trfm})

    values = np.vstack(df["distribution"].values)

    n = values.shape[1]
    values = values.reshape((-1, n-1, n))

    values = values / values.sum(axis=2, keepdims=True)  # numbers to probabilities

    a = values[1:]
    b = values[:-1]
    diff = np.abs(a - b)

    diff = diff.reshape((-1, n*(n-1)))

    x = np.arange(diff.shape[0], dtype=np.float64)
    x *= evals_per_point

    if df["beta"].unique() == 0:
        print("Beta 0")
        c = color_a
    else:
        print("Beta >0")
        c = color_b
    plt.plot(x, diff, alpha=alpha, color=c) # color="gray")


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

    if df["beta"].unique() == 0:
        print("Beta 0")
        c = color_a
    else:
        print("Beta >0")
        c = color_b

    plt.plot(x, diff, alpha=alpha, color=c)


if __name__ == "__main__":
    import sys
    from glob import glob

    problem = sys.argv[1]

    plt.figure(figsize=(10, 8))
    matplotlib.rcParams.update({'font.size': 22}) # increase font size

    epi = 32*64
    for path in glob(f"results_{problem}/*.csv"):
        if problem == "lop": # N-bee75eec
            plot_pl(path, evals_per_point=epi*10)
        else:
            plot_umd(path, evals_per_point=epi*10)

    plt.ylabel("Absolute difference")
    plt.xlabel("Number of evaluations")
    plt.gca().spines[['right', 'top']].set_visible(False)

    from matplotlib.lines import Line2D
    ax = plt.gca()
    custom_lines = [Line2D([0], [0], color=color_a, lw=3),
                    Line2D([0], [0], color=color_b, lw=3)]
    box = ax.get_position()
    ## Legend on the bottom
    ax.set_position([box.x0, box.y0 + box.height * 0.1,
                     box.width, box.height * 0.9])
    # Put a legend below current axis
    ax.legend(custom_lines, ["Original", "Penalized"], loc='upper center', bbox_to_anchor=(0.5, -0.13), ncol=2, frameon=False)
    plt.savefig(f"distrib_betas_{problem}.png")
    plt.show()
