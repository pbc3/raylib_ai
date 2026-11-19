#pragma once
#include "math.h"
#include <vector>


struct Enemy{
  Vec2 world_pos;
  Vec2 velocity;
  float threshold = 2.0f;
  float size;
  int player_tile;
  std::vector<Vec2> cur_path;
  int cur_waypoint;
  void follow_path(float dt);
};

