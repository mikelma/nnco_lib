import sys
import torch
from torch.optim import Adam
import torch.nn as nn
from pypermu import problems
from nnco.pl import PLHead
from nnco import utility
import numpy as np
from scipy.stats import entropy
import wandb

problem = problems.lop.Lop(sys.argv[1])

config = {
    'batch size'    : 32,
    'num samples'   : 64 ,
    'noise len'     : 128,
    'learning rate' : 0.003,
    'instance'      : sys.argv[1].split('/')[-1],
    'max evals'     : 1000*problem.size**2,
    'probelm size'  : problem.size,
}

NUM_ITERS = int(config['max evals']/(config['batch size']*config['num samples']))

wandb.init(project='nnco-convergency-experiment', config=config)

def mikel_divergence(w, reduction=None):
    moda = np.argsort(w)[::-1] # argsort max to min
    n = len(w)
    res = []
    for i, e in enumerate(moda[:-1]):
        normalized = w / w.max()
        res.append(entropy(w)/np.log(n-i))
        w[e] = 0.

    if reduction == 'sum':
        res = np.sum(res)
    if reduction == 'mean':
        res = np.mean(res)
    if reduction == 'max':
        res = np.max(res)

    return res

def entropy_pl(ws):
    # an entropy list for each n position
    H = [[] for _ in range(problem.size-1)]
    for logits in ws: # for each w vector in the batch
        for i, h in enumerate(mikel_divergence(torch.exp(logits).numpy())):
            H[i].append(h)

    # mean across batch
    for i in range(len(H)):
        H[i] = np.mean(H[i])

    return H

model = nn.Sequential(
            # torch.nn.Linear(NOISE_LEN, NOISE_LEN),
            # torch.nn.ReLU(),
            PLHead(
                input_dim=config['noise len'],
                sample_length=problem.size,
                num_samples=config['num samples'],
            ),
        )

optimizer = Adam(model.parameters(), lr=config['learning rate'])

best_fitness = []
for iter in range(NUM_ITERS):
    x = torch.normal(mean=0, std=1, 
            size=(config['batch size'], config['noise len']))
    
    samples, logps, distrib = model(x)

    fitness = [problem.evaluate(batch) for batch in samples.cpu().numpy()]
    fitness = torch.as_tensor(fitness, dtype=torch.float32)

    u = utility.standarized_utility(fitness)
    loss = -(logps * u).mean()

    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    # ---- logging ---- #
    if len(best_fitness) == 0 or fitness.max() > best_fitness[-1]:
        best_fitness.append(fitness.max().item())
    else:
        best_fitness.append(best_fitness[-1])

    h = np.mean(entropy_pl(distrib))

    wandb.log({
        'best fitness': best_fitness[-1],
        'mikel convergency avg': h,
        'loss': loss.item(),
    }, step=iter)

    print(f'{iter}/{NUM_ITERS} mean: {fitness.mean()}, best: {best_fitness[-1]}')

print(f'Best solution found {best_fitness[-1]}')

n_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

wandb.config.update({
    'problem': 'lop',
    'num trainable params': n_params,
})
