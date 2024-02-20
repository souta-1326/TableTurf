#pragma once
class Card{
public:
  static constexpr int MAX_H = 8;
  static constexpr int MAX_W = 8;
  const int H,W,N_SQUARE,SP_COST;
  //時計回りにN度回転した時のマス (左上から詰めて)
  bool R0[MAX_H][MAX_W]={},R90[MAX_W][MAX_H]={},R180[MAX_H][MAX_W]={},R270[MAX_W][MAX_H]={};
  //スペシャルマスの位置(ない場合は共に-1)
  int R0_SP_H,R0_SP_W,R90_SP_H,R90_SP_W,R180_SP_H,R180_SP_W,R270_SP_H,R270_SP_W;
  constexpr Card(const int h,const int w,const int n_square,const short r0[MAX_H][MAX_W],const int sp_cost);
  //特別なカード(SP_COST==3,N_SQUARE==13,SPマスなし)
  constexpr bool is_special_card() const {return R0_SP_H == -1;};
};
constexpr Card::Card(const int h,const int w,const int n_square,const short r0[MAX_H][MAX_W],const int sp_cost):H(h),W(w),N_SQUARE(n_square),SP_COST(sp_cost),
R0_SP_H(-1),R0_SP_W(-1),R90_SP_H(-1),R90_SP_W(-1),R180_SP_H(-1),R180_SP_W(-1),R270_SP_H(-1),R270_SP_W(-1){
  R0_SP_H = R0_SP_W = R90_SP_H = R90_SP_W = R180_SP_H = R180_SP_W = R270_SP_H = R270_SP_W = -1;
  for(int i=0;i<h;i++){
    for(int j=0;j<w;j++){
      R0[i][j] = r0[i][j];
      R90[j][H-1-i] = r0[i][j];
      R180[H-1-i][W-1-j] = r0[i][j];
      R270[W-1-j][i] = r0[i][j];
      if(r0[i][j] == 2){
        R0_SP_H = i;
        R0_SP_W = j;
        R90_SP_H = j;
        R90_SP_W = H-1-i;
        R180_SP_H = H-1-i;
        R180_SP_W = W-1-j;
        R270_SP_H = W-1-j;
        R270_SP_W = i;
      }
    }
  }
}