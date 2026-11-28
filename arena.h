#pragma once
#include "math_utils.h"
#include "utils.h"
#include <array>
#include <exception>
#include <stdexcept>
#include "entity.h"
#include <random>

constexpr int TILE_SIZE = 32;
const float BULLET_DAMAGE = 20.f;

enum TerrainType{
  GROUND, WALL, WATER, OPEN_DOOR, OBSTACLE, CLOSED_DOOR
};

struct Arena {
  int cols;
  int rows;
  std::vector<std::vector<int>> grid;
  int map_height;
  int map_width;

  std::uniform_real_distribution<float> dist_x;
  std::uniform_real_distribution<float> dist_y;
  std::random_device rd;
  std::mt19937 gen;

  // each row has cols entries. row = y, col = x.
  Arena(const std::string &csv_path);
  Arena(int cols_, int rows_): cols(cols_), rows(rows_), grid(rows, std::vector<int>(cols, 0)), map_height(rows * TILE_SIZE), map_width(cols * TILE_SIZE) {};
  int tile(int r, int c);

  bool isWall(int _row, int _col);
  bool isWall(int tile);
  bool isWater(int tile);
  bool isObstacle(int tile);
  bool isClosedDoor(int tile_);
  bool isOpenDoor(int tile_);
  bool isDoor(int tile_);
  bool isWalkable(int tile);
  bool isSuitableArtefact(int tile);
  void close_door(int player_tile, std::vector<int>& open_door_tiles);

  int flatten(int row, int col);
  int vec2_to_tile(Vec2 world_pos);
  std::pair<int, int> tile_to_row_col(int tile);
  Vec2 tile_to_world(int tile);
  Vec2 random_pos();
  int terrain_cost(int _row, int _col);
  // guard zones
  void fill_zone(int start_node, int zone);
  std::vector<int> tile_to_zone; // input the players current tile, get out what zone player is in
  int get_player_zone(int player_tile);
  void setup_guard_zones();

};

std::vector<Vec2> tile_path_to_vec2(Arena& arena, std::vector<int>& tile_path);
bool out_of_bounds(Arena& arena, Vec2 requested_pos);
bool valid_movement(Arena& arena, Vec2 requested_pos, float size);



