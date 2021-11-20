import torch
import torch.nn as nn
import torch.nn.functional as F
from torch import Tensor
from torch.distributions.categorical import Categorical

from typing import Tuple

class UMDHead(nn.Module):
    def __init__(self, 
            input_dim: int = 128, 
            num_samples: int = 3,
            sample_length: int = 5,
            rho_function: nn.Module = None):

        super(UMDHead, self).__init__()

        self.input_dim = input_dim
        self.num_samples = num_samples
        self.sample_length = sample_length
        self.rho_function = rho_function

        self.linears = nn.ModuleList([
            nn.Linear(input_dim, sample_length-i) for i in range(sample_length)
        ])

    def forward(self, x: Tensor) -> Tuple[Tensor, Tensor]:
        logps = []   # list of (batch_size, 1, num_samples) sized tensors
        samples = [] # same as `logps`
        for linear in self.linears:
            logits = linear(x) # logits: (batch_size, sample_length-i)

            if self.rho_function != None:
                logits = self.rho_function(logits) 

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

        return samples, logps


