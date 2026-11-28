#pragma once
#include "math_utils.h"
#include <iostream>

enum class Team { PLAYER, ENEMY };
struct Entity {
    Team team;
    Vec2 world_pos;
    float size;
    float health;
    bool dead;
    // virtual void update(float dt) {};
    virtual float& take_damage(float dmg);
};