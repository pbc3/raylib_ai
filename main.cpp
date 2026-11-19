#include "arena.h"
#include "raylib.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include "enemy.h"
#include "pathfinding.h"
#include "player.h"


constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;
constexpr int ARENA_WIDTH = SCREEN_WIDTH / TILE_SIZE;   // screen x / tile
constexpr int ARENA_HEIGHT = SCREEN_HEIGHT / TILE_SIZE; // screen y / tile
constexpr Color GRID_COLOUR = {48, 59, 72, 255};
// tile centring, possibly needed.
// constexpr int OFFSET_X = (SCREEN_WIDTH - ARENA_WIDTH * TILE_SIZE) / 2;
// constexpr int OFFSET_Y = (SCREEN_HEIGHT - ARENA_HEIGHT * TILE_SIZE) / 2;



// todo - factor these bools into a single enum, with helper functions like
// is_move_up etc
enum class MovementState : uint8_t {
  move_up = 0, move_down, move_left, move_right
};

struct input_state {
  bool move_up;
  bool move_down;
  bool move_left;
  bool move_right;
  MovementState movement_state;
  bool firing;
  Vec2 mouse_dir;
};


input_state handle_input(Camera2D camera, Player &player) {
  input_state input{false, false, false, false,MovementState::move_down, false, {0, 0}};
  if (IsKeyDown(KEY_W)) {
    // player_pos.y -= 1;
    input.move_up = true;
  }
  if (IsKeyDown(KEY_A)) {
    // player_pos.x -= 1;
    input.move_left = true;
  }
  if (IsKeyDown(KEY_S)) {
    // player_pos.y += 1;
    input.move_down = true;
  }
  if (IsKeyDown(KEY_D)) {
    // player_pos.x += 1;
    input.move_right = true;
  }
  // bullets
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    input.firing = true;
  }
  // aim direction
  Vector2 screen_mouse_pos = GetMousePosition();
  Vector2 world_mouse_pos = GetScreenToWorld2D(screen_mouse_pos, camera);
  input.mouse_dir =
      (*reinterpret_cast<Vec2 *>(&world_mouse_pos) - player.world_pos)
          .normalised();
  // TODO - poll mouse events in my own way. GLFW is causing issues.
  // i want to draw a tiny triangle,on the edge of the box, in the direction we
  // are facing

  return input;
}


float bullet_speed = 200.f;
struct Bullet {
  Vec2 world_pos;
  Vec2 velocity;
};

void draw_arena() {
  for (int i = 0; i < ARENA_HEIGHT; i++) {
    for (int j = 0; j < ARENA_WIDTH; j++) {
      // x = column * 50
      // y = row * 50
      int x = j * TILE_SIZE;
      int y = i * TILE_SIZE;
      DrawRectangleLines(x, y, TILE_SIZE, TILE_SIZE, BLUE);
    }
  }
  return;
}

void draw_grid(Arena &arena) {
  for (int i = 0; i < arena.rows; i++) {
    for (int j = 0; j < arena.cols; j++) {
      // x = column * 50
      // y = row * 50
      int x = j * TILE_SIZE;
      int y = i * TILE_SIZE;
      if (arena.isWall(i, j)) {
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, {100, 125, 150, 255});
      }
      // if (arena.grid[i][j] == 1) {
      //	DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, { 100, 125, 150, 255
      //});
      // }
    }
  }

  // <= because we want to draw edges
  for (int i = 0; i <= arena.rows; i++) {
    DrawLine(0, i * TILE_SIZE, arena.map_width, i * TILE_SIZE, GRID_COLOUR);
  }

  for (int j = 0; j <= arena.cols; j++) {
    DrawLine(j * TILE_SIZE, 0, j * TILE_SIZE, arena.map_height, GRID_COLOUR);
  }

  DrawRectangleLines(0, 0, arena.map_width, arena.map_height,
                     {141, 169, 196, 255});
};



std::vector<Bullet> bullets;

