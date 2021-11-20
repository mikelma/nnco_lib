import argparse
import uuid
import csv
import time
from collections import OrderedDict

import torch
import torch.nn as nn
from torch.optim import Adam

from pypermu import problems
from pypermu import utils as permutils

import nnco
from nnco import utility

# parse command line arguments
parser = argparse.ArgumentParser()

parser.add_argument('-i', '--instance', metavar='PATH', type=str, nargs=1,
                    required=True, help='Path to the instance file')
parser.add_argument('-p', '--problem', type=str, nargs=1, required=True, 
                    choices=['pfsp', 'lop'], help='Combinatorial problem')
parser.add_argument('-b', '--batch-size', metavar='N', type=int, nargs=1,
                    required=True, help='Batch size')
parser.add_argument('-d', '--hidden-dim', metavar='N', type=int, nargs=1,
                    required=True, help='Hidden layer dimension')
parser.add_argument('-l', '--prehead-layers', metavar='N', type=int, nargs=1,
                    required=True, help='Number of pre-head layers')
parser.add_argument('-n', '--noise-len', metavar='N', type=int, nargs=1,
                    required=True, help='Length of the noise vector')
parser.add_argument('-o', '--output', metavar='PATH', type=str, nargs=1,
                    required=False, help='Path to write the CSV summary of the experiment to')

args = parser.parse_args()

# initialize combinatorial problem
if args.problem[0] == 'lop':
    problem = problems.lop.Lop(args.instance[0])

elif args.problem[0] == 'pfsp':
    problem = problems.pfsp.Pfsp(args.instance[0])


# ------------- Configuration ------------- #
config = {
    'instance': args.instance[0],
    'problem': args.problem[0],
    'problem size': problem.size,
    'max evaluations': 1000*problem.size**2,

    # hyperparameters
    'batch size': args.batch_size[0],
    'noise length': args.noise_len[0],
    'hidden dim': args.hidden_dim[0],
    'prehead layers': args.prehead_layers[0],

    'num samples': 64,
    'learning rate': 0.003,
    'utility function': utility.standarized_utility,

} 
config['iterations'] = int(
        config['max evaluations'] / (config['batch size'] * config['num samples'])
)
# ----------------------------------------- #

## build the NN model
# build the prehead modues, that is, the linear layers before 
# the head module
prehead = []
in_dim = config['noise length']
for i in range(config['prehead layers']):
    prehead.append(
        (f'linear_{i}', 
            nn.Linear(in_dim, config['hidden dim']))
    )
    prehead.append((f'relu_{i}', nn.ReLU()))
    in_dim = config['hidden dim']

if config['problem'] == 'lop':
    head_class = nnco.pl.PLHead

elif config['problem'] == 'pfsp':
    head_class = nnco.umd.UMDHead

# build head module
head = head_class(
        input_dim=config['hidden dim'],
        sample_length=config['problem size'],
        num_samples=config['num samples'],
)

model = nn.Sequential(OrderedDict(prehead + [('head', head)]))

## optimizer configuration
optimizer = Adam(model.parameters(), lr=config['learning rate'])

## main loop
if config['problem'] == 'lop':
    compare = torch.max
elif config['problem'] == 'pfsp':
    compare =  torch.min

# fitness of the best solution found so far
best_fitness = None
# start time of the main loop, in order to measure the elapsed time later
start_time = time.time()

for iter in range(config['iterations']):
    x = torch.normal(mean=0, std=1, 
            size=(config['batch size'], config['noise length']))

    samples, logps = model(x)

    if config['problem'] == 'pfsp':
        # convert marina vectors to permutations
        permus = [permutils.transformations.marina2permu(b) for b in samples.cpu().numpy()]
        permus = [permutils.transformations.permu2inverse(batch) for batch in permus]
        fitness = [problem.evaluate(batch) for batch in permus]

    elif config['problem'] == 'lop':
        # move sampled permutations to the cpu in order to be evaluated
        permus = samples.cpu().numpy()

    # evaluate samples
    fitness = [problem.evaluate(batch) for batch in permus]
    fitness = torch.as_tensor(fitness, dtype=torch.float32)

    # compute loss value
    u = config['utility function'](fitness)
    loss = (logps * u).mean()

    if config['problem'] == 'lop':
        loss = -loss # LOP is a maximization problem!

    # optimize the model!
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()


    # ----- logging ----- #
    if best_fitness == None:
        best_fitness = compare(fitness)

    else:
        best_fitness = compare(best_fitness, compare(fitness))

    # print(f'best: {best_fitness.item()}')
    # ------------------- #

config['elapsed time in secs'] = time.time() - start_time
config['best fitness'] = best_fitness.item()
config['utility function'] =  config['utility function'].__name__
config['instance'] =  config['instance'].split('/')[-1]
config['num trainable params'] = sum(p.numel() for p in model.parameters() if p.requires_grad)

if args.output != None:
    fname = args.output[0]
else:
    fname = f'{str(uuid.uuid4())}.csv'

with open(fname, 'w') as f:
    w = csv.DictWriter(f, config.keys())
    w.writeheader()
    w.writerow(config)
