import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import numpy as np
import time
from .common import *

class ResBlock(nn.Module):
  def __init__(self,filters,use_bias):
    super(ResBlock,self).__init__()
    
    self.conv1 = nn.Conv2d(filters,filters,kernel_size=3,padding=1,bias=use_bias)
    self.bn1 = nn.BatchNorm2d(filters)
    self.conv2 = nn.Conv2d(filters,filters,kernel_size=3,padding=1,bias=use_bias)
    self.bn2 = nn.BatchNorm2d(filters)

  def forward(self,x):
    x_copied = x
    x = F.relu(self.bn1(self.conv1(x)))
    x = self.bn2(self.conv2(x))
    x += x_copied
    x = F.relu(x)
    return x

# 入力 (batch_size,N_card*2+41,stage::h,stage::w)
# 1x2 通常マス
# 1x2 SPマス
# 1 壁・盤面外
# 12 何ターン目か
# 12x2 SPポイントがkポイント以上溜まっているか(1≤k≤12)
# N_card x2 デッキに含まれる未使用カード(手札を含む)か

# 出力 (batch_size,(N_CARD*(H*W*8+1),(1)))
INPUT_C = N_CARD*2+41
ACTION_SPACE_OF_EACH_CARD = H*W*8+1

class AlphaZeroResNet(nn.Module):
  def __init__(self,H:int,W:int,n_blocks=19,filters=256,use_bias=False):
    super(AlphaZeroResNet,self).__init__()

    self.conv1 = nn.Conv2d(INPUT_C,filters,kernel_size=3,padding=1,bias=use_bias)
    self.bn1 = nn.BatchNorm2d(filters)

    #: residual tower
    self.blocks = nn.Sequential(*[ResBlock(filters,use_bias) for _ in range(n_blocks)])

    #: policy (action) head
    self.conv_p_action_put = nn.Conv2d(filters,N_CARD*8,kernel_size=1,bias=True)
    self.conv_p_action_pass = nn.Conv2d(filters,2,kernel_size=1,bias=use_bias)
    self.bn_p_action_pass = nn.BatchNorm2d(2)
    self.logits_action_pass = nn.Linear(2*H*W,N_CARD)
    self.flat_p_action_pass = nn.Flatten()
    self.flat_p_action = nn.Flatten()

    #: policy (redraw) head
    self.conv_p_redraw = nn.Conv2d(filters,4,kernel_size=1,bias=use_bias)
    self.bn_p_redraw = nn.BatchNorm2d(4)
    self.flat_p_redraw = nn.Flatten()
    self.logits_redraw = nn.Linear(4*H*W,2)

    #: value head
    self.conv_v = nn.Conv2d(filters,4,kernel_size=1,bias=use_bias)
    self.bn_v = nn.BatchNorm2d(4)
    self.flat_v = nn.Flatten()
    self.fc1_v = nn.Linear(4*H*W,256)
    self.fc2_v = nn.Linear(256,1)

  def forward(self,x):
    x = F.relu(self.bn1(self.conv1(x)))
    
    x = self.blocks(x)

    #: policy (action) head
    # x_p_action = F.relu(self.bn_p_action(self.conv_p_action(x)))
    # x_p_action = self.flat_p_action(x_p_action)
    # logits_action = self.logits_action(x_p_action)
    x_p_action_put = self.conv_p_action_put(x).view(-1,N_CARD,8*H*W)
    x_p_action_pass = self.flat_p_action_pass(self.bn_p_action_pass(self.conv_p_action_pass(x)))
    x_p_action_pass = F.relu(self.logits_action_pass(x_p_action_pass)).view(-1,N_CARD,1)
    x_p_action = torch.cat((x_p_action_pass,x_p_action_put),dim=2)
    logits_action = self.flat_p_action(x_p_action)


    #: policy (redraw) head
    x_p_redraw = F.relu(self.bn_p_redraw(self.conv_p_redraw(x)))
    x_p_redraw = self.flat_p_redraw(x_p_redraw)
    logits_redraw = self.logits_redraw(x_p_redraw)

    #: value head
    x_v = F.relu(self.bn_v(self.conv_v(x)))
    x_v = self.flat_v(x_v)
    x_v = self.fc2_v(self.fc1_v(x_v))
    value = torch.tanh(x_v)

    return logits_action,logits_redraw,value

class AlphaZeroResNet_CPP(nn.Module):
  def __init__(self,model:nn.Module):
    super(AlphaZeroResNet_CPP,self).__init__()
    self.model = model
  def forward(self,x):
    return torch.cat(self.model.forward(x),dim=1)

def main():
  H,W=26,9
  model = AlphaZeroResNet(H,W,n_blocks=19).to("mps")
  print(sum(p.numel() for p in model.parameters() if p.requires_grad))
  dummy_input = torch.rand(64,INPUT_C,H,W).to("mps")
  torch.mps.synchronize()
  start = time.time()
  logits_action,logits_redraw,value = model(dummy_input)
  torch.mps.synchronize()
  elapsed_time = time.time() - start
  print(elapsed_time)


if __name__ == "__main__":
  main()