#pragma once
#include "math_utils.h"
#include <vector>
#include "arena.h"
#include "pathfinding.h"
#include <algorithm>
#include <chrono>
#include "entity.h"
#include "fsm.h"
#include "player.h"
#include <numbers>
#include "bullet.h"

struct Enemy;

struct Perceptions{
  Vec2 last_seen_player;
  bool spotted_player;
  bool in_range;
  float view_angle_rads = 1.5708f; // 90* in radians
  float max_distance = 250.f;
  void update(Enemy* agent, Arena& arena, Player& player);
};

struct Enemy : public Entity{
  
  Vec2 velocity;
  float threshold = .2f;
  int goal_tile = -1;
  std::chrono::steady_clock::time_point last_shot_at;
  std::vector<Vec2> cur_path;
  int cur_waypoint = 0;

  EnemyStateMachine fsm;
  Perceptions perceptions;
  Vec2 forward;
  Vec2 player_pos;
	std::vector<Vec2> patrol_targets = {{150,300},{200,300},{400,200}};
  int current_patrol_idx;

  void update(Arena& arena, Player& player, float dt, std::vector<Bullet>& bullets);
  void follow_path(float dt, Arena& arena);
  void replan(Arena& arena, Vec2 player_pos);
  float& take_damage(float dmg);
  void engage_player(Arena& arena, float dt, Player& player, std::vector<Bullet>& bullets);
  void patrol(Arena& arena, float dt);
};


