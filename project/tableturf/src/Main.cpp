#include <Siv3D.hpp>
#include "card_database.hpp"
void Main(){
  while(System::Update()){
    Circle{ 400, 300, 20 }.draw();
  }
}