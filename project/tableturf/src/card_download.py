#Reference:Leanny,https://github.com/Leanny/splat3/blob/main/data/mush/800/MiniGameCardInfo.json
#TODO:MIT Licenseに対応した正しい著作者表示をする
import requests,json
from pprint import pprint
MAX_H = 8
MAX_W = 8
URL = "https://raw.githubusercontent.com/Leanny/splat3/master/data/mush/800/MiniGameCardInfo.json"
text = requests.get(URL).text
cards_data = json.loads(text)
def data_format(data:dict):
  #Empty:0 Fill:1 Special:2
  card_format = [[0]*MAX_W for _ in range(MAX_H)]
  for i in range(MAX_H):
    for j in range(MAX_W):
      card_format[i][j] = ["Empty","Fill","Special"].index(data['Square'][(MAX_H-1-i)*MAX_W+j])
  #寄せられる縦と横のマス数を計測
  up_h,down_h,left_w,right_w = MAX_H,0,MAX_W,0
  for i in range(MAX_H):
    for j in range(MAX_W):
      if card_format[i][j] >= 1:
        up_h = min(up_h,i)
        down_h = max(down_h,i)
        left_w = min(left_w,j)
        right_w = max(right_w,j)
  h = down_h-up_h+1
  w = right_w-left_w+1
  #左上に寄せる
  for i in range(MAX_H):
    for j in range(MAX_W):
      card_format[i][j] = card_format[i+up_h][j+left_w] if (i+up_h < MAX_H and j+left_w < MAX_W) else 0
  return data['Number'],h,w,data['Square'].count('Fill')+data['Square'].count('Special'),card_format,data['SpecialCost']
#先頭にダミーを加える(0-indexed to 1-indexed)
cards_data_format = [(0,0,0,0,[[0]*8 for _ in range(8)],0)]+[data_format(card_data) for card_data in cards_data]
cards_data_format.sort()
with open("card_database.hpp",mode='w') as f:
  print('#pragma once',file=f)
  print('#include "card.hpp"',file=f)
  N_card = len(cards_data)
  print(f'constexpr int N_card = {N_card};',file=f)
  print('constexpr short cards_cell[N_card+1][Card::MAX_H][Card::MAX_W] = {',file=f)
  #リストの[]を{}に変更し、空白を無くす
  for id,h,w,N_square,card_cell,SP_cost in cards_data_format:
    print(str(card_cell).replace('[','{').replace(']','}').replace(' ','')+",",file=f)
  print('};',file=f)
  print('constexpr Card cards[N_card+1] = {',file=f)
  for i,(id,h,w,N_square,card_cell,SP_cost) in enumerate(cards_data_format):
    print(f'{"{"}{h},{w},{N_square},cards_cell[{i}],{SP_cost}{"}"},',file=f)
  print('};',file=f)

with open("network/common.py",mode='w') as f:
  N_card = len(cards_data)
  print(f'N_CARD = {N_card}',file=f)

