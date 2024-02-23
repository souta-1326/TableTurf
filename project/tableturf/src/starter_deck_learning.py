import torch
from torch import nn,optim
import time
import subprocess
import sys
import math
import pickle
from tqdm import tqdm
from network import network
from network.common import*
from network import datasets
from save_and_load import*

from torch.nn.parallel import DistributedDataParallel as DDP
import torch.distributed as dist
import torch.multiprocessing as mp
from torch.utils.data import DataLoader
from torch.utils.data.distributed import DistributedSampler

from config import*

def train_mps(load_model_path:str,save_model_path:str,learning_rate):
  model = load_model(load_model_path)
  # define loss function and optimizer
  loss_fn_policy_action = nn.CrossEntropyLoss()
  loss_fn_policy_redraw = nn.CrossEntropyLoss()
  loss_fn_value = nn.MSELoss()
  optimizer = optim.SGD(model.parameters(),lr=learning_rate,weight_decay=1e-4)
  
  # create dataloader
  with open(dataset_path,"rb") as file:
    train_data = pickle.load(file)
  dataloader = DataLoader(train_data,batch_size=learning_batch_size,shuffle=True,pin_memory=True,num_workers=2)

  for _ in range(n_epochs):
    with tqdm(enumerate(dataloader),total=len(dataloader)) as pbar_loss:
      for i,(data,label_policy_action,label_policy_redraw,label_value) in pbar_loss:
        data = data.to(device)
        label_policy_action = label_policy_action.to(device)
        label_policy_redraw = label_policy_redraw.to(device)
        label_value = label_value.to(device)
        # initialize grad
        optimizer.zero_grad()
        # forward
        output_policy_action,output_policy_redraw,output_value = model(data)
        # compute loss
        loss_policy_action = loss_fn_policy_action(output_policy_action,label_policy_action)
        loss_policy_redraw = loss_fn_policy_redraw(output_policy_redraw,label_policy_redraw)
        loss_value = loss_fn_value(output_value,label_value)
        # backward
        (loss_policy_action+loss_policy_redraw+loss_value).backward()
        # update parameters
        optimizer.step()
        if i==0:
          print(f"loss_policy_action:{loss_policy_action:.3f}")
          print(f"loss_policy_redraw:{loss_policy_redraw:.3f}")
          print(f"loss_value:{loss_value:.3f}")
  
  save_model(model,save_model_path)
  

def train_cuda(load_model_path:str,save_model_path:str,gpu_id:int,learning_rate:float):
  # load model
  model = load_model(load_model_path).to(gpu_id)
  # define loss function and optimizer
  loss_fn_policy_action = nn.CrossEntropyLoss()
  loss_fn_policy_redraw = nn.CrossEntropyLoss()
  loss_fn_value = nn.MSELoss()
  optimizer = optim.SGD(model.parameters(),lr=learning_rate,weight_decay=1e-4)
  # create dataloader
  with open(dataset_path,"rb") as file:
    train_data = pickle.load(file)
  num_cpus_per_gpu = num_cpus//num_gpus
  dataloader = DataLoader(train_data,batch_size=learning_batch_size,shuffle=True,num_workers=num_cpus_per_gpu,pin_memory=True)

  scaler = torch.cuda.amp.GradScaler()
  for _ in range(n_epochs):
    with tqdm(enumerate(dataloader),total=len(dataloader)) as pbar_loss:
      for i,(data,label_policy_action,label_policy_redraw,label_value) in pbar_loss:
        data = data.to(gpu_id)
        label_policy_action = label_policy_action.to(gpu_id)
        label_policy_redraw = label_policy_redraw.to(gpu_id)
        label_value = label_value.to(gpu_id)
        # initialize grad
        optimizer.zero_grad()
        with torch.autocast(device_type=device):
          # forward
          output_policy_action,output_policy_redraw,output_value = model(data)
          # compute loss
          loss_policy_action = loss_fn_policy_action(output_policy_action,label_policy_action)
          loss_policy_redraw = loss_fn_policy_redraw(output_policy_redraw,label_policy_redraw)
          loss_value = loss_fn_value(output_value,label_value)
        # backward
        scaler.scale(loss_policy_action+loss_policy_redraw+loss_value).backward()
        # update parameters
        scaler.step(optimizer)
        # update the scaler
        scaler.update()
        if i==0:
          print(f"loss_policy_action:{loss_policy_action:.3f}")
          print(f"loss_policy_redraw:{loss_policy_redraw:.3f}")
          print(f"loss_value:{loss_value:.3f}")
  
  # update current model
  save_model(model,save_model_path)

