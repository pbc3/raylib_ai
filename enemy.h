#pragma once
#include "math_utils.h"
#include <vector>
#include "arena.h"
#include "pathfinding.h"
#include <algorithm>

#include "entity.h"


struct Enemy : public Entity{
  // Vec2 world_pos;
  Vec2 velocity;
  float threshold = .2f;
  float health = 100;
  // float size;
  int player_tile;
  std::vector<Vec2> cur_path;
  int cur_waypoint;
  void follow_path(float dt, Arena& arena, Vec2 player_pos);
  void update(float dt);
  void replan(Arena& arena, Vec2 player_pos);
  float& take_damage(float dmg);
};

