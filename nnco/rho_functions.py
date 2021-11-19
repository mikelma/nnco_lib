import torch
from torch import Tensor
import torch.nn as nn

class LogPNormalization(nn.Module):
    def __init__(self):
        super(LogPNormalization, self).__init__()

    def forward(self, logits: Tensor) -> Tensor :
        # print(logits.min(), logits.max(), logits.isnan().any())
        # res = torch.exp(logits - torch.sum(logits, dim=-1, keepdim=True))
        # print(res.min(), res.max(), res.isnan().any())
        # quit()
        # return logits - torch.sum(logits, dim=-1, keepdim=True)
        print('uninmplemented')
        quit()
