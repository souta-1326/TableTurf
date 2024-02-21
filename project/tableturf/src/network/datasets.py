import torch
import numpy as np
from .common import*
from .network import*

class Datasets(torch.utils.data.Dataset):
  def __init__(self):
    self.inputs_first5 = []
    self.inputs_is_filled_all1 = []
    self.policy_actions_index_policy_pair = []
    self.policy_redraws = []
    self.values = []
  def add(self,data_path,buffer_size):
    with open(data_path,"r") as file:
      N_file_data = int(file.readline()) # ファイル内データの個数
      N_trashed_data = max(0,len(self)+N_file_data-buffer_size)
      # buffer_sizeを超えた要素を削除
      del self.inputs_first5[:N_trashed_data]
      del self.inputs_is_filled_all1[:N_trashed_data]
      del self.policy_actions_index_policy_pair[:N_trashed_data]
      del self.policy_redraws[:N_trashed_data]
      del self.values[:N_trashed_data]

      for _ in range(N_file_data):
        # input
        # 0~5(盤面)
        input_first5 = []
        for _ in range(5):
          s = file.readline()
          input_first5.append([list(map(float,s[i*W:(i+1)*W])) for i in range(H)])
        self.inputs_first5.append(input_first5)

        # 6~N_CARD*2+40(その他)
        N_channel_all1 = int(file.readline()) # 1で埋められるチャネル数
        channel_indexes_all1 = list(map(int,file.readline().split()))
        assert N_channel_all1 == len(channel_indexes_all1)
        input_is_filled_all1 = [False]*INPUT_C
        for channel_index in channel_indexes_all1:
          input_is_filled_all1[channel_index] = True

        self.inputs_is_filled_all1.append(input_is_filled_all1)
        
        # for channel_index in range(5,INPUT_C):
        #   input_list.append(np.ones((H,W)) if is_filled_all1[channel_index] else np.zeros((H,W)))
        
        # self.inputs.append(torch.from_numpy(np.array(input_list)).to(torch.float32))

        # 出力(policy_action)
        N_positive_policy_actions = int(file.readline())
        index_policy_action_pairs = file.readline().split()
        assert N_positive_policy_actions*2 == len(index_policy_action_pairs)

        # policy_action_array = np.zeros(N_CARD*ACTION_SPACE_OF_EACH_CARD)
        policy_action_index_policy_pair = []
        for i in range(N_positive_policy_actions):
          idx = int(index_policy_action_pairs[i*2])
          policy_action = float(index_policy_action_pairs[i*2+1])
          policy_action_index_policy_pair.append((idx,policy_action))

        self.policy_actions_index_policy_pair.append(policy_action_index_policy_pair)
        
        # self.policy_actions.append(torch.from_numpy(policy_action_array).to(torch.float32))

        # 出力(policy_redraw)
        self.policy_redraws.append(torch.tensor(list(map(float,file.readline().split()))).to(torch.float32))

        # 出力(value)
        self.values.append(torch.tensor([float(file.readline())]).to(torch.float32))


  def __len__(self):
    return len(self.values)
  def __getitem__(self,idx):
    # input を復元
    input_array = np.array(
      self.inputs_first5[idx]+
      [np.ones((H,W)) if self.inputs_is_filled_all1[idx][i] else np.zeros((H,W)) for i in range(5,INPUT_C)])
    # policy_action を復元
    policy_action_array = np.zeros(N_CARD*ACTION_SPACE_OF_EACH_CARD)
    for policy_index,policy in self.policy_actions_index_policy_pair[idx]:
      policy_action_array[policy_index] = policy

    return torch.from_numpy(input_array).to(torch.float32),torch.from_numpy(policy_action_array).to(torch.float32),self.policy_redraws[idx],self.values[idx]