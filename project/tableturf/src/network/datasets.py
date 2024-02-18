import torch
import numpy as np
from .common import*
from .network import*

class Datasets(torch.utils.data.Dataset):
  def __init__(self):
    self.inputs = []
    self.policy_actions = []
    self.policy_redraws = []
    self.values = []
  def add(self,data_path,buffer_size):
    with open(data_path,"r") as file:
      N_file_data = int(file.readline()) # ファイル内データの個数
      N_trashed_data = max(0,len(self)+N_file_data-buffer_size)
      # buffer_sizeを超えた要素を削除
      del self.inputs[:N_trashed_data]
      del self.policy_actions[:N_trashed_data]
      del self.policy_redraws[:N_trashed_data]
      del self.values[:N_trashed_data]

      for _ in range(N_file_data):
        # input
        # 0~5(盤面)
        input_list = []
        for _ in range(5):
          s = file.readline()
          input_list.append([list(map(float,s[i*W:(i+1)*W])) for i in range(H)])
        # 6~N_CARD*2+40(その他)
        N_channel_all1 = int(file.readline()) # 1で埋められるチャネル数
        channel_indexes_all1 = list(map(int,file.readline().split()))
        assert N_channel_all1 == len(channel_indexes_all1)
        is_filled_all1 = [False]*INPUT_C
        for channel_index in channel_indexes_all1:
          is_filled_all1[channel_index] = True
        for channel_index in range(5,INPUT_C):
          input_list.append(np.ones((H,W)) if is_filled_all1[channel_index] else np.zeros((H,W)))
        
        self.inputs.append(torch.from_numpy(np.array(input_list)).to(torch.float32))

        # 出力(policy_action)
        N_positive_policy_actions = int(file.readline())
        index_policy_action_pairs = file.readline().split()
        assert N_positive_policy_actions*2 == len(index_policy_action_pairs)

        policy_action_array = np.zeros(N_CARD*ACTION_SPACE_OF_EACH_CARD)
        for i in range(N_positive_policy_actions):
          idx = int(index_policy_action_pairs[i*2])
          policy_action = float(index_policy_action_pairs[i*2+1])
          policy_action_array[idx] = policy_action
        
        self.policy_actions.append(torch.from_numpy(policy_action_array).to(torch.float32))

        # 出力(policy_redraw)
        self.policy_redraws.append(torch.tensor(list(map(float,file.readline().split()))).to(torch.float32))

        # 出力(value)
        self.values.append(torch.tensor([float(file.readline())]).to(torch.float32))


  def __len__(self):
    return len(self.inputs)
  def __getitem__(self,idx):
    return self.inputs[idx],self.policy_actions[idx],self.policy_redraws[idx],self.values[idx]
  