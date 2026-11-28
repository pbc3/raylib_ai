#pragma once
/*
    agent starts with a state.
    each frame, we call the current states on_update() function, passing in the
   current controller?



*/
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
  std::vector<int> cover_positions = {523, 1208, 1294, 944, 1295, 1189};
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

/*

Enemy B - Sentry:

States:
Guard, Investigate, Engage, Return to Post, Flee to zone +1, heal in it, return
to original post, Heal

Have 4 manually placed "zones". each sentry guards i % 4 zone.
if a enemy is below threshold health, they flee to another zone (zone = zone + 1
mod 4)

Transitions:
Guard:
- Enemy In Zone? - > Investigate
- Enemy In Range -> Engage
- Health < threshold ? -> flee to zone

Investigate:
- Unsuccessful search? - > return to post
- in range and spotted - > engage
- Health < threshold ? -> flee to zone

Engage:
- enemy dead -> return
- health below <- run to other zone

Return to Post:
 - arrived at post -> guard
 - enemy spotted on the way -> ignore and return
 - health < thresh - flee to other zone

 Flee To Post+1%Size:
 - arrivied at post -> heal


 Heal:
 - back to 80+ health - > return to post
 - enemy spotted - flee to zone + 1


additional support structures & functionaility needed:
std::array<int,3> guard_zone = {tile, tile, tile}; - the zones, which i will
define manually. 3 corners of the map basically int default_zone; - the zone
given at spawn time, doesnt change. one from the 3 above. a slow 360* fov spin
upon arriving at the tile to investigate. for "flee to zone" i will just use
replan on enter of that state, then each tick check movement result from follow
path.

*/