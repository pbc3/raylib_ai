#include "player.h"


// void Player::update(float dt, Arena& input_state input){
 
// }

 float& Player::take_damage(float dmg){
    health -= dmg;
    if(dmg <= 0){
        dead = true;
    }
    return health;
 };