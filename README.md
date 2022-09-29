# nnco-lib: Distribution Estimation with neural networks for combinatorial optimization

This repository contains all the source code employed in the paper *Distribution Estimation with neural networks for
combinatorial optimization*, as well as instructions and examples on how to use it.

**NOTE:** The paper hasn't been published yet. Once published,
a link to the paper will be provided.

**NOTE:** This repository is hosted in [SourceHut](https://git.sr.ht/~mikelma/nnco_lib),
but a [GitHub](https://github.com/mikelma/nnco_lib) mirror is also maintained
for convenience.

- ⭐ **Try it online!** ⭐ [Colab notebook](https://colab.research.google.com/drive/1BNtRUHaL6qe2E1CO1mxJORO4o-oPMP5f?usp=sharing)
- [Jupyter notebook](https://git.sr.ht/~mikelma/nnco_lib/tree/main/item/Estimating_Probability_Distributions_with_Neural_Networks_for_Combinatorial_Optimization.ipynb)
- [Jupyter notebook (.py version)](https://git.sr.ht/~mikelma/nnco_lib/tree/main/item/estimating_probability_distributions_with_neural_networks_for_combinatorial_optimization.py)

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
* `matplotlib` (optional, only used in the examples)
* [`Rust`](https://www.rust-lang.org/learn/get-started) (version 1.41 and up): Used to build [`pypermu`](https://github.com/mikelma/pypermu) (a precompiled binary is provided in this repo, only for Linux x86_64)

## How to use

### From a notebook

- [Colab notebook](https://colab.research.google.com/drive/1BNtRUHaL6qe2E1CO1mxJORO4o-oPMP5f?usp=sharing)
- [Jupyter notebook](https://git.sr.ht/~mikelma/nnco_lib/tree/main/item/Estimating_Probability_Distributions_with_Neural_Networks_for_Combinatorial_Optimization.ipynb)
- [Jupyter notebook (.py version)](https://git.sr.ht/~mikelma/nnco_lib/tree/main/item/estimating_probability_distributions_with_neural_networks_for_combinatorial_optimization.py)

### Using the library

This repository provides a python module (library) that you can
use to develop your own experiments. The module is located in the
`nnco/` directory inside this repo.

Download & build & use:

```bash
mkdir my-own-project
cd my-own-project

git clone --recurse-submodules https://github.com/mikelma/nnco_lib.git

# Build dependencies
cd nnco_lib/pypermu && cargo b --release && mv target/release/libpypermu.so ../../pypermu.so
cd ../..

ln -s nnco_lib/nnco nnco

# If executing the following commands don't produce any errors, you are ready to go!
echo "import nnco" >> test.py
echo "from pypermu import problems" >> test.py
python3 test.py
```


#### Running the examples

The best way to get started is to run the provided examples.
Examples are located in the `examples/` direcotry within this repo.

In order to run the examples (these steps assume that you have
followed the instructions above):

```bash
# PFSP + UMD
cp examples/main_umd.py
python3 main_umd.py instances/PFSP/tai20_5_0.fsp

# LOP + PL
cp examples/main_pl.py
python3 main_pl.py instances/LOP/tai20_5_0.fsp
```