void update(Arena &arena, float dt, Player &player, Enemy& enemy, input_state input) {
// ideally, reduce this to contained function calls. enemy.update(dt);player.update(dt);bullet.update(dt);
  // todo - movement needs more work. what if the player can touch more than 4
  // tiles at once?

  // player update
      // intended movement
    Vec2 candidate{};

    // combine candidate intended movement into vector [-1,1] so opposites
    // naturally cancel to 0,
    if (input.move_down) {
      // candidate.y += player.velocity.y * dt;
      candidate.y += 1;
    }
    if (input.move_up) {
      // candidate.y -= player.velocity.y * dt;
      candidate.y -= 1;
    }
    if (input.move_left) {
      candidate.x -= 1;
    }
    if (input.move_right) {
      candidate.x += 1;
    }
    // normalise it so we get even movement across diagonal,
    candidate.normalise();

    // current vertical tile
    int c_top = static_cast<int>(
        std::floor(static_cast<float>(player.world_pos.y / TILE_SIZE)));
    int c_bottom = static_cast<int>(std::floor(static_cast<float>(
        (player.world_pos.y + player.size - 1) / TILE_SIZE)));

    // new horizontal movement
    float new_x = player.world_pos.x;
    new_x += candidate.x * player.velocity.x * dt;
    // calculate AABB tiles
    int left =
        static_cast<int>(std::floor(static_cast<float>(new_x / TILE_SIZE)));
    int right = static_cast<int>(
        std::floor(static_cast<float>((new_x + player.size - 1) / TILE_SIZE)));

    // verify horizontal movement
    if (!arena.isWall(c_top, left) && !arena.isWall(c_top, right) &&
        !arena.isWall(c_bottom, left) && !arena.isWall(c_bottom, right)) {
      player.world_pos.x = new_x;
    }

    // current horizontal tiles
    int c_left = static_cast<int>(
        std::floor(static_cast<float>(player.world_pos.x / TILE_SIZE)));
    int c_right = static_cast<int>(std::floor(static_cast<float>(
        (player.world_pos.x + player.size - 1) / TILE_SIZE)));

    // new vertical movement
    float new_y = player.world_pos.y;
    new_y += candidate.y * player.velocity.y * dt;

    int top =
        static_cast<int>(std::floor(static_cast<float>(new_y / TILE_SIZE)));
    int bottom = static_cast<int>(
        std::floor(static_cast<float>((new_y + player.size - 1) / TILE_SIZE)));

    // verify vertical movement
    if (!arena.isWall(top, c_left) && !arena.isWall(top, c_right) &&
        !arena.isWall(bottom, c_left) && !arena.isWall(bottom, c_right)) {
      player.world_pos.y = new_y;
    }
    
  {
    // bullets
    if (input.firing) {
      Bullet b;
      b.world_pos = player.world_pos;
      b.velocity = input.mouse_dir * bullet_speed;
      bullets.push_back(b);
    }

    for (Bullet &b : bullets) {
      if ( b.world_pos.y > arena.map_height ||  b.world_pos.x > arena.map_width ||  b.world_pos.x <= 0 ||
         b.world_pos.y <= 0){
          continue;}
      // if(b.tile == enemy.tile || player.tile){take damage...}
      b.world_pos += b.velocity * dt;
    }

   
    // enemy update
    // for(Enemy& enemy : enemies)

      // request new path
      if(enemy.player_tile != arena.vec2_to_tile(player.world_pos)){
      auto new_path = astar(arena, arena.vec2_to_tile(enemy.world_pos),arena.vec2_to_tile(player.world_pos));
      std::vector<Vec2> wp;
      for(int p : new_path){
            wp.push_back(arena.tile_to_world(p)); 
      }
      enemy.cur_path = wp;
      enemy.player_tile = arena.vec2_to_tile(player.world_pos);
      enemy.cur_waypoint=0;
    }
      enemy.follow_path(dt);
    

    // todo - introduce some temp variable so we can check this first..?
    if (new_y > arena.map_height || new_x > arena.map_width || new_x <= 0 ||
        new_y <= 0 || new_x + player.size > arena.map_width ||
        new_y + player.size > arena.map_height) {
      return;
    }


  }
}



