import sys
import torch
from torch.optim import Adam
import torch.nn as nn
from pypermu import problems
from nnco.pl import PLHead, PlackettLuce
from nnco import utility
import numpy as np
from scipy.stats import entropy
from collections import OrderedDict

problem = problems.lop.Lop(sys.argv[1])

# DEVICE = 'cuda:0'
DEVICE = 'cpu'
config = {
    'batch size'    : 32,
    'num samples'   : 64 ,
    'noise len'     : 128,
    'num prehead'   : 2,
    'hidden dim'    : 128,
    'learning rate' : 0.003,
    'instance'      : sys.argv[1].split('/')[-1],
    'max evals'     : 1000*problem.size**2,
    'problem size'  : problem.size,

    # regularization beta
    'reg beta'      : 0.01,
}

NUM_ITERS = int(config['max evals']/(config['batch size']*config['num samples']))

def mikel_divergence(w, reduction=None):
    moda = np.argsort(w)[::-1] # argsort max to min
    n = len(w)
    res = []

    # add an small correction value in order to avoid numerical errors
    w += 1e-200

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
        for i, h in enumerate(mikel_divergence(torch.exp(logits).cpu().numpy())):
            H[i].append(h)

    # mean across batch
    for i in range(len(H)):
        H[i] = np.mean(H[i])

    return H

prehead_layers = []
for i in range(config['num prehead']):
    in_dim = config['noise len'] if i == 0 else config['hidden dim']
    prehead_layers.append((f'linear_{i}', torch.nn.Linear(in_dim, config['hidden dim'])))
    prehead_layers.append((f'relu_{i}', nn.ReLU()))

in_dim = config['noise len'] if config['num prehead'] == 0 else config['hidden dim']
head = PLHead(input_dim=in_dim,
              sample_length=problem.size,
              num_samples=config['num samples'],
              device=DEVICE)

model = nn.Sequential(OrderedDict(prehead_layers + [('head', head)])).to(DEVICE)

optimizer = Adam(model.parameters(), lr=config['learning rate'])

n_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

log = {
    # NOTE: `best fitness` is saved to the `best_fitness` 
    # variable and loged in the end of the experiment
    'mikel convergency': [],
    'w vec l2 norm': [],
    'loss': [],
    'tot var dist': [],
    'mean moda logp': [],
}

best_fitness = []
for iter in range(NUM_ITERS):
    x = torch.normal(mean=0, std=1, 
            size=(config['batch size'], config['noise len'])).to(DEVICE)
    
    samples, logps, distrib = model(x)

    fitness = [problem.evaluate(batch) for batch in samples.cpu().numpy()]
    fitness = torch.as_tensor(fitness, dtype=torch.float32, device=DEVICE)

    u = utility.standarized_utility(fitness)
    loss = -(logps * u).mean()

    #### NEW ####
    modas = torch.argsort(distrib, descending=True, dim=-1)
    antimodas = torch.argsort(distrib, descending=False, dim=-1)
    permus = torch.cat([modas.unsqueeze(0), antimodas.unsqueeze(0)], dim=0)
    pl = PlackettLuce(logits=distrib, device=DEVICE)
    moda_logps, antimoda_logps = pl.log_prob(permus)
    
    moda_p = torch.exp(moda_logps.double())
    antimoda_p = torch.exp(antimoda_logps.double())
    
    v = 1/config['problem size']
    tot_var_dist = torch.max(moda_p - v, antimoda_p - v).mean()
    # print(f'{iter}/{NUM_ITERS} tot var: {tot_var_dist.item()}')
    # if len(best_fitness) > 0: print(best_fitness[-1], fitness.mean())

    #########################################
    # TVD Loss
    # loss += config['reg beta']*tot_var_dist
    #########################################

    #########################################
    # Moda logP Loss
    # logps_map = {
    #         44: -125.317,
    #         50: -148.477,
    #         56: -172.352,
    #         60: -188.628,
    #         79: -269.291,
    #         150: -605.020,
    #         250: -1134.045,
    # }

    # logp_uniform = torch.as_tensor(logps_map[config['problem size']])
    # dist_unif = torch.abs(moda_logps.mean() - logp_uniform)
    moda_logp_mean = moda_logps.mean()
    # print(f'loss before: {loss.item()}, logp moda: {moda_logp_mean.item()}')
    loss += config['reg beta']*moda_logp_mean
    # print(f'loss: {loss.item()}, moda_logp_mean: {moda_logp_mean}\n')

    #########################################
    
    # compute the L2 norm of the PL distribution's weights
    w = distrib.mean(0) # average across batch
    w_l2 = w.pow(2).sum().sqrt()

    # loss += config['reg beta']*w_l2

    #### NEW ####

    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    # ---- logging ---- #
    if len(best_fitness) == 0 or fitness.max() > best_fitness[-1]:
        best_fitness.append(fitness.max().item())
    else:
        best_fitness.append(best_fitness[-1])

    h = np.mean(entropy_pl(distrib.detach()))

    log['mikel convergency'].append(h)
    log['w vec l2 norm'].append(w_l2.item())
    log['loss'].append(loss.item())
    log['tot var dist'].append(tot_var_dist.item())
    log['mean moda logp'].append(moda_logp_mean.item())

    # print(f'{iter}/{NUM_ITERS} mean: {fitness.mean()}, best: {best_fitness[-1]}')

log['best fitness'] = best_fitness

import log as Log
Log.save_dicts(config, log, dir='results_convergency')
