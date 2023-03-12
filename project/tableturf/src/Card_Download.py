#Reference:leanny,https://raw.githubusercontent.com/Leanny/leanny.github.io/master/splat3/data/mush/300/MiniGameCardInfo.json
import requests,json
from pprint import pprint
MAX_H = 8
MAX_W = 8
URL = "https://raw.githubusercontent.com/Leanny/leanny.github.io/master/splat3/data/mush/300/MiniGameCardInfo.json"
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
  return data['Number'],h,w,card_format,data['SpecialCost']

cards_data_format = [data_format(card_data) for card_data in cards_data]
with open("Card_Database.hpp",mode='w') as f:
  print('#include "Card.hpp"',file=f)
  N_card = len(cards_data)
  print(f'constexpr int N_card = {N_card};',file=f)
  print('constexpr short Cards_Cell[N_card][8][8] = {',file=f)
  for card_data_format in cards_data_format:
    print(str(card_data_format[3]).replace('[','{').replace(']','}').replace(' ','')+",",file=f)
  print('};',file=f)
  print('constexpr Card Cards[N_card] = {',file=f)
  for i,(id,h,w,card_cell,sp_cost) in enumerate(cards_data_format):
    print(f'{"{"}{id},{h},{w},Cards_Cell[{i}],{sp_cost}{"}"},',file=f)
  print('};',file=f)


