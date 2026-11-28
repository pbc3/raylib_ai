#include "arena.h"
#include <array>
#include <exception>
#include <stdexcept>

bool out_of_bounds(Arena& arena, Vec2 requested_pos) {
    if (requested_pos.y > arena.map_height || requested_pos.x > arena.map_width ||
        requested_pos.x <= 0 || requested_pos.y <= 0) {
        return true;
    }
    return false;
};

bool valid_movement(Arena& arena, Vec2 requested_pos, float size) {
    // requested_pos is the centre of the entity
    //  calculate 4 corners from a centre pos
    float half_size = size / 2;
    // AABB
    Vec2 top_left = {requested_pos.x - half_size, requested_pos.y - half_size};
    Vec2 top_right = {requested_pos.x + half_size, requested_pos.y - half_size};
    Vec2 bottom_right = {requested_pos.x + half_size, requested_pos.y + half_size};
    Vec2 bottom_left = {requested_pos.x - half_size, requested_pos.y + half_size};
    std::array<Vec2, 4> boundaries{top_left, top_right, bottom_right, bottom_left};
    for (int i = 0; i <= 3; i++) {
        Vec2 boundary = boundaries[i];
        int tile = arena.vec2_to_tile(boundary);

        if (out_of_bounds(arena, boundary) || !arena.isWalkable(tile)) {
            return false;
        }
    }
    return true;
};

std::vector<Vec2> tile_path_to_vec2(Arena& arena, std::vector<int>& tile_path) {
    std::vector<Vec2> world_path;
    for (int p : tile_path) {
        world_path.push_back(arena.tile_to_world(p));
    }
    return world_path;
}

Arena::Arena(const std::string& csv_path) {
    auto res = parse_csv(csv_path);
    if (!res.has_value()) {
        // error
        throw std::runtime_error("csv parse failed");
    }
    if (auto& vec = res.value(); vec.empty()) {
        // empty vec, handle?
        throw std::logic_error("grid vector is empty");
    }

    // grid = res.value();

    grid = std::move(res.value());
    int r = grid.size();
    int c = grid[r - 1].size(); // assume size is even across all  rows..
    //
    cols = c;
    rows = r;
    map_height = rows * TILE_SIZE;
    map_width = cols * TILE_SIZE;

    // setup random

    float max_x = cols * TILE_SIZE;
    float max_y = rows * TILE_SIZE;
    gen = std::mt19937(rd());
    dist_x = std::uniform_real_distribution<float>(0.f, max_x);
    dist_y = std::uniform_real_distribution<float>(0.f, max_y);
}

Vec2 Arena::random_pos() {
    Vec2 pos = {0, 0};
    while (!valid_movement(*this, pos, 30)) {
        pos = {dist_x(gen), dist_y(gen)};
    }
    return pos;
}

int Arena::terrain_cost(int _row, int _col) {
    int value = tile(_row, _col);

    switch (value) {
    case TerrainType::GROUND: {
        return 2;
    }
    case TerrainType::WATER: {
        return 4;
    }
    case TerrainType::OPEN_DOOR: {
        return 2;
    }
    case TerrainType::FAST_GROUND:{
      return 1;
    }
    }
    return 1;
};

float Arena::terrain_speed_modifier(int tile_){
  auto [row,col] = tile_to_row_col(tile_);
  int value = tile(row,col);
  switch(value){
    case TerrainType::WATER:{
      return 0.5f;
    }
    case TerrainType::FAST_GROUND:{
      return 1.5f;
    }
    default:{
      return 1.f;
    }
  }
}

bool Arena::isWall(int _row, int _col) {
    int tile_val = tile(_row, _col);
    return tile_val == -1 || tile_val == TerrainType::WALL;
}
bool Arena::isWall(int tile_) {
    auto [row, col] = tile_to_row_col(tile_);
    int tile_val = tile(row, col);
    return tile_val == 1 ? true : false;
}
bool Arena::isWater(int tile_) {
    auto [row, col] = tile_to_row_col(tile_);
    int tile_val = tile(row, col);
    return tile_val == 2 ? true : false;
}
bool Arena::isClosedDoor(int tile_) {
    auto [row, col] = tile_to_row_col(tile_);
    int tile_val = tile(row, col);
    return tile_val == 5 ? true : false;
}
bool Arena::isOpenDoor(int tile_) {
    auto [row, col] = tile_to_row_col(tile_);
    int tile_val = tile(row, col);
    return tile_val == 3 ? true : false;
}

bool Arena::isDoor(int tile_) {
    bool is = isClosedDoor(tile_) || isOpenDoor(tile_);
    return is;
}
bool Arena::isObstacle(int tile_) {
    auto [row, col] = tile_to_row_col(tile_);
    int tile_val = tile(row, col);
    return tile_val == 4 ? true : false;
}
bool Arena::isWalkable(int tile) {
    if (isWall(tile) || isObstacle(tile) || isClosedDoor(tile) ||
        out_of_bounds(*this, tile_to_world(tile))) {
        return false;
    }
    return true;
}
bool Arena::isSuitableArtefact(int tile) {
    if (!isWalkable(tile) || isDoor(tile)) {
        return false;
    }
    return true;
}
int Arena::tile(int r, int c) {
    if (r >= rows || r < 0 || c >= cols || c < 0) {
        //__debugbreak();
        return -1; // treat oob as a wall
    }
    return grid[r][c];
}

std::pair<int, int> Arena::tile_to_row_col(int tile) {
    int row = tile / cols;
    int col = tile % cols;
    // if(row >= rows || col >= cols){
    // return
    // }
    return {row, col};
}

Vec2 Arena::tile_to_world(int tile) {
    int row = tile / cols;
    int col = tile % cols;
    return {col * TILE_SIZE + TILE_SIZE / 2.0f, row * TILE_SIZE + TILE_SIZE / 2.0f};
}

int Arena::flatten(int row, int col) {
    return (row * cols + col);
}

int Arena::vec2_to_tile(Vec2 world_pos) {
    int col = std::floor(world_pos.x / TILE_SIZE);
    int row = std::floor(world_pos.y / TILE_SIZE);
    return flatten(row, col);
}

void Arena::close_door(int player_tile, std::vector<int>& closed_door_tiles) {
    auto [row, col] = tile_to_row_col(player_tile);
    std::vector<int> tiles = {flatten(row + 1, col), flatten(row - 1, col), flatten(row, col + 1),
                              flatten(row, col - 1)};
    for (int i = 0; i <= 3; i++) {
        if (isClosedDoor(tiles[i])) {
            auto [r, c] = tile_to_row_col(tiles[i]);
            grid[r][c] = 3;
            std::erase_if(closed_door_tiles, [&](int x) { return x == tiles[i]; });
            break; // only close 1 door
        }
        if (isOpenDoor(tiles[i])) {
            auto [r, c] = tile_to_row_col(tiles[i]);
            grid[r][c] = 5;
            closed_door_tiles.push_back(tiles[i]);
            break; // only close 1 door
        }
    }
}
