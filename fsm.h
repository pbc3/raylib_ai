#pragma once
/*
    agent starts with a state.
    each frame, we call the current states on_update() function, passing in the current controller?



*/
#include <vector>

struct Enemy;
struct Arena;
struct Player;
struct Bullet;

enum class EnemyState{
	PATROL, CHASE, ENGAGE
};

struct EnemyStateMachine {
	EnemyState state = EnemyState::PATROL;
	void update(Enemy& agent, float dt, Arena& arena, Player& player, std::vector<Bullet>& bullets);
	void on_exit(Enemy& agent);
	void change_state(Enemy& agent, Arena& arena, EnemyState next_state);
	void on_enter(Enemy& agent, Arena& arena);
};







