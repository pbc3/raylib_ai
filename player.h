#pragma once
#include "entity.h"
#include "input.h"
#include "math_utils.h"
#include "raylib.h"
#include <algorithm>

struct Arena;

struct Player : public Entity {
  // Vec2 world_pos;
  Vec2 velocity;
  // float size;
  // float health = 100;
  bool crit_hit = false;
  float crit_timer = 0.f;
  float last_dmg;
  float &take_damage(float dmg);
};

void update_player(Arena &arena, Player &player, InputState input, float dt);