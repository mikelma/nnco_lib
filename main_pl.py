import sys
import torch
from torch.optim import Adam
import torch.nn as nn
from pypermu import problems
from nnco.pl import PLHead
from nnco import utility

BATCH_SIZE    = 32
NUM_SAMPLES   = 64
LEARNING_RATE = 0.003
NOISE_LEN     = 128
INSTANCE      = sys.argv[1] 

problem = problems.lop.Lop(INSTANCE)

MAX_EVALS = 1000*problem.size**2
NUM_ITERS = int(MAX_EVALS/(BATCH_SIZE*NUM_SAMPLES))

model = nn.Sequential(
            # torch.nn.Linear(NOISE_LEN, NOISE_LEN),
            # torch.nn.ReLU(),
            PLHead(
                input_dim=NOISE_LEN,
                sample_length=problem.size,
                num_samples=NUM_SAMPLES,
            ),
        )

optimizer = Adam(model.parameters(), lr=LEARNING_RATE)

best_fitness = []
best_solution = None
for iter in range(NUM_ITERS):

    # x = torch.rand(NOISE_LEN*BATCH_SIZE).view(BATCH_SIZE, NOISE_LEN)
    x = torch.normal(mean=0, std=1, size=(BATCH_SIZE, NOISE_LEN))
    # x = torch.ones((BATCH_SIZE, NOISE_LEN))
    # x = torch.zeros((BATCH_SIZE, NOISE_LEN))
    
    samples, logps = model(x)

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
        loc = fitness.argmax().item()
        #best_solution = samples[loc // BATCH_SIZE, loc % NUM_SAMPLES]
    else:
        best_fitness.append(best_fitness[-1])

    print(f'{iter}/{NUM_ITERS} loss: {loss.item()}, mean: {fitness.mean()}, best: {best_fitness[-1]}')

# bf = problem.evaluate(best_solution.view(1, -1).cpu().numpy())
# print(f'Best solution found (fitness {bf}):\n{best_solution}')
print(f'Best solution found {best_fitness[-1]}')

import matplotlib.pyplot as plt

plt.plot(range(NUM_ITERS), best_fitness, label='best fitness')
# plt.hlines(236464, 0, NUM_ITERS, 'r', label='Best known')
plt.xlabel('Iteration')
plt.xlabel('Fitness')
plt.title(INSTANCE.split('/')[-1])
plt.legend()
plt.show()
