#pragma once
#include "entity.h"
#include <vector>


struct Bullet : public Entity {
    // Vec2 world_pos;
    Vec2 velocity;
    float size = 2.0f;
    bool expired;
    Team team;
    float bullet_speed = 200.f;
  };

void spawn_bullet(std::vector<Bullet>& bullets, Vec2 spawn_pos, Vec2 direction, Team team);