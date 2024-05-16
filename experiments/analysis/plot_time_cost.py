import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import seaborn as sns
# import opinionated
# plt.style.use("opinionated_rc")
import utils
import cmcrameri # scientific colormaps
import sys
import cmcrameri.cm as cmc


LEGEND = True

if len(sys.argv) < 2:
    print("Missing CLI argument with the problem type: PFSP or LOP")
    quit(1)
problem = sys.argv[1]

if problem == "PFSP":
    nnco_data = utils.load_nnco_data()[utils.common_fields]
    umda_data = utils.load_umda_data()[utils.common_fields]
    rkeda_data = utils.load_rkeda_data()[utils.common_fields]
    data = pd.concat([nnco_data, umda_data, rkeda_data])
    hue_order = ["NNCO", "UMDA", "RK-EDA"]
    palette =  [cmc.navia(1/6), cmc.navia(2/6), cmc.navia(3/6)]
    markers = ["o", "X", "s"]
else:
    nnco_data = utils.load_nnco_data()[utils.common_fields]
    pleda_data = utils.load_pleda_data()
    gs_data = utils.load_gs_data()
    data = pd.concat([nnco_data, pleda_data, gs_data])
    hue_order = ["NNCO", "GS", "PL-EDA"]
    palette =  [cmc.navia(1/6), cmc.navia(4/6), cmc.navia(5/6)]
    markers = ["o", "P", "D"]

data = data[data["problem"] == problem]

matplotlib.rcParams.update({'font.size': 22}) # increase font size

plt.figure(figsize=(9, 8))

sns.lineplot(data=data, x="problem size", y="elapsed time in secs",
             hue="algorithm", style="algorithm", palette=palette,
             linewidth=3, markersize=12, hue_order=hue_order, legend=LEGEND, markers=markers)

sns.despine() # remove top and right spines
ax = plt.gca()
ax.set_yscale('log')

if LEGEND:
    box = ax.get_position()
    ## Legend on the bottom
    ax.set_position([box.x0, box.y0 + box.height * 0.1,
                     box.width, box.height * 0.9])
    # Put a legend below current axis
    ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.12), ncol=5, frameon=False)

plt.xlabel("Problem size")
plt.ylabel("Elapsed time (sec.)")
# plt.title(problem)

# plt.tight_layout()
plt.savefig(f"time_cost_{problem}.pdf")
plt.show()
