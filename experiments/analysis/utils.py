import pandas as pd

common_fields = [
    "algorithm",
    "instance",
    "problem size",
    "problem",
    "best fitness",
    "elapsed time in secs",
]


def get_all_instance_sizes(paths_to_best_knowns="best_knowns.csv"):
    result = {}
    df = pd.read_csv(paths_to_best_knowns)
    for instance in df["instance"].unique():
        result[instance] = df[df["instance"] == instance]["size"].unique()[0]
    return result

instance_sizes = get_all_instance_sizes()

def load_nnco_data(path="results/results_nnco.csv"):
    df = pd.read_csv(path)
    df["algorithm"] = "NNCO"
    df["problem"] = df["problem"].str.upper()
    df["instance"] = df["instance"].str.replace(".fsp", "")
    return df

def load_umda_data(path="results/results_umda.csv"):
    df = pd.read_csv(path)
    df["algorithm"] = "UMDA"
    df = df.rename(columns={"min": "best fitness"})
    df["instance"] = df["instance"].str.replace(".fsp", "")
    df["problem size"] = df["instance"].apply(lambda x: instance_sizes[x])
    df = df[df["problem"] == "PFSP"]
    return df

def load_rkeda_data(path="results/results_rkeda.csv"):
    df = pd.read_csv(path)
    df = df[df.instance.str.contains("tai")]
    df["algorithm"] = "RK-EDA"
    df = df.rename(columns={"instance size": "problem size"})
    df["problem"] = "PFSP"
    return df

def load_pleda_data(path="results/results_pleda.csv"):
    df = pd.read_csv(path, sep=";")
    df["algorithm"] = "PL-EDA"
    df["instance"] = df["instance"].apply(lambda x: x.split("/")[-1])
    df["problem size"] = df["instance"].apply(lambda x: instance_sizes[x])
    df["problem"] = "LOP"
    return df

def load_gs_data(path="results/results_gs.csv"):
    df = pd.read_csv(path)
    df["instance"] = df["instance"].apply(lambda x: x.split("/")[-1])
    df["problem size"] = df["instance"].apply(lambda x: instance_sizes[x])
    df["problem"] = "LOP"
    return df

def compute_gaps(df, problem):
    bk = pd.read_csv("best_knowns.csv")

    get_bk = lambda x: float(bk[bk["instance"] == x]["result"].iloc[0])
    df["best known"] = df["instance"].apply(lambda x: get_bk(x))

    if problem == "LOP":
        df["gap"] =  (df["best known"] - df["best fitness"]) / df["best known"]
        # df["gap"] = 1 - (df["best fitness"] / df["best known"])
    else:
        df["gap"] = (df["best fitness"] - df["best known"])/ df["best known"]
        # df["gap"] =  1 - (df["best known"] / df["best fitness"])

    df["gap"] *= 100

    return df
