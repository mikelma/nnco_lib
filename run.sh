#!/bin/bash

LR=(0.01 0.001 0.0001 0.00001 0.000001)
PREHEADS=(0 1 2 3 4 5 6)

batch_size=32
noise_dim=128
hidden_dim=128

instance="../instances/PFSP/tai20_5_8.fsp"
# instance="../instances/PFSP/tai50_20_8.fsp"
problem="pfsp"

iters=0
for lr in "${LR[@]}"; do
    for preheads in "${PREHEADS[@]}"; do
        python priscilla.py -i ${instance} -p ${problem} -b ${batch_size} -d ${hidden_dim} -l ${preheads} -n ${noise_dim} -lr ${lr} -dev "cuda:0"
        # echo "python priscilla.py -i ${instance} -p ${problem} -b ${batch_size} -d ${hidden_dim} -l ${preheads} -n ${noise_dim} -lr ${lr} -dev cuda:0" 
        iters=$((iters + 1))
        echo "iteration: $iters"
    done
done

