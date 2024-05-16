import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
import seaborn as sns
import glob
import sys
import cmcrameri.cm as cmc

problem = sys.argv[1]

dfs = []

for path in glob.glob(f"results_{problem}/*.csv"):
    dfs.append(pd.read_csv(path))

df = pd.concat(dfs)

color_a = cmc.lipari(0.7)
color_b = cmc.lipari(0.1)

print(df.beta.unique())

df["iteration"] *= 32*64

plt.figure(figsize=(10, 8))
matplotlib.rcParams.update({'font.size': 22}) # increase font size

sns.lineplot(data=df, x="iteration", y="p mode", hue="beta", palette=[color_a, color_b])
plt.gca().spines[['right', 'top']].set_visible(False)
plt.ylabel("Log-probability of the mode")
plt.xlabel("Number of evaluations")
from matplotlib.lines import Line2D
custom_lines = [Line2D([0], [0], color=color_a, lw=3),
                Line2D([0], [0], color=color_b, lw=3)]
ax = plt.gca()
box = ax.get_position()
## Legend on the bottom
ax.set_position([box.x0, box.y0 + box.height * 0.1,
                 box.width, box.height * 0.9])
# Put a legend below current axis
ax.legend(custom_lines, ["Original", "Penalized"], loc='upper center', bbox_to_anchor=(0.5, -0.13), ncol=2, frameon=False)
ax.ticklabel_format(style='sci', axis='both', scilimits=(0,0))
plt.savefig(f"prob_mode_{problem}.png")
plt.show()



plt.figure(figsize=(10, 7))
matplotlib.rcParams.update({'font.size': 22}) # increase font size

if problem == "lop":
    df = df[df["best fitness"] >= 2.1e5] # for better visualization of the convergence

sns.lineplot(data=df, x="iteration", y="best fitness", hue="beta", palette=[color_a, color_b])
plt.gca().spines[['right', 'top']].set_visible(False)
plt.ylabel("Objective value")
plt.xlabel("Number of evaluations")

from matplotlib.lines import Line2D
custom_lines = [Line2D([0], [0], color=color_a, lw=3),
                Line2D([0], [0], color=color_b, lw=3)]
ax = plt.gca()
box = ax.get_position()
## Legend on the bottom
ax.set_position([box.x0, box.y0 + box.height * 0.1,
                 box.width, box.height * 0.9])
# Put a legend below current axis
ax.legend(custom_lines, ["Original", "Penalized"], loc='upper center', bbox_to_anchor=(0.5, -0.13), ncol=2, frameon=False)

plt.gca().ticklabel_format(style='sci', axis='both', scilimits=(0,0))

plt.savefig(f"performance_{problem}.png")
plt.show()
