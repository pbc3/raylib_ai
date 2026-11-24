#pragma once
#include "math_utils.h"
#include "utils.h"
#include <array>
#include <exception>
#include <stdexcept>
#include "entity.h"

constexpr int TILE_SIZE = 32;

struct Arena {
  int cols;
  int rows;
  std::vector<std::vector<int>> grid;
  int map_height;
  int map_width;
  // each row has cols entries. row = y, col = x.
  Arena(const std::string &csv_path);
  Arena(int cols_, int rows_)
      : cols(cols_), rows(rows_), grid(rows, std::vector<int>(cols, 0)),
        map_height(rows * TILE_SIZE), map_width(cols * TILE_SIZE) {};
  int tile(int r, int c);
  bool isWall(int _row, int _col);
  bool isWall(int tile);
  // bool isObstacle(int tile_idx) {
  //	return tile_idx in set of obstacles
  // }
  int flatten(int row, int col);
  int vec2_to_tile(Vec2 world_pos);
  std::pair<int, int> tile_to_row_col(int tile);
  Vec2 tile_to_world(int tile);
};

std::vector<Vec2> tile_path_to_vec2(Arena& arena, std::vector<int>& tile_path);


const float BULLET_DAMAGE = 20.f;

bool out_of_bounds(Arena& arena, Vec2 requested_pos);
bool valid_movement(Arena& arena, Vec2 requested_pos, float size);

// bool bullet_hit(Bullet& bullet, Entity& entity);
// Entity* hit_entity(Player& player, std::vector<Enemy>& enemies, Bullet& bullet);
// void spawn_bullet(std::vector<Bullet>& bullets, Vec2 spawn_pos, Vec2 direction, Team team);



