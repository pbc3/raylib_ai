#pragma once
#include "math_utils.h"
#include <vector>

struct Enemy;
struct Arena;
struct Player;
struct Bullet;

enum class EnemyState {
  PATROL,
  CHASE,
  TAKE_COVER,
  HEAL,
  ENGAGE,
  GUARD,
  INVESTIGATE,
  FLEE,
  RETURN_TO_POST
};

struct EnemyStateMachine {
  std::vector<int> cover_positions = {666, 84, 219, 230, 442, 505, 549, 813, 995, 1135};
  float cover_threshold = 41.f;
  float rotation_speed = 1.0f;
  float heal_timer = 0.f;
  float replan_timer = 0.f;
  float heal_rate = 20.0f;
  float hp_per_sec = 5.0f;
  
  EnemyState state = EnemyState::PATROL;
  void update_enemy_A(Enemy &agent, float dt, Arena &arena, Player &player,
                      std::vector<Bullet> &bullets);
  void update_enemy_B(Enemy &agent, float dt, Arena &arena, Player &player,
                      std::vector<Bullet> &bullets);
  void on_exit(Enemy &agent);
  void change_state(Enemy &agent, Arena &arena, EnemyState next_state,
                    Player &player);
  void on_enter(Enemy &agent, Arena &arena, Player &player);
  Vec2 find_cover(Arena &arena, Enemy &agent, Player &player);
};

