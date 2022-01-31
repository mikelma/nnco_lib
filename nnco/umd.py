import torch
import torch.nn as nn
import torch.nn.functional as F
from torch import Tensor
from torch.distributions.categorical import Categorical

class UMDHead(nn.Module):
    def __init__(self, 
            input_dim: int = 128, 
            num_samples: int = 3,
            sample_length: int = 5,
            #num_prehead_layers: int = 1,
            #hidden_dim: int = 128,
            rho_function: nn.Module = None):

        super(UMDHead, self).__init__()

        self.input_dim = input_dim
        self.num_samples = num_samples
        self.sample_length = sample_length
        self.rho_function = rho_function

        # self.num_prehead_layers = num_prehead_layers

        # add prehead layers
        # layers = []
        # for i in range(num_prehead_layers):
        #     in_dim = input_dim if i == 0 else hidden_dim
        #     layers.append(nn.ModuleList([
        #         nn.Linear(in_dim, hidden_dim) for _ in range(sample_length)
        #     ]))
        # self.prehead_layers = nn.ModuleList(layers)

        # head layers
        # in_dim = hidden_dim if num_prehead_layers > 0 else input_dim
        self.out_layer = nn.ModuleList([
            nn.Linear(input_dim, sample_length-i) for i in range(sample_length)
        ])

    def forward(self, x) -> tuple[Tensor, Tensor, list]:
        # copy the input n times, one for each output distribution
        # hidden = [x]*self.sample_length

        # go through all the pprehead layers
        # for i, layer in enumerate(self.prehead_layers):
        #     for j, linear in enumerate(layer):
        #         hidden[j] = torch.relu(linear(hidden[j]))

        if type(x) != list:
            # copy the input tensor `n` times, one for each output distribution
            x = [x]*self.sample_length

        logps = []   # list of (batch_size, 1, num_samples) sized tensors
        samples = [] # same as `logps`
        distrib = []
        for lin_in, linear in zip(x, self.out_layer):
            logits = linear(lin_in) # logits: (batch_size, sample_length-i)

            if self.rho_function != None:
                logits = self.rho_function(logits) 

            distrib.append(logits.detach())
            dist = Categorical(logits=logits)

            s = dist.sample((self.num_samples,)) # s: (num_samples, batch_size)
            logp = dist.log_prob(s)              # logp: (num_samples, batch_size)
            
            # s: (batch_size, 1, num_samples)
            s = s.T.unsqueeze(1)
            # logps: (batch_size, 1, num_samples)
            logp = logp.T.unsqueeze(1)

            samples.append(s)
            logps.append(logp)

        # samples: (batch_size, num_samples, sample_length)
        samples = torch.cat(samples, dim=1).permute(0, 2, 1)
        # logps: (batch_size, num_samples)
        logps = torch.cat(logps, dim=1).sum(1)

        return samples, logps, distrib

class LinearParallel(nn.Module):
    '''
    Parallel linear layer. Applies multiple linear
    layers in parallel to a list of input tensors.
    '''
    def __init__(self,
            num_linears: int,
            in_dim: int,
            out_dim: int,
            activation: nn.Module = nn.ReLU()):

        super(LinearParallel, self).__init__()

        self.activation = activation
        self.num_linears = num_linears

        self.linears = nn.ModuleList(
                [nn.Linear(in_dim, out_dim) for _ in range(num_linears)])

    def forward(self, x) -> list[Tensor]:
        '''
        If the input `x` is a list of tensors, feeds each linear layer with
        its corresponding input tensor, else, if `x` is a tensor, all linears
        are feed with the same tensor. Finally all ouput vectors get an activation
        function applied and are returner in a list.
        '''

        if type(x) != list:
            x = [x]*self.num_linears

        result = []
        for linear, linear_in in zip(self.linears, x):
            result.append(self.activation(linear(linear_in)))

        return result
