import torch
import sys
import torch
from torch.optim import Adam
import torch.nn as nn
from pypermu import problems
from pypermu import utils as permutils

from nnco.umd import UMDHead, LinearParallel
from nnco import utility, rho_functions

BATCH_SIZE    = 32
NUM_SAMPLES   = 64 
LEARNING_RATE = 0.003
NOISE_LEN     = 128
INSTANCE      = sys.argv[1] 
HIDDEN_DIM    = 128

problem = problems.pfsp.Pfsp(INSTANCE)

MAX_EVALS = 1000*problem.size**2
NUM_ITERS = int(MAX_EVALS/(BATCH_SIZE*NUM_SAMPLES))

model = nn.Sequential(
            LinearParallel(
                in_dim=NOISE_LEN,
                out_dim=HIDDEN_DIM,
                num_linears=problem.size,
                activation=nn.ReLU()),
            UMDHead(
                input_dim=HIDDEN_DIM,
                sample_length=problem.size,
                num_samples=NUM_SAMPLES,
                # rho_function=rho_functions.LogPNormalization()
            ),
        )

optimizer = Adam(model.parameters(), lr=LEARNING_RATE)

best_fitness = []
for iter in range(NUM_ITERS):
    x = torch.normal(mean=0, std=1, size=(BATCH_SIZE, NOISE_LEN))
    # x = torch.rand(NOISE_LEN*BATCH_SIZE).view(BATCH_SIZE, NOISE_LEN)
    
    samples, logps, _ = model(x)

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

    print(f'{iter}/{NUM_ITERS} loss: {loss.item()}, mean: {fitness.mean()}, best: {best_fitness[-1]}')

print(f'Best solution found {best_fitness[-1]}')

import matplotlib.pyplot as plt

plt.plot(range(NUM_ITERS), best_fitness, label='best fitness')
plt.xlabel('Iteration')
plt.xlabel('Fitness')
plt.title(INSTANCE.split('/')[-1])
plt.legend()
plt.show()
