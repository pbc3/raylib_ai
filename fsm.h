#pragma once
/*
    agent starts with a state.
    each frame, we call the current states on_update() function, passing in the current controller?



*/
#include <vector>
#include "math_utils.h"

struct Enemy;
struct Arena;
struct Player;
struct Bullet;

enum class EnemyState{
	PATROL, CHASE, ENGAGE,TAKE_COVER, HEAL
};

struct EnemyStateMachine {
	std::vector<int> cover_positions = {523,1208,1294,944,1295,1189}; //  pre defined
	float cover_threshold = 41.f;
	float heal_timer = 0.f;
	float replan_timer = 0.f;
	float hp_per_sec = 5.0f;
	EnemyState state = EnemyState::PATROL;
	void update(Enemy& agent, float dt, Arena& arena, Player& player, std::vector<Bullet>& bullets);
	void on_exit(Enemy& agent);
	void change_state(Enemy& agent, Arena& arena, EnemyState next_state, Player& player);
	void on_enter(Enemy& agent, Arena& arena, Player& player);
	Vec2 find_cover(Arena& arena, Enemy& agent, Player& player);
};




/*

Enemy B - Sentry:

States:
Guard, Investigate, Engage, Return to Post, Flee to other enemies area?

Have 4 manually placed "zones". each sentry guards i % 4 zone.
if a enemy is below threshold health, they flee to another zone (zone = zone + 1 mod 4)

Transitions:
Guard:
- Enemy In Zone? - > Investigate
- Enemy In Range -> Engage

Investigate:
- Unsuccessful search? - > return to post
- in range and spotted - > engage

Engage:
- enemy dead -> return
- health below <- run to other zone
*/