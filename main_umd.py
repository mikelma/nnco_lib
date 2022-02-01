import torch
import sys
import torch
from torch.optim import Adam
import torch.nn as nn
from pypermu import problems
from pypermu import utils as permutils
from scipy.stats import entropy
import numpy as np

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

model = nn.Sequential(
            LinearParallel(
                in_dim=config['noise len'],
                out_dim=config['hidden dim'],
                num_linears=problem.size,
                activation=nn.ReLU()),
            UMDHead(
                input_dim=config['hidden dim'],
                sample_length=problem.size,
                num_samples=config['num samples'],
                # hidden_dim=config['hidden dim'],
                # num_prehead_layers=config['num prehead'],
            ),

            # UMDHead(
            #     input_dim=config['noise len'],
            #     sample_length=problem.size,
            #     num_samples=config['num samples'],
            # ),
        ).to(DEVICE)

optimizer = Adam(model.parameters(), lr=config['learning rate'])

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

n_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

wandb.config.update({
    'problem': 'pfsp',
    'num trainable params': n_params,
})
