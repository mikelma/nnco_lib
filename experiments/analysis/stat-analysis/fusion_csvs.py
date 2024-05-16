import utils
import pandas as pd
import sys


if len(sys.argv) < 2:
    print("Missing CLI argument with the problem type: PFSP or LOP")
    quit(1)
problem = sys.argv[1]

if problem == "PFSP":
    nnco_data = utils.load_nnco_data()[utils.common_fields]
    umda_data = utils.load_umda_data()[utils.common_fields]
    rkeda_data = utils.load_rkeda_data()[utils.common_fields]
    data = pd.concat([nnco_data, umda_data, rkeda_data])
else:
    nnco_data = utils.load_nnco_data()[utils.common_fields]
    pleda_data = utils.load_pleda_data()
    gs_data = utils.load_gs_data()
    data = pd.concat([nnco_data, pleda_data, gs_data])

data = data[data["problem"] == problem]

data.to_csv(f"result_joined_{problem}.csv")
