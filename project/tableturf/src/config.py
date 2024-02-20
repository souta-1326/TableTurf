environment = "mac"

if environment == "mac":
  from config_local import*
elif environment == "colab":
  from config_colab import*
elif environment == "abci":
  from config_abci import*