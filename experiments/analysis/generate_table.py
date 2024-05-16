import pandas as pd
import sys
import utils
import numpy as np

if len(sys.argv) < 2:
    print("Missing CLI argument with the problem type: PFSP or LOP")
    quit(1)
problem = sys.argv[1]

if problem == "PFSP":
    nnco_data = utils.load_nnco_data()[utils.common_fields]
    umda_data = utils.load_umda_data()[utils.common_fields]
    rkeda_data = utils.load_rkeda_data()[utils.common_fields]
    data = pd.concat([nnco_data, umda_data, rkeda_data])
    algorithms = ["NNCO", "UMDA", "RK-EDA"]
else:
    nnco_data = utils.load_nnco_data()[utils.common_fields]
    pleda_data = utils.load_pleda_data()
    gs_data = utils.load_gs_data()
    data = pd.concat([nnco_data, pleda_data, gs_data])
    algorithms = ["NNCO", "GS", "PL-EDA"]

data = data[data["problem"] == problem]

## Compute gaps to best knowns
df = utils.compute_gaps(data, problem)

if problem == "PFSP":
    key = lambda s: [int(n) for n in s[3:].split("_")]
else:
    key = lambda s: [utils.instance_sizes[s], s]

instances = sorted(df.instance.unique(), key=key)

def fmt_big(n):
    n = round(n, 2)
    s = "{:.4}".format(float(n))
    # s = s.replace("+", "")

    t = s.split("e")
    if len(t) > 1:
        s = str(round(float(t[0]), 2)) + "e" + t[1]
    return s

for instance in instances:
    alg_times = []
    alg_gaps = []

    for algo in algorithms:
        sel = df[(df.instance == instance) & (df.algorithm == algo)]

        if len(sel) == 0:
            alg_times.append((float("inf"), float("inf")))
            alg_gaps.append((float("inf"), float("inf")))
            continue

        t_mean = np.mean(sel["elapsed time in secs"] / 60)
        t_std = np.std(sel["elapsed time in secs"] / 60)

        gap_mean = np.mean(sel["gap"])
        gap_std = np.std(sel["gap"])

        alg_times.append((t_mean, t_std))
        alg_gaps.append((gap_mean, gap_std))

        problem_size = int(sel["problem size"].unique()[0])
        #print(sel)

    instance = instance.replace('_', '\_')
    row = f"& {instance} & {problem_size} & "

    best_idx = np.argmin([v for v, _ in alg_times])
    for i, (mean, std) in enumerate(alg_times):
        if i == best_idx:
            row += "\\tb{"
        row += f"{fmt_big(mean)} $\pm$ {fmt_big(std)}"
        if i == best_idx:
            row += "}"
        row += " & "

    best_idx = np.argmin([v for v, _ in alg_gaps])
    for i, (mean, std) in enumerate(alg_gaps):
        if i == best_idx:
            row += "\\tb{"
        row += f"{round(mean, 2)} $\pm$ {round(std, 2)}"
        if i == best_idx:
            row += "}"
        row += " & "

    row = row[:-2] + "\\\\"

    row = row.replace("inf", "-")

    print(row) #, alg_gaps)
    # quit()

print("NOTE: Don't copy the last \\\\")
