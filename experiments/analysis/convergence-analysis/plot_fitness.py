import seaborn as sns
import matplotlib.pyplot as plt
import matplotlib
from glob import glob
import pandas as pd
import sys
import cmcrameri.cm as cmc

dfs = []

if sys.argv[1] == "LOP":
    for path in glob("results/gs_*.csv"):
        df = pd.read_csv(path)
        df["algorithm"] = "GS"
        df["evaluations"] = df["iteration"].apply(lambda x: 100*x)

        dfs.append(df[["evaluations", "best fitness", "algorithm"]])

    for path in glob("results/pleda_*.csv"):
        df = pd.read_csv(path)
        df["algorithm"] = "PL-EDA"
        df["evaluations"] = df["iteration"].apply(lambda x: 500*x)
        dfs.append(df[["evaluations", "best fitness", "algorithm"]])

    for path in glob("results/convergency_lop_*.csv"):
        df = pd.read_csv(path)
        df["algorithm"] = "NNCO"
        df["evaluations"] = df["iteration"].apply(lambda x: 32*64*x)
        dfs.append(df[["evaluations", "best fitness", "algorithm"]])

    hue_order = ["NNCO", "GS", "PL-EDA"]
    palette =  [cmc.navia(1/6), cmc.navia(4/6), cmc.navia(5/6)]
    markers = ["o", "P", "D"]

else:
    for path in glob("results/umda_*.csv"):
        df = pd.read_csv(path)
        df["algorithm"] = "UMDA"
        df["evaluations"] = df["iteration"].apply(lambda x: 50*100*x)
        dfs.append(df[["evaluations", "best fitness", "algorithm"]])

    for path in glob("results/rk-eda_*.csv"):
        df = pd.read_csv(path)
        df["algorithm"] = "RK-EDA"
        df["evaluations"] = df["iteration"].apply(lambda x: 50*100*x)
        dfs.append(df[["evaluations", "best fitness", "algorithm"]])

    for path in glob("results/convergency_pfsp_*.csv"):
        df = pd.read_csv(path)
        df["algorithm"] = "NNCO"
        df["evaluations"] = df["iteration"].apply(lambda x: 32*64*x)
        dfs.append(df[["evaluations", "best fitness", "algorithm"]])

    hue_order = ["NNCO", "UMDA", "RK-EDA"]
    palette =  [cmc.navia(1/6), cmc.navia(2/6), cmc.navia(3/6)]
    markers = ["o", "X", "s"]

df = pd.concat(dfs)

if sys.argv[1] == "LOP":
    df = df[df["evaluations"] <= 0.5e6]

LEGEND = True

matplotlib.rcParams.update({'font.size': 22}) # increase font size
plt.figure(figsize=(9, 7))

ax = plt.gca()
ax.locator_params(nbins=4)

ax.ticklabel_format(style="sci", axis="both", scilimits=(0,0))
g = sns.lineplot(data=df, x="evaluations",
                 y="best fitness", errorbar="sd",
                 hue="algorithm", style="algorithm",
                 palette=palette, linewidth=3,
                 markersize=12, hue_order=hue_order,
                 legend=LEGEND, markers=False)

sns.despine() # remove top and right spines
if LEGEND:
    box = ax.get_position()
    ## Legend on the bottom
    ax.set_position([box.x0, box.y0 + box.height * 0.1,
                     box.width, box.height * 0.9])
    # Put a legend below current axis
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.12), ncol=5, frameon=False)

plt.xlabel("Number of evaluations")
plt.ylabel("Objective value")
plt.savefig(f"fitness_curve_{sys.argv[1]}.pdf")
plt.show()
