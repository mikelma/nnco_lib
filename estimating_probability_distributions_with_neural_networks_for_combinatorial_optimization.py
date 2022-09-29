# -*- coding: utf-8 -*-
"""Estimating Probability Distributions with Neural Networks for Combinatorial Optimization.ipynb

Automatically generated by Colaboratory.

Original file is located at
    https://colab.research.google.com/drive/1BNtRUHaL6qe2E1CO1mxJORO4o-oPMP5f

# Estimating Probability Distributions with Neural Networks for Combinatorial Optimization

This notebook shows two simple examples of the python library developed for the *Estimating Probability Distributions with Neural Networks for Combinatorial Optimization* paper.

The repository of the project is available in [SourceHut](https://git.sr.ht/~mikelma/nnco_lib) and in [GitHub](https://github.com/mikelma/nnco_lib) (mirror).



> **Quick note:**  The objective of this document is to illustrate practical usage of the ideas developed in the paper, and it assumes that the user has read it. Feel free to [send me a mail](mailto:mmalagon002@ikasle.ehu.eus) if you have any question!

## Dependencies

First of all, let's clone the project's repository and build it's dependencies. This might take few minutes to complete, as some dependencies have to be compiled from source.

> If you are running this notebook in your own machine, please make sure that you have  
[Rust](https://www.rust-lang.org/learn/get-started) (version 1.41 and up) and [PyTorch](https://pytorch.org/get-started/locally/) (tested in v1.12) installed.

*The nnco_lib repository provides:*
- A python library implementing the optimization framework proposed in the paper.
- The set of optimization instances used in the paper (LOP and PFSP).
- The `pypermu` python library: provides fast implementations of common permutation operations and problems (it is implemented using the Rust programming language).
"""

# Commented out IPython magic to ensure Python compatibility.
#@title Dependency installation & building
# install build dependencies
!apt install rust-all
# clone project's repo
!git clone --recurse-submodules https://git.sr.ht/~mikelma/nnco_lib
# change notebook's cwd
# %cd nnco_lib
# build pypermu libarary
!cd pypermu && cargo b --release && mv target/release/libpypermu.so ../pypermu.so
# unzip LOP instances
!mkdir -p all_instances/{LOP,PFSP} && cd all_instances/LOP && unzip -q ../../instances/LOP/IO.zip && cd ../PFSP && unzip -q ../../instances/PFSP/PFSP.zip

#@title Select correct git branch
import sys,os
# get runtime's python version
py_version = int(sys.version.split(' ')[0].split('.')[1])
# check if the minor version is below 9
if py_version <= 8:
    # move to the `cluster` branch. This branch has few modifications 
    # in the typing that makes it work with python's below v3.9 
    print("* Python version lower than 3.9: moving to `cluster` branch")
    os.system("git checkout cluster")

"""## Optimization of a LOP instance

In this section we will use the optimization scheme introduced in the paper to approach a LOP instance.

The code next block loads all the necessary modules and data needed to run this example. Note that the only python requirements are [PyTorch](https://pytorch.org/) and [`nnco_lib`](https://git.sr.ht/~mikelma/nnco_lib) (where the paper's ideas have been implemented).
"""

import torch
from torch.optim import Adam
import torch.nn as nn

# libraries included in the `nnco_lib`  repository 
from nnco.pl import PLHead
from nnco import utility
from pypermu import problems

"""The next code block is used to detect if there's a GPU card available to use. GPU cards highly increase the optimization speed of this algorithm, however, for the simple examples provided in this notebook a GPU card isn't mandatory."""

#@title Check for available GPU 

if torch.cuda.device_count() > 0:
    device = 'cuda:0'
else:
    device = 'cpu'

print(f"* Device: {device}")

"""Let's select the path where the LOP instance to optimize is located. In this code block the size of the instance has also been specified for later usage."""

instance = "all_instances/LOP/N-be75eec"
size = 50

problem = problems.lop.Lop(instance)

"""In the following cell the hyperparameters of the algorithm are defined. 
The provided parameters are some sane defaults, but feel free to play with this values!
"""

# Number of solutions sampled per iteration from the probability distribution 
# over the solution space of the problem.
num_samples = 64

# Dimension of the noise vector: the input to the generative model.
noise_dim = 128

# Same as the usual batch size and learning rate in ML.
batch_size = 32
learning_rate = 0.003

"""As the algorithm's execution has to be somehow limited, in this example we'll follow the same stopping criterion as in the paper: the number of solutions that the algorithm is able to evaluate is limited. 
<br> <br>

Following the paper,
$$
\text{max_evals} = 1000n^2
$$

<br> <br>
Consequently,
$$
\text{max_iters} = \frac{\text{max_evals}}{\text{batch_size} * \text{num_samples}} 
$$

"""

max_evals = 1000*size**2
num_iters = int(max_evals/(batch_size*num_samples))

print("* Maximum solution evaluations:", max_evals)
print("* Total number of iterations:  ", num_iters)

"""### Defining the generative model 

In this code block, the generative model is defined as a PyTorch model. 
Again, feel free to play with different NN architectures. However, be sure to always set the input size equal to the dimension of the noise vector (`noise_dim`) and the ouput shape to: `num_samples` $\times$ `sample_size`.

> The last layer of the model, [`PLHead`](https://git.sr.ht/~mikelma/nnco_lib/tree/main/item/nnco/pl.py), is a custom module provided in the `nnco_lib` library. Given an input vector, `PLHead` feeds the vector to a [`LinearLayer`](https://pytorch.org/docs/stable/generated/torch.nn.Linear.html), then the output of this liner layer is used to parametrize a PL distribution. Finally, the PL distribution is samped and the obtained solutions together with their log prababilities are returned.
"""

model = nn.Sequential(
            ## for example, you can uncomment the two lines below to add an 
            ## extra layer to the generative model

            # torch.nn.Linear(noise_dim, noise_dim),
            # torch.nn.ReLU(),
            
            PLHead(
                input_dim=noise_dim,
                sample_length=size,
                num_samples=num_samples,
                device=device
            ),
        ).to(device)
model

"""In order to update the parameters of the generative model that we've just created, we'll initialize a PyTorch optimizer. 
In this case, the gradient descent algorithm employed is the [Adam](https://pytorch.org/docs/stable/generated/torch.optim.Adam.html) algorithm (same as in the paper), but [other algorithms](https://pytorch.org/docs/stable/optim.html) can be used too.
"""

optimizer = Adam(model.parameters(), lr=learning_rate)

"""### Main loop

First of all, let's create a simple function that will help us track the progress of the optimization process. This function just saves the best objective value found so far and prints some relevant information, such as the mean fitness of the current samples and the value of the loss function.
"""

best_fitness = []

def logger(fitness_lst, iter, max_iters, loss, is_minim=False):
    val_max = fitness_lst.max().item()
    if len(best_fitness) == 0 or (val_max > best_fitness[-1] and not is_minim) or (val_max < best_fitness[-1] and is_minim):
        best_fitness.append(val_max)
    else:
        best_fitness.append(best_fitness[-1])
    print(f'{iter}/{max_iters} loss: {loss.item()}, mean: {fitness.mean()}, \
best: {best_fitness[-1]}')

"""Finally, the code bock below holds the main loop of the algorithm. Run it to start optimizing! ⚡

This is the description of the steps that compose the main loop:

1. Sample a set $\mathcal{Z}$ of input vectors $\mathbf{z}$ from a normal distribution:
$$\mathcal{Z} = \{ \mathbf{z}^i | \mathbf{z}^i_j \sim \mathcal{N}(0, 1), j=1,\ldots,m\}_{i=1,\ldots,\tau}$$

2. Sample a set of solutions $\mathcal{D}^{\mathbf{z}}$ for each input vector $\mathbf{z}$, 
$$~{\mathcal{D}^{\mathbf z} = \{ \mathbf{x}^{i, \mathbf z} | \mathbf{x}^i \sim P_\theta(\mathbf x | \mathbf z) \}_{i=1,\ldots,\lambda}}$$

3. Compute the objective function value $f(\mathbf{x})$ of each solution $\mathbf{x}$.

4. Standarize the objective function values obtained in the previous step.

5. Compute the loss function value and the gradient estimate $d_t$:
$$
d_t = \frac{1}{\tau\lambda} \sum_{\mathbf z \in \mathcal{Z}} \sum_{\mathbf x \in \mathcal{D}^{\mathbf z}}\nabla_\theta \log(P_\theta (\mathbf x| \mathbf z))f(\mathbf x)
$$
<br>And update the parameters $\theta$ of the generative model $g_\theta$ as, $$\theta_{t+1} \gets \theta_t - \alpha d_t$$




"""

for iter in range(num_iters):
    # (1)
    z = torch.normal(mean=0, std=1, 
                     size=(batch_size, noise_dim), device=device)
    
    # (2)
    samples, logps = model(z)

    # (3)
    fitness = [problem.evaluate(batch) for batch in samples.cpu().numpy()]
    fitness = torch.as_tensor(fitness, dtype=torch.float32, device=device)

    # (4)
    u = utility.standarized_utility(fitness)

    # (5)
    loss = -(logps * u).mean()
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    logger(fitness, iter, num_iters, loss)

print(f'Best objective function value: {best_fitness[-1]}')

"""### Result visualization"""

import matplotlib.pyplot as plt

plt.plot(range(len(best_fitness)), best_fitness)
plt.xlabel('Iteration')
plt.ylabel('Best objective function value')
plt.title(instance.split('/')[-1])
plt.show()

"""## Optimizing a PFSP instance

The flexibility of the presented framework allows to optimize different combinatorial problems with minimal changes to the implementation. To illustrate this fact, in this section we'll approach a completly different CO problem, the PFSP. 

<br>

As it was the case with the LOP, the first step will be to import the optimization instance that we want to optimize. Note that the new instance to optimize has the same size as the one in the previous example, $n=50$, thus there is no need to compute the maximum number of iterations again.
"""

instance = "all_instances/PFSP/tai50_5_8.fsp"
problem = problems.pfsp.Pfsp(instance)

"""### The generative model

When optimizing the LOP, the employed probability distribution was the *Placket-Luce* distribution. 
With the purpose following the decissions made in the paper, we'll move to the *Univariate Marginals Distribution* (UMD) to optimize the PFSP.

In consequence, the architecture of the generative model must change to output the parameters of the UMD instead of the PL distribution.

> Again, the last layer of the model is a custom layer, [`UMDHead`](https://git.sr.ht/~mikelma/nnco_lib/tree/main/item/nnco/umd.py), implemented in `nnco_lib`. This layer is follows the same procedure as the `PLHead` model, except that it builts and samples a UMD instead of a PL distribution.

> [`LinearParallel`](https://git.sr.ht/~mikelma/nnco_lib/tree/main/item/nnco/umd.py) is a custom layer that given an input vector(s), runs the input through $n$ linear layers, returning the generated $n$ output vectors.
"""

from nnco.umd import UMDHead, LinearParallel
model = nn.Sequential(
            ## it is possible to replace the `LinearParallel` layer with a 
            ## normal Linear layer.

            # torch.nn.Linear(noise_dim, noise_dim),
            # torch.nn.ReLU(),

            LinearParallel(
                in_dim=noise_dim,
                out_dim=noise_dim,
                num_linears=size,
                activation=nn.ReLU()),
            UMDHead(
                input_dim=noise_dim,
                sample_length=size,
                num_samples=num_samples,
            )
        ).to(device)

# generate an optimizer for the new model
optimizer = Adam(model.parameters(), lr=learning_rate)

"""### The main loop

Before starting to optimize the PFSP instance, there are two more changes to the implementation shown in the LOP example:

1. Instead of directly evaluating the permutations sampled from the probability distribution (see 3rd step in the block below), in the PFSP we'll evaluate the inverse of the samples (refer to the paper for more information). In summary, instead of calculating $f(\sigma)$, $f(\sigma^{-1})$ is computed.

2. In the LOP example, there was a minus sign in front of the loss function (see 4th step). For the PFSP the minus sign has been removed. The change is caused by the fact thet the LOP is a maximization problem and the PFSP is a minimization problem (again, refer to the paper for additional information). 
"""

from pypermu import utils as permutils

best_fitness = [] # restart best fitness list (used by `logger`)
for iter in range(num_iters):
    # (1)
    z = torch.normal(mean=0, std=1, 
                     size=(batch_size, noise_dim), device=device)
    
    # (2)
    samples, logps = model(z)

    # (3)
    permus = [permutils.transformations.marina2permu(b) for b in samples.cpu().numpy()]
    permus = [permutils.transformations.permu2inverse(batch) for batch in permus]

    fitness = [problem.evaluate(batch) for batch in permus]
    fitness = torch.as_tensor(fitness, dtype=torch.float32, device=device)

    # (4)
    u = utility.standarized_utility(fitness)

    # (5)
    loss = (logps * u).mean()
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    logger(fitness, iter, num_iters, loss, is_minim=True)

print(f'Best objective function value: {best_fitness[-1]}')

"""### Visualizing the results"""

import matplotlib.pyplot as plt

plt.plot(range(len(best_fitness)), best_fitness)
plt.xlabel('Iteration')
plt.ylabel('Best objective function value')
plt.title(instance.split('/')[-1])
plt.show()