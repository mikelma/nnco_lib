import torch 

def standarized_utility(x):
    mu = x.mean(-1, keepdim=True)
    sigma = x.std(-1, keepdim=True) + 1e-5

    return (x - mu) / sigma
