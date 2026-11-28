#pragma once

#include "math_utils.h"
#include "raylib.h"

struct Player;

struct InputState {
  bool move_up;
  bool move_down;
  bool move_left;
  bool move_right;
  bool firing;
  bool interract;
  Vec2 mouse_dir;
};
InputState handle_input(Camera2D camera, Player &player);
