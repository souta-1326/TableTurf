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
  std::vector<Board<stage>> board_log = Battle<stage>(agent_P1,agent_P2,deck_P1,deck_P2);


  Scene::SetBackground(Color{57,48,131});
  int current_turn = 0;
  Visualizer_Stage<stage>::set_board(board_log[current_turn]);
  while(System::Update()){
    Visualizer_Stage<stage>::visualize();
    if(KeyRight.down()) current_turn = std::min(current_turn+1,Board<stage>::TURN_MAX);
    if(KeyLeft.down()) current_turn = std::max(current_turn-1,0);
    Visualizer_Stage<stage>::set_board(board_log[current_turn]); 
  }
}
void test3(){
  using stage = Main_Street;
  AI_ISMCTS<stage> agent_P1(20000,0.001),agent_P2(20000,0.001);
  Deck deck_P1 = test_deckset[0],deck_P2({38, 200, 183, 20, 72, 227, 188, 218, 170, 98, 120, 43, 29, 54, 165 });
  std::vector<Board<stage>> board_log = Battle<stage>(agent_P1,agent_P2,deck_P1,deck_P2);


  Scene::SetBackground(Color{57,48,131});
  int current_turn = 0;
  Visualizer_Stage<stage>::set_board(board_log[current_turn]);
  while(System::Update()){
    Visualizer_Stage<stage>::visualize();
    if(KeyRight.down()) current_turn = std::min(current_turn+1,Board<stage>::TURN_MAX);
    if(KeyLeft.down()) current_turn = std::max(current_turn-1,0);
    Visualizer_Stage<stage>::set_board(board_log[current_turn]); 
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
  AI_ISMCTS<stage> agent(20000,0.001);
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
      std::vector<int> next_cards = Visualizer_Deck::visualize(my_deck);
      if(next_cards.size() == 1){
        int next_card = next_cards[0];
        Visualizer_Deck::reset_is_clicked();
        //next_cardが山札のカードなのか確かめる
        std::vector<int> my_deck_stock = my_deck.get_stock();
        if(std::count(my_deck_stock.begin(),my_deck_stock.end(),next_card) == 1){
          my_deck.choose_card_by_card_id(board_P1.used_cards_P1[current_turn-1],next_card);
          current_phase = Battle_Phase::think_action;
        }
        else{
          std::cout << "INVALID: the card is not in stock" << std::endl;
        }
      }
    }
    else if(current_phase == Battle_Phase::get_opponent_card){
      std::vector<int> opponent_cards = Visualizer_Deck::visualize(opponent_deck);
      if(opponent_cards.size() == 1){
        int opponent_card = opponent_cards[0];
        Visualizer_Deck::reset_is_clicked();
        //opponent_cardがまだ使われていないカードなのか確かめる
        std::vector<int> opponent_used_cards = board_P1.used_cards_P2;
        if(std::count(opponent_used_cards.begin(),opponent_used_cards.end(),opponent_card) == 0){
          Visualizer_Stage<stage>::set_P2_card(opponent_card);
          current_phase = Battle_Phase::get_opponent_action;
        }
        else{
          std::cout << "INVALID: the card is already used by opponent" << std::endl;
        }
      }
    }
    else if(current_phase == Battle_Phase::get_opponent_action){
      auto both_choice = Visualizer_Stage<stage>::visualize(2);
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
      std::cout << "CONTROL GAME SCREEN: put card like this then press O & K" << std::endl;
      Visualizer_Stage<stage>::set_P1_hand(my_action);
      current_phase = Battle_Phase::wait_putting_card;
      Visualizer_Stage<stage>::visualize(3);
    }
    else if(current_phase == Battle_Phase::wait_putting_card){
      Visualizer_Stage<stage>::visualize(3);
      if(KeyO.down() && KeyK.down()){
        std::cout << "input opponent card" << std::endl;
        current_phase = Battle_Phase::get_opponent_card;
      }
    }
    else if(current_phase == Battle_Phase::end){
      break;
    }
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
  Initializer initializer;
  while(true){
    battle_on_splatoon3();
  }
}
