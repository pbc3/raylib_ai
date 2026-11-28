#pragma once
#include "arena.h"
#include "artefacts.h"
#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include <algorithm>
#include <array>
#include <vector>

struct InputState;

struct GameLevel {
  Player player;
  std::vector<Enemy> enemies;
  std::vector<Bullet> bullets;
  std::array<ArtefactSet, 6> artefact_tiles;
  Arena &arena;
  std::vector<int> door_tiles;
  void restart_level();
  void update(Arena &arena, float dt, GameLevel &level, InputState input);
};

void update_enemies(Arena &arena, std::vector<Bullet> &bullets, Player &player,
                    std::vector<Enemy> &enemies, float dt);
void spawn_enemy_B(std::vector<Enemy> &enemies, Arena &arena, Player &player);
void spawn_enemy_A(std::vector<Enemy> &enemies, Arena &arena, Player &player);
void update_bullets(Arena &arena, std::vector<Bullet> &bullets, Player &player,
                    std::vector<Enemy> &enemies, float dt);
Entity *hit_entity(Player &player, std::vector<Enemy> &enemies, Bullet &bullet);