int main() {
  SetTargetFPS(60);
  // todo - correct path should be assets/arena/arena.csv
  auto csv = R"(C:\dev\gai_cwk\assets\arena.csv)";
  Arena arena(csv);
  const auto adjacent = neighbours(arena, 146);
  for(auto [cost, node] : adjacent){
    std::cout << "cost : " << cost << " node : " << node << "\n";
  }
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GameAI");

  Player player{};
  player.world_pos = {100, 100};
  player.velocity = {350, 350};
  player.size = 30.f;


  Enemy enemy{};
  enemy.world_pos = {750,450};
  enemy.velocity = {350,350};
  enemy.size = 30.f;

  // setup initial paths
  int player_tile = arena.vec2_to_tile(player.world_pos);
  int enemy_tile = arena.vec2_to_tile(enemy.world_pos);
  auto path = astar(arena,enemy_tile,player_tile);
  std::vector<Vec2> world_path;
  for(int p : path){
    world_path.push_back(arena.tile_to_world(p));
  }
  enemy.cur_path = world_path;
  enemy.cur_waypoint = 0;
  enemy.player_tile = player_tile;

  // todo - clamp camera to map edges
  Camera2D camera = {0};
  camera.zoom = 1.f;
  camera.offset = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

  while (!WindowShouldClose()) {
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, GREEN);
    DrawText(TextFormat("Health: %i", (int) player.health), 160, 10, 20, GREEN);
    // if(IsKeyDown(KEY_R)) {
    //  reload csv
    // }

    float dt = GetFrameTime();
    // handle_input()
    input_state input = handle_input(camera, player);
    // update()
    // {
    // player_tile = arena.vec2_to_tile(player.world_pos);
    // enemy_tile = arena.vec2_to_tile(enemy.world_pos);
    // path = astar(arena,enemy_tile,player_tile);
    // std::vector<Vec2> wp;
    // for(int p : path){
    //     wp.push_back(arena.tile_to_world(p)); 
    //   }
    // enemy.cur_path = wp;
    //   }
    update(arena, dt, player, enemy, input);
    // Draw
    // camera.target = *reinterpret_cast<Vector2*>(&player.world_pos);
    BeginDrawing();
    ClearBackground({32, 38, 48, 255});
    Vector2 player_target = {player.world_pos.x + (player.size / 2),
                             player.world_pos.y + (player.size / 2)};
    camera.target = player_target;
    BeginMode2D(camera);


    // RENDERING. todo - factor this out into a func
    //  todo - stop drawing the grid every frame, draw it once into a texture,
    //  and just repeatdly show that
    draw_grid(arena);
    for (Bullet &b : bullets) {

      DrawCircle(b.world_pos.x, b.world_pos.y, 3, RED);
    }
    DrawRectangleV(*reinterpret_cast<Vector2 *>(&player.world_pos),
                   {player.size, player.size}, BLUE);
    DrawRectangleV(*reinterpret_cast<Vector2 *>(&enemy.world_pos),
{enemy.size, enemy.size}, BLACK);
    // DrawRectangle(player.world_pos.x, player.world_pos.y, player_size,
    // player_size, BLUE);/**/
    DrawRectangleLines(player.world_pos.x, player.world_pos.y, player.size,
                       player.size, RAYWHITE);
    for(auto waypoint_pos : enemy.cur_path){
      // Vec2 waypoint_pos = arena.tile_to_world(waypoint);
      DrawCircle(waypoint_pos.x, waypoint_pos.y, 5, RED);
    }
  //
  // DrawRectangleLines(width, height, player_size, player_size, RAYWHITE);
  // DrawRectangle(100, 100, 600, 400, { 100, 100, 100, 255 });
  EndMode2D();
  EndDrawing();
}

CloseWindow();
return 0;
}
