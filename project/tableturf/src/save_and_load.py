import torch
from torch import nn
from config import*
import glob
import subprocess
from network import network
from network.common import*
from network import datasets
import pickle

def save_model(model:nn.Module,save_model_path):
  model.eval()
  torch.save(model.state_dict(),save_model_path)

  # Google Driveに保存
  if save_drive_dir_path != None:
    torch.save(model.state_dict(),save_drive_dir_path+save_model_path)

def load_model():
  model = network.AlphaZeroResNet(H,W,n_blocks=n_blocks).to(device)
  model.load_state_dict(torch.load(model_path,map_location=device))
  return model

def create_model_cpp(model:nn.Module):
  model_cpp = network.AlphaZeroResNet_CPP(model).to(device)
  dummy_input = torch.rand(2,network.INPUT_C,H,W).to(device)
  traced_model_cpp = torch.jit.trace(model_cpp,dummy_input)
  traced_model_cpp.save(model_cpp_path)

def create_new_files():
  #フォルダ作成
  subprocess.run("mkdir model data log",shell=True)
  # model_pathが存在しないなら、新たにmodelを用意
  if not glob.glob(model_path):
    model = network.AlphaZeroResNet(H,W,n_blocks).to(device)
    save_model(model,model_path,None)
  
  # data_pathが存在しないなら、新たに用意
  if not glob.glob(data_path):
    with open(data_path,"w") as file:
      #データの個数を0とおく
      file.write("0\n")
  
  # log_pathが存在しないなら、新たに用意
  if not glob.glob(log_path):
    with open(log_path,"w") as file:
      file.write("")

  # dataset_pathが存在しないなら、新たに用意
  if not glob.glob(dataset_path):
    empty_dataset = datasets.Datasets()
    with open(dataset_path,"wb") as file:
      pickle.dump(empty_dataset,file)
  
  # step_count_pathが存在しないなら、新たに用意
  if not glob.glob(step_count_path):
    step_count = 0
    with open(step_count_path,"wb") as file:
      pickle.dump(step_count,file)