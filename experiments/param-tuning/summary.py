
import pandas as pd
import itertools
import sys
import numpy as np

if len(sys.argv) != 2:
    print("Incorrect number of arguments, expected file name.\n")
    quit(1)

df = pd.read_csv(sys.argv[1])


instances = df["instance"].unique()
for instance in instances:
    t = np.mean(df[df.instance == instance]["elapsed time in secs"])
    print(f"Instance: {instance}, num. of runs:", len(df[df.instance == instance]), f", mean time: {t/60} mins")
