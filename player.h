#pragma once
#include "math_utils.h"



struct Player {
  Vec2 world_pos;
  Vec2 velocity;
  float size;
  float health = 100;
};