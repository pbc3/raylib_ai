#pragma once
#include "arena.h"
#include "bullet.h"
#include "entity.h"
#include "fsm.h"
#include "math_utils.h"
#include "pathfinding.h"
#include "player.h"
#include <algorithm>
#include <chrono>
#include <numbers>
#include <random>
#include <vector>

struct Enemy;

struct Perceptions {
    Vec2 last_seen_player;
    bool spotted_player;
    bool obstructed_los;
    bool in_zone;
    bool in_range;
    float engage_range = 150.f;
    float view_angle_rads = 2.0944f; // 90* in radians
    float max_distance = 650.f;
    void update(Enemy* agent, Arena& arena, Player& player);
};

enum class MovementState { MOVING, BLOCKED, ARRIVED, NO_PATH };

enum class EnemyType { CHASER, GUARD };

struct Enemy : public Entity {
    EnemyType type;
    int id;
    Vec2 velocity;
    float threshold = .2f;
    int goal_tile = -1;
    std::chrono::steady_clock::time_point last_shot_at;
    std::vector<Vec2> cur_path;
    int cur_waypoint = 0;
    float replan_timer;

    EnemyStateMachine fsm;
    Perceptions perceptions;
    Vec2 forward;
    Vec2 move_direction;
    Vec2 player_pos;
    std::vector<Vec2> patrol_targets;
    int current_patrol_idx;
    bool scanning;
    Vec2 current_goal_pos;
    float rotation_progress = 0.f;
    int guard_zone;
    int current_investigate_corner = 0;
    int current_flee_zone;
    std::array<int, 3> guard_zones = {890, 215, 311};
    std::array<int, 3> flee_zones = {892, 217, 313};
    std::array<std::vector<int>, 3> investigate_zones = {
        {{933, 1196, 962, 682, 689},       // zone 0 corners
         {102, 343, 282, 122, 91},         // zone 1 corners
         {117, 397, 717, 587, 147, 591}}}; // zone 2 corners

    void setup_guard_zones(Arena& arena);
    void fill_zone(int start_node, int zone, Arena& arena);
    void update(Arena& arena, Player& player, float dt, std::vector<Bullet>& bullets);
    MovementState follow_path(float dt, Arena& arena);
    void replan(Arena& arena, Vec2 goal_pos);
    float& take_damage(float dmg);
    void engage_player(Arena& arena, float dt, Player& player, std::vector<Bullet>& bullets);
    void patrol(Arena& arena, float dt);
    void setup_patrol_paths(Arena& arena);
};
