#include "player.h"



 float& Player::take_damage(float base_dmg){
 

    // probablistic damage 
    // 2d6
    int damage_modifer =  GetRandomValue(1,6) + GetRandomValue(1,6) - 7;
    base_dmg = std::max(0.f,base_dmg + damage_modifer);
    health = std::max(0.f,health-base_dmg);

    if(health <= 0){
        dead = true;
    }
    return health;
 };