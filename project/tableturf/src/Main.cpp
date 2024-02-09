//#define NDEBUG //コメントを外すとassertチェックされない
#include <Siv3D.hpp>
#include <algorithm>
#include "board.hpp"
#include "siv3d/visualizer_stage.hpp"
#include "siv3d/visualizer_deck.hpp"
#include "battle.hpp"
#include "AI_random.hpp"
#include "AI_ISMCTS.hpp"
#include "deck_database.hpp"
void test1(){
  Board<Thunder_Point> board;
  Visualizer_Stage<Thunder_Point>::set_board(board);
  Scene::SetBackground(Color{57,48,131});
  Choice<Thunder_Point> 
  P1[12] = 
  {{196,1,11,4,0,0},{1,2,9,0,0,0},{14,2,11,7,0,0},{10,2,12,7,0,0},
   {148,1,13,0,0,0},{102,1,12,13,0,0},{185,3,7,13,0,1},{146,3,13,1,0,0},
   {23,1,1,12,0,0},{52,2,17,3,0,0},{35,3,9,0,0,0},{72,2,3,8,0,1}},
  P2[12] = 
  {{158,2,4,9,0,0},{148,0,8,3,0,0},{55,3,9,11,0,0},{135,3,7,1,0,0},
   {145,0,10,9,0,0},{146,2,7,3,0,0},{61,0,6,10,0,0},{72,0,11,2,0,1},
   {42,1,0,13,0,0},{23,3,0,8,0,0},{102,1,0,10,0,0},{37,0,11,11,0,1}
  };
  // board.put_both_cards_without_validation(196,1,11,4,0,0,158,2,4,9,0,0);
  // board.put_both_cards_without_validation(1,2,9,0,0,0,148,0,8,3,0,0);
  // board.put_both_cards_without_validation(14,2,11,7,0,0,55,3,9,11,0,0);
  // board.put_both_cards_without_validation(10,2,12,7,0,0,135,3,7,1,0,0);
  // board.put_both_cards_without_validation(148,1,13,0,0,0,145,0,10,9,0,0);
  // board.put_both_cards_without_validation(102,1,12,13,0,0,146,2,7,3,0,0);
  // board.put_both_cards_without_validation(185,3,7,13,0,1,61,0,6,10,0,0);
  // board.put_both_cards_without_validation(146,3,13,1,0,0,72,0,11,2,0,1);
  // board.put_both_cards_without_validation(23,1,1,12,0,0,42,1,0,13,0,0);
  // board.put_both_cards_without_validation(52,2,17,3,0,0,23,3,0,8,0,0);
  // board.put_both_cards_without_validation(35,3,9,0,0,0,102,1,0,10,0,0);
  // board.put_both_cards_without_validation(72,2,3,8,0,1,37,0,11,11,0,1);
  for(int turn=0;turn<12;turn++){
    // board.put_both_cards_without_validation(P1[turn],P2[turn]);
    board.put_both_cards_without_validation(P2[turn].swap_player(),P1[turn].swap_player());
  }
  while(System::Update()){
    Visualizer_Stage<Thunder_Point>::visualize();
  }
}
int read_turn(){
  int turn;
  std::cin >> turn;
  return turn;
}
void test2(){
  using stage = Main_Street;
  AI_random<stage> agent_P1,agent_P2;
  Deck deck_P1 = starter_deck,deck_P2 = starter_deck;
  std::vector<Board<stage>> board = Battle<stage>(agent_P1,agent_P2,deck_P1,deck_P2);


  Scene::SetBackground(Color{57,48,131});
  int current_turn = 0;
  AsyncTask<int> task_input;
  task_input = Async(read_turn);
  Visualizer_Stage<stage>::set_board(board[current_turn]);
  while(System::Update()){
    Visualizer_Stage<stage>::visualize();
    if(task_input.isReady()){
      current_turn = task_input.get();
      Visualizer_Stage<stage>::set_board(board[current_turn]);
      task_input = Async(read_turn);
    }
  }
}
void test3(){
  using stage = Thunder_Point;
  AI_ISMCTS<stage> agent_P1(5000),agent_P2(5000);
  Deck deck_P1 = starter_deck,deck_P2 = starter_deck;
  std::vector<Board<stage>> board = Battle<stage>(agent_P1,agent_P2,deck_P1,deck_P2);

  Scene::SetBackground(Color{57,48,131});
  int current_turn = 0;
  AsyncTask<int> task_input;
  task_input = Async(read_turn);
  Visualizer_Stage<stage>::set_board(board[current_turn]);
  while(System::Update()){
    Visualizer_Stage<stage>::visualize();
    if(task_input.isReady()){
      current_turn = task_input.get();
      Visualizer_Stage<stage>::set_board(board[current_turn]);
      task_input = Async(read_turn);
    }
  }
}
enum class Battle_Phase{
  start,
  get_first_hand,
  get_next_hand,
  get_opponent_card,
  get_opponent_action,
  think_do_redraw,
  think_action,
  wait_putting_card,
  end
};
void battle_on_splatoon3(){
  using stage = Main_Street;
  //自分と相手のデッキを入力
  Deck my_deck = starter_deck;
  Deck opponent_deck = starter_deck;

  Scene::SetBackground(Color{57,48,131});
  Board<stage> board_P1,board_P2;
  Visualizer_Stage<stage>::set_board(board_P1);

  //AIを起動
  AI_ISMCTS<stage> agent(10000);
  agent.set_deck_P1(my_deck);
  agent.set_deck_P2(opponent_deck);

  Battle_Phase current_phase = Battle_Phase::start;
  int current_turn = 0;
  while(System::Update()){
    if(current_phase == Battle_Phase::start){
      std::cout << "Click your hand" << std::endl;
      current_phase = Battle_Phase::get_first_hand;
    }
    else if(current_phase == Battle_Phase::get_first_hand){
      std::vector<int> hand = Visualizer_Deck::visualize(my_deck);
      //4枚指定されたら、my_deckの手札を設定
      if(hand.size() == Deck::N_CARD_IN_HAND){
        Visualizer_Deck::reset_is_clicked();
        my_deck.reset(hand);
        //redraw
        if(current_turn == 0){
          current_phase = Battle_Phase::think_do_redraw;
        }
        else{
          current_phase = Battle_Phase::think_action;
        }
      }
    }
    else if(current_phase == Battle_Phase::get_next_hand){
      std::vector<int> next_card = Visualizer_Deck::visualize(my_deck);
      if(next_card.size() == 1){
        Visualizer_Deck::reset_is_clicked();
        my_deck.choose_card_by_card_id(board_P1.used_cards_P1[current_turn-1],next_card[0]);
        current_phase = Battle_Phase::think_action;
      }
    }
    else if(current_phase == Battle_Phase::get_opponent_card){
      std::vector<int> opponent_card = Visualizer_Deck::visualize(opponent_deck);
      if(opponent_card.size() == 1){
        Visualizer_Deck::reset_is_clicked();
        Visualizer_Stage<stage>::set_P2_card(opponent_card[0]);
        current_phase = Battle_Phase::get_opponent_action;
      }
    }
    else if(current_phase == Battle_Phase::get_opponent_action){
      auto both_choice = Visualizer_Stage<stage>::visualize();
      //OKボタンが押されたら、つまりboard.current_turnが進んだらget_next_handに移行
      if(current_turn+1 == board_P1.current_turn){
        current_turn++;
        auto [choice_P1,choice_P2] = both_choice.value();

        //board_P2も進める(board_P1はvisualizerによって既に進められている)
        assert(board_P2.is_valid_placement(true,choice_P2.swap_player(),0));
        assert(board_P2.is_valid_placement(false,choice_P1.swap_player(),0));
        board_P2.put_both_cards_without_validation(choice_P2.swap_player(),choice_P1.swap_player());

        //12ターン経過したら終わり、そうでなければ追加された手札を取得
        if(current_turn > Board<stage>::TURN_MAX) current_phase = Battle_Phase::end;
        else{
          std::cout << "Click your next hand" << std::endl;
          current_phase = Battle_Phase::get_next_hand;
        }
      }
    }
    else if(current_phase == Battle_Phase::think_do_redraw){
      std::cout << "Now thinking..." << std::endl;
      bool do_redraw = agent.redraw(my_deck);
      std::cout << "CONTROL GAME SCREEN: " << (do_redraw ? "redraw":"DO NOT redraw") << std::endl;
      if(do_redraw){
        current_turn++;
        //再び手札を入力する
        std::cout << "Click your hand" << std::endl;
        current_phase = Battle_Phase::get_first_hand;
      }
      else{
        current_turn++;
        current_phase = Battle_Phase::think_action;
      }
    }
    else if(current_phase == Battle_Phase::think_action){
      std::cout << "Now thinking..." << std::endl;
      Choice my_action = agent.get_action(board_P1,board_P2,my_deck);
      std::cout << "CONTROL GAME SCREEN: put card like this then input OK to console" << std::endl;
      Visualizer_Stage<stage>::set_P1_hand(my_action);
      current_phase = Battle_Phase::wait_putting_card;
      Visualizer_Stage<stage>::visualize(2);
    }
    else if(current_phase == Battle_Phase::wait_putting_card){
      Visualizer_Stage<stage>::visualize(2);
      std::string console_input;
      std::cin >> console_input;
      if(console_input == "OK" || console_input == "ok"){
        std::cout << "input opponent card" << std::endl;
        current_phase = Battle_Phase::get_opponent_card;
      }
    }
  }

  //0ターン目(カードを引き直すか)
  //カードのIDを入力する
  std::cout << "Click your hand" << std::endl;

  for(int i=0;i<4;i++){

  }
}
void test4(){
  Deck deck = starter_deck;
  Scene::SetBackground(Color{57,48,131});
  while(System::Update()){
    Visualizer_Deck::visualize(starter_deck);
  }
}
void Main(){
  battle_on_splatoon3();
}
