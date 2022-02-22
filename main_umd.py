import torch
import sys
import torch
from torch.optim import Adam
import torch.nn as nn
from pypermu import problems
from pypermu import utils as permutils
from scipy.stats import entropy
import numpy as np
from collections import OrderedDict

import wandb

from nnco.umd import UMDHead, LinearParallel
from nnco import utility, rho_functions

problem = problems.pfsp.Pfsp(sys.argv[1])

DEVICE = 'cuda:0'
config = {
    'batch size'    : 32,
    'num samples'   : 64 ,
    'noise len'     : 128,
    'hidden dim'    : 128,
    'num prehead'   : 1,
    'learning rate' : 0.003,
    'instance'      : sys.argv[1].split('/')[-1],
    'max evals'     : 1000*problem.size**2,
    'probelm size'  : problem.size,
}

NUM_ITERS = int(config['max evals']/(config['batch size']*config['num samples']))

wandb.init(project='nnco-convergency-experiment', config=config)

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

prehead_layers = []
for i in range(config['num prehead']):
    in_dim = config['noise len'] if i == 0 else config['hidden dim']
    prehead_layers.append((f'parlinear_{i}',
        LinearParallel(in_dim=in_dim,
            out_dim=config['hidden dim'],
            num_linears=problem.size)))

in_dim = config['noise len'] if config['num prehead'] == 0 else config['hidden dim']
head = UMDHead(input_dim=in_dim,
               sample_length=problem.size,
               num_samples=config['num samples'])

model = nn.Sequential(OrderedDict(prehead_layers + [('head', head)])).to(DEVICE)

optimizer = Adam(model.parameters(), lr=config['learning rate'])

n_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

wandb.config.update({
    'problem': 'pfsp',
    'num trainable params': n_params,
})

best_fitness = []
for iter in range(NUM_ITERS):
    x = torch.normal(mean=0, std=1, 
            size=(config['batch size'], config['noise len'])).to(DEVICE)
    
    samples, logps, distrib = model(x)

    permus = [permutils.transformations.marina2permu(b) for b in samples.cpu().numpy()]
    permus = [permutils.transformations.permu2inverse(batch) for batch in permus]

    fitness = [problem.evaluate(batch) for batch in permus]
    fitness = torch.as_tensor(fitness, dtype=torch.float32, device=DEVICE)

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
    
    h = entropy_umd(distrib, reduction='mean')

    wandb.log({
        'best fitness': best_fitness[-1],
        'entropy approx avg': h,
        'loss': loss.item(),
    }, step=iter)

    # ----------------- #

    print(f'{iter}/{NUM_ITERS} mean: {fitness.mean()}, best: {best_fitness[-1]}')
