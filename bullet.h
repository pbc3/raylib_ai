#pragma once
// #include "entity.h"


struct Bullet : public Entity {
    // Vec2 world_pos;
    Vec2 velocity;
    float size = 2.0f;
    bool expired;
    Team team;
    float bullet_speed = 200.f;
  };