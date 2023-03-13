class Card{
public:
  static constexpr int MAX_H = 8;
  static constexpr int MAX_W = 8;
  constexpr Card(const int h,const int w,const int n_square,const short r0[MAX_H][MAX_W],const int sp_cost):H(h),W(w),N_square(n_square),SP_cost(sp_cost){
    SP_H = SP_W = -1;
    for(int i=0;i<h;i++){
      for(int j=0;j<w;j++){
        R0[i][j] = r0[i][j];
        R90[j][H-1-i] = r0[i][j];
        R180[H-1-i][W-1-j] = r0[i][j];
        R270[W-1-j][i] = r0[i][j];
        if(r0[i][j] == 2){
          SP_H = i;
          SP_W = j;
        }
      }
    }
  }
private:
  const int H,W,N_square,SP_cost;
  //時計回りにN度回転した時のマス (左上から詰めて)
  bool R0[MAX_H][MAX_W]={},R90[MAX_W][MAX_H]={},R180[MAX_H][MAX_W]={},R270[MAX_W][MAX_H]={};
  //スペシャルマスの位置(ない場合は共に-1)
  int SP_H,SP_W;
};