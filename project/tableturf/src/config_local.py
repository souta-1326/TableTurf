import os

num_cpus = 2
num_gpus = 1
device = "mps"

n_blocks = 19

model_cpp_path = "model/model_cpp.pt"
save_drive_dir_path = None #末尾は'/'
data_path = "data/selfplay_data.txt"
dataset_path = "data/dataset.pickle"
step_count_path = "data/step_count.pickle"
log_path = "log/log.txt"

starter_deck_selfplay_program = "build/starter_deck_selfplay"

#for DDP
os.environ["MASTER_ADDR"] = "localhost"
os.environ["MASTER_PORT"] = "50000"

PV_ISMCTS_num_simulations = 800
simple_ISMCTS_num_simulations = 5000
dirichlet_alpha = 0.15
eps = 0.25
diff_bonus = 0.001

num_games_overall = 320*30
num_games_in_selfplay = 320
num_games_in_parallel = 2
num_games_in_testplay = 16
buffer_size = 500000

learning_batch_size = 128
n_epochs = 5
lr_schedule = [(0,0.02),(300000,0.002),(500000,0.0002)]
def get_learning_rate(step_count:int):
  learning_rate = -1
  for schedule_step_count,schedule_learning_rate in lr_schedule:
    if step_count >= schedule_step_count:
      learning_rate = schedule_learning_rate
    else:
      break
  assert learning_rate != -1
  print(f"learning rate: {learning_rate}")
  return learning_rate
