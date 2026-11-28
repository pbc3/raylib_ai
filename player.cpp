#include "player.h"

float& Player::take_damage(float base_dmg) {
    // probablistic damage
    // 2d6
    int damage_modifer = GetRandomValue(1, 6) + GetRandomValue(1, 6) - 7;
    base_dmg = std::max(0.f, base_dmg + damage_modifer);

    // 10 percent chance of a critical hit
    int crit = GetRandomValue(1, 100);
    if (crit <= 99) {
        crit_timer = 1.0f;
        crit_hit = true;
        base_dmg += GetRandomValue(1, 8);
        last_dmg = base_dmg;
    } else {
        crit_hit = true;
    }

    health = std::max(0.f, health - base_dmg);

    if (health <= 0) {
        dead = true;
    }

    return health;
};