# DistributedDataParallel
def train_DDP(rank,load_model_path:str,save_model_path:str,learning_rate):
  # create default process group
  dist.init_process_group("nccl",rank=rank,world_size=num_gpus)
  # load model
  model = load_model(load_model_path).to(rank)
  # construct DDP model
  model_DDP = DDP(model,device_ids=[rank])
  # define loss function and optimizer
  loss_fn_policy_action = nn.CrossEntropyLoss()
  loss_fn_policy_redraw = nn.CrossEntropyLoss()
  loss_fn_value = nn.MSELoss()
  optimizer = optim.SGD(model_DDP.parameters(),lr=learning_rate,weight_decay=1e-4)
  # create dataloader
  with open(dataset_path,"rb") as file:
    train_data = pickle.load(file)
  sampler = DistributedSampler(dataset=train_data,num_replicas=num_gpus,rank=rank,shuffle=True)
  dataloader = DataLoader(train_data,batch_size=learning_batch_size,sampler=sampler,num_workers=1,pin_memory=True)

  scaler = torch.cuda.amp.GradScaler()
  for epoch_id in range(n_epochs):
    sampler.set_epoch(epoch_id)
    with tqdm(enumerate(dataloader),total=len(dataloader),disable=(rank!=0)) as pbar_loss:
      for i,(data,label_policy_action,label_policy_redraw,label_value) in pbar_loss:
        data = data.to(rank)
        label_policy_action = label_policy_action.to(rank)
        label_policy_redraw = label_policy_redraw.to(rank)
        label_value = label_value.to(rank)
        # initialize grad
        optimizer.zero_grad()
        with torch.autocast(device_type=device):
          # forward
          output_policy_action,output_policy_redraw,output_value = model_DDP(data)
          # compute loss
          loss_policy_action = loss_fn_policy_action(output_policy_action,label_policy_action)
          loss_policy_redraw = loss_fn_policy_redraw(output_policy_redraw,label_policy_redraw)
          loss_value = loss_fn_value(output_value,label_value)
        # backward
        scaler.scale(loss_policy_action+loss_policy_redraw+loss_value).backward()
        # update parameters
        scaler.step(optimizer)
        # update the scaler
        scaler.update()
        if rank==0 and i==0:
          print(f"loss_policy_action:{loss_policy_action:.3f}")
          print(f"loss_policy_redraw:{loss_policy_redraw:.3f}")
          print(f"loss_value:{loss_value:.3f}")
  
  dist.barrier()
  # update current model
  if rank == 0:
    # model_DDP.moduleとしないとバグるらしい
    save_model(model_DDP.module,save_model_path)

def main(current_model_path:str):
  create_new_files(current_model_path)
  with open(dataset_path,"rb") as file:
    train_data = pickle.load(file)
  with open(step_count_path,"rb") as file:
    step_count = pickle.load(file)

  # training loop
  for i in range(num_games_overall//num_games_in_selfplay):
    # C++用のモデル(policyとvalueが一体化)を作成
    create_model_cpp(current_model_path)

    # testplay & selfplay を開始
    num_cpus_for_selfplay = num_cpus*(num_gpus-1)//num_gpus
    num_gpus_for_selfplay = num_gpus-1
    command = f"{starter_deck_selfplay_program} {num_cpus_for_selfplay} {num_gpus_for_selfplay} {device} {num_games_in_parallel} {num_games_in_selfplay} {num_games_in_testplay} {buffer_size} {PV_ISMCTS_num_simulations} {simple_ISMCTS_num_simulations} {diff_bonus} {dirichlet_alpha} {eps} {model_cpp_path} {data_path} {log_path}"
    print(command)
    proc = subprocess.Popen(command,shell=True)

    # その間に1つのGPUで学習
    print("dataset size:",len(train_data))
    print("step count:",step_count)
    learning_rate = get_learning_rate(step_count)

    # update network
    if len(train_data) == 0:
      print("train_data is empty, so skip learning")
    else:
      start_learn_time = time.time()
      save_model_path = generate_model_path()
      if device=="cuda":
        # mp.spawn(train_DDP,args=(current_model_path,save_model_path,learning_rate),nprocs=num_gpus,join=True)
        learning_gpu_id = num_gpus-1
        train_cuda(current_model_path,save_model_path,learning_gpu_id,learning_rate)
        torch.cuda.synchronize()
      else:
        train_mps(current_model_path,save_model_path,learning_rate)
        torch.mps.synchronize()
      end_learn_time = time.time()

      current_model_path = save_model_path
      step_count += n_epochs*math.ceil(len(train_data)/learning_batch_size)
      with open(step_count_path,"wb") as file:
        pickle.dump(step_count,file)
      with open(log_path,"a") as log_f:
        print(f"learn time: {end_learn_time-start_learn_time:.3f}",file=log_f)
    
    # selfplay が終了
    proc.wait()
    assert proc.returncode == 0
    print("selfplay done")

    # train_dataに selfplay で得られたデータを追加
    train_data.add(data_path,buffer_size)
    with open(dataset_path,"wb") as file:
      pickle.dump(train_data,file)

  
if __name__ == "__main__":
  args = sys.argv
  current_model_path = generate_model_path()
  if len(args) >= 2:
    current_model_path = args[1]
  main(current_model_path)