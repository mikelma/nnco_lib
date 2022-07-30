# nnco-lib: Distribution Estimation with neural networks for combinatorial optimization

This repository contains all the source code of the research
paper: *Distribution Estimation with neural networks for
combinatorial optimization*.

**NOTE:** The paper hasn't been published yet. Once published,
a link to the paper will be provided.

## Abstract

Probability distributions have been successfully used for
approaching combinatorial optimization problems. However,
in many cases, using such methods introduce large computational
overheads, and in some cases, controlling the convergence
of the algorithm has shown challenging.

The aim of this paper is to introduce a new framework that by
the hand of neural networks is able to efficiently estimate
probability distributions over the search space of combinatorial
problems. Particularly, the approach permits to iteratively
update the parameters of a neural network model whose output
describes probability distributions that give high probability
to the solutions of the problem that are likely to have high
objective function value. Conducted experiments reveal that
the presented proposal has better performance in terms of objective
value and execution time when compared to analogous estimation
of distribution algorithms. Not limited to that, the experimental
analysis points out a much more controllable convergence of the
algorithm by adjusting the settings of the neural network.

## Dependencies

* `pytorch` (tested on 1.12.0)
* `numpy` (tested on 1.22.3)
* [`pypermu`](https://github.com/mikelma/pypermu) (a precompiled binary
is provided in this repo, only for Linux x86_64)
* `matplotlib` (optional, only used in the examples)

## How to use


### Using as a library

This repository provides a python module (library) that you can
use to develop your own experiments. The module is located in `nnco/`, just
copy this directory to the working directory of your project.

Example:

```bash
mkdir my-own-project
cd my-own-project

git clone https://github.com/mikelma/nnco_lib.git
ln -s nnco_lib/nnco nnco

echo "import nnco" >> main.py
python3 main.py
```


### Running the examples

The best way to get started is to run the provided examples.
Examples are located in the `examples/` direcotry within this repo.

In order to run the examples:

```bash
git clone https://github.com/mikelma/nnco_lib.git
cd nnco_lib

cp examples/main_umd.py
python3 main_umd.py instances/PFSP/tai20_5_0.fsp
```
