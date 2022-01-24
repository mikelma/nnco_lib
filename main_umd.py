import torch
import sys
import torch
from torch.optim import Adam
import torch.nn as nn
from pypermu import problems
from pypermu import utils as permutils
from collections import OrderedDict
from scipy.stats import entropy
import numpy as np

from nnco.umd import UMDHead
from nnco import utility, rho_functions
from log import log_to_csv

BATCH_SIZE    = 32
NUM_SAMPLES   = 64 
LEARNING_RATE = 0.003
NOISE_LEN     = 128
INSTANCE      = sys.argv[1] 

problem = problems.pfsp.Pfsp(INSTANCE)

MAX_EVALS = 1000*problem.size**2
NUM_ITERS = int(MAX_EVALS/(BATCH_SIZE*NUM_SAMPLES))

# NUM_ITERS = 3 # DEBUG!

def entropy_umd(distrib, reduction=None):
    H = []
    for logits in distrib[:-1]:
        logits = torch.exp(logits)
        l = logits / logits.max(1, keepdim=True)[0]
        h = entropy(l.cpu(), axis=1) / np.log(l.size(1))
        H.append(np.mean(h))

    if reduction == 'mean':
        H = np.mean(H)

    return H

model = nn.Sequential(
            torch.nn.Linear(NOISE_LEN, NOISE_LEN),
            torch.nn.ReLU(),
            UMDHead(
                input_dim=NOISE_LEN,
                sample_length=problem.size,
                num_samples=NUM_SAMPLES,
                # rho_function=rho_functions.LogPNormalization()
            ),
        )

optimizer = Adam(model.parameters(), lr=LEARNING_RATE)

log = OrderedDict()
log['iteration'] =  []
log['best fitness'] =  []
log['entropy mean'] =  []
for i in range(problem.size-1):
    log[f'entropy-{i}'] = []

best_fitness = []
for iter in range(NUM_ITERS):
    x = torch.normal(mean=0, std=1, size=(BATCH_SIZE, NOISE_LEN))
    
    samples, logps, distrib = model(x)

    permus = [permutils.transformations.marina2permu(b) for b in samples.cpu().numpy()]
    permus = [permutils.transformations.permu2inverse(batch) for batch in permus]

    fitness = [problem.evaluate(batch) for batch in permus]
    fitness = torch.as_tensor(fitness, dtype=torch.float32)

    u = utility.standarized_utility(fitness)
    loss = (logps * u).mean()

    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    # ---- logging ---- #
    if len(best_fitness) == 0 or fitness.min() < best_fitness[-1]:
        best_fitness.append(fitness.min().item())
    else:
        best_fitness.append(best_fitness[-1])
    
    log['iteration'].append(iter)
    log['best fitness'].append(best_fitness[-1])

    H = entropy_umd(distrib)
    for i, h in enumerate(H):
        log[f'entropy-{i}'].append(h)
    log['entropy mean'].append(np.mean(H))

    # ----------------- #

    print(f'{iter}/{NUM_ITERS} loss: {loss.item()}, mean: {fitness.mean()}, best: {best_fitness[-1]}')

print(f'Best solution found {best_fitness[-1]}')

log['problem'] = ['pfsp']*NUM_ITERS
log['instance'] = [INSTANCE.split('/')[-1]]*NUM_ITERS
n_params = sum(p.numel() for p in model.parameters() if p.requires_grad)
log['num trainable parameters'] = [n_params]*NUM_ITERS

log_to_csv(log)
