# nnco-lib: A combinatorial optimization framework for probability-based algorithms by means of generative models

This repository contains all the source code and data of the paper *A combinatorial optimization framework for probability-based algorithms by means of generative models*.

**NOTE:** The paper hasn't been published yet. Once published, a link to the paper will be provided.

**NOTE:** This repository is hosted in [SourceHut](https://git.sr.ht/~mikelma/nnco_lib),
but a [GitHub](https://github.com/mikelma/nnco_lib) mirror is also maintained
for convenience.

- ⭐ **Try it online!** ⭐ [Colab notebook](https://colab.research.google.com/drive/1BNtRUHaL6qe2E1CO1mxJORO4o-oPMP5f?usp=sharing)
- [Jupyter notebook](./A_combinatorial_optimization_framework_for_probability_based_algorithms_by_means_of_generative_models.ipynb)
- [Jupyter notebook (.py version)](./A_combinatorial_optimization_framework_for_probability_based_algorithms_by_means_of_generative_models.py)

## Abstract

Probability-based algorithms have proven to be a solid alternative for approaching optimization problems. Nevertheless, in many cases, using probabilistic models that efficiently exploit the characteristics of the problem involves large computational overheads, and therefore, lower complexity models such as \mdf{those that are univariate} are usually employed within approximation algorithms.

With the motivation to address such an issue, in this paper, we aim to introduce an iterative optimization framework that employs generative models to efficiently estimate the parameters of probability models for optimization problems. This allows the use of complex probabilistic models (or those that are appropriate for each problem) in a way that is feasible to apply them iteratively. Specifically, the framework is composed of three elements: a generative model, a probability model whose probability rule is differentiable, and a loss function. The possibility of modifying any of the three elements of the framework offers the flexibility to design algorithms that best adapt to the problem at hand.

Experiments conducted on two case studies reveal that the presented approach has strong performance in terms of objective value and execution time when compared to other probability-based algorithms.
Moreover, the experimental analysis demonstrates that the convergence of the algorithms is controllable by adjusting the components of the framework.

## Dependencies

> ⚠️ Note that a [Colab notebook](https://colab.research.google.com/drive/1BNtRUHaL6qe2E1CO1mxJORO4o-oPMP5f?usp=sharing) is available to try the library without needing to install or setup anything.

The first step is to clone the repo:

```bash
git clone https://github.com/mikelma/nnco_lib.git
```

Then, install python dependencies

```bash
pip install -r requirements.txt
```

The modules to install are:

* `torch` 🔥
* `numpy` 🐍
* `matplotlib` 🎨 (optional, only used in the examples)

Finally, this repo depends on [`pypermu`](https://github.com/mikelma/pypermu), a python module implemented in Rust. `pypermu` provides fast
implementations of common permutation optimization problems and operations. For convenience, this repo contains a precompiled binary of the library,
so you might not need to build it from source. If you need or want to build it from source, see the next section.

### Building pyypermu (optional)

Follow the instructions on Rust's website ([here](https://www.rust-lang.org/learn/get-started)) to set up Rust in your machine.

Clone the repository with submodules:

```bash
git clone --recurse-submodules https://github.com/mikelma/nnco_lib.git
```

`cd` into the `pypermu` submodule and build the library (with optimizations) using `cargo`:

```bash
cd nnco_lib/pypermu && cargo b --release
```

Finally, move the resulting binary to the `nnco_lib` repo's path
```bash
mv target/release/libpypermu.so ../../pypermu.so
```

## Running the examples

Besides the notebooks, example scripts are located in the `examples/` directory.

To run the examples (assuming that dependencies are installed and working):

```bash
# PFSP + UMD
cp examples/main_umd.py
python3 main_umd.py instances/PFSP/tai20_5_0.fsp

# LOP + PL
cp examples/main_pl.py
python3 main_pl.py instances/LOP/tai20_5_0.fsp
```
