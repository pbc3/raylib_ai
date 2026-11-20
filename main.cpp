
#include "raylib.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>

#include "arena.h"
#include "enemy.h"
#include "player.h"
#include "bullet.h"
#include "pathfinding.h"

constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;
constexpr int ARENA_WIDTH = SCREEN_WIDTH / TILE_SIZE;   // screen x / tile
constexpr int ARENA_HEIGHT = SCREEN_HEIGHT / TILE_SIZE; // screen y / tile
constexpr Color GRID_COLOUR = {48, 59, 72, 255};
// tile centring, possibly needed.
// constexpr int OFFSET_X = (SCREEN_WIDTH - ARENA_WIDTH * TILE_SIZE) / 2;
// constexpr int OFFSET_Y = (SCREEN_HEIGHT - ARENA_HEIGHT * TILE_SIZE) / 2;

std::vector<Color> colours{WHITE,RED};

const float BULLET_DAMAGE = 20.f;

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
    input.move_up = true;
  }
  if (IsKeyDown(KEY_A)) {
    input.move_left = true;
  }
  if (IsKeyDown(KEY_S)) {
    input.move_down = true;
  }
  if (IsKeyDown(KEY_D)) {
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
  // i want to draw a tiny triangle,on the edge of the box, in the direction we
  // are facing

  return input;
}




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



bool bullet_hit(Bullet& bullet, Entity& entity){
    Rectangle r;
    float half_size = entity.size / 2;
    r.x = entity.world_pos.x - half_size;
    r.y = entity.world_pos.y - half_size;
    r.width = half_size * 2;
    r.height = half_size * 2;
  return CheckCollisionCircleRec(*reinterpret_cast<Vector2*>(&bullet.world_pos),bullet.size,r);
};

Entity* hit_entity(Player& player, std::vector<Enemy>& enemies, Bullet& bullet){
  // loop over each enemy, and then the player. see if the bullet collides
  // check enemies
  if(bullet.team == Team::PLAYER){
    for(Enemy& enemy : enemies){
      if(bullet_hit(bullet,enemy)){
        bullet.expired = true;
        return &enemy;
      }
    }
  }
    else{
      // check whether bullet hit player
      if(bullet_hit(bullet,player)){
        bullet.expired = true;
        return &player;
      }
    }
    return nullptr;
}

void spawn_bullet(std::vector<Bullet>& bullets, Vec2 spawn_pos, Vec2 direction, Team team){
  Bullet b;
  b.world_pos = spawn_pos;
  b.velocity = direction * b.bullet_speed;
  b.expired = false;
  b.team = team;
  b.size = 5.0f;
  bullets.push_back(b);
}



void update(Arena &arena, float dt, Player &player,  std::vector<Enemy>& enemies,  std::vector<Bullet>& bullets, input_state input) {
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

  

      // new horizontal movement  
      float new_x = player.world_pos.x;
      new_x += candidate.x * player.velocity.x * dt;
      Vec2 requested_movement = {new_x, player.world_pos.y};
      if(valid_movement(arena,requested_movement,player.size)){
         player.world_pos.x = new_x;
      }
      // new vertical movement
      float new_y = player.world_pos.y;
      new_y += candidate.y * player.velocity.y * dt;
      requested_movement.x = player.world_pos.x;
      requested_movement.y = new_y;
      if(valid_movement(arena,requested_movement,player.size)){
         player.world_pos.y = new_y;
      }
      

            // enemy update
      for(Enemy& enemy : enemies){
        enemy.follow_path(dt,arena,player.world_pos);
      }


      {
      // bullets
      if (input.firing) {
        spawn_bullet(bullets, player.world_pos, input.mouse_dir, Team::PLAYER);
      }

      auto cur_time = std::chrono::steady_clock::now();
      auto cooldown_interval = std::chrono::seconds(3);
      if(!enemies.empty()){
      if(cur_time - enemies[0].last_shot_at >= cooldown_interval){
        // shoot again after cooldown
        Vec2 direction = player.world_pos - enemies[0].world_pos;
        direction.normalise();
     
        spawn_bullet(bullets,enemies[0].world_pos,direction,Team::ENEMY);
        enemies[0].last_shot_at = cur_time;
      }}
      



      for (Bullet &b : bullets) {
        Vec2 new_bulet_pos = b.world_pos + b.velocity * dt;
        // check whether bullet is out of bounds
        if(valid_movement(arena,new_bulet_pos,b.size)){
          b.world_pos = new_bulet_pos;
          // check whether bullet hit enemy or player
          Entity* hit_target = hit_entity(player,enemies,b);
          if(hit_target){
            hit_target->take_damage(BULLET_DAMAGE);
          }
        }
        else{
          b.expired = true;
        }
      }
    
    // remove dead bullets
      std::erase_if(bullets,[](Bullet& b){
        return b.expired;
      });
  
      std::erase_if(enemies,[](Enemy& e){
        return e.dead;
      });
  

    }
  }



  int main() {

    SetTargetFPS(360);

    // todo - correct path should be assets/arena/arena.csv
    auto csv = R"(C:\dev\gai_cwk\assets\arena.csv)";
    Arena arena(csv);


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GameAI");

    //gameplay setup
    Player player{};
    player.world_pos = {100,100};
    player.velocity = {550, 550};
    player.size = 25.f;

    std::vector<Enemy> enemies;
    // todo - factor into a spawn enemy function, then factor that all into a setup_game() including the enemy, initial path, and the spawning of the
    // needed stuff for section 2
    Enemy enemy{};
    enemy.world_pos = {456,400};
    enemy.velocity = {350,350};
    enemy.size = 30.f;
    Enemy enemy1{};
    enemy1.world_pos = {657,400};
    enemy1.velocity = {300,300};
    enemy1.size = 20.f;
    enemy1.health = 100; enemy.health = 100; player.health = 100;
    enemy.dead = false; enemy1.dead = false;
    // setup initial paths
    
    enemies.push_back(enemy);
    enemies.push_back(enemy1);

    int player_tile = arena.vec2_to_tile(player.world_pos);
    for(Enemy& enemy_agent : enemies){
      int enemy_tile = arena.vec2_to_tile(enemy_agent.world_pos);
      auto path = astar(arena,enemy_tile,player_tile);
      std::vector<Vec2> world_path;
      for(int p : path){
       world_path.push_back(arena.tile_to_world(p));
      }
      enemy_agent.cur_path = world_path;
      enemy_agent.player_tile = player_tile;
      enemy_agent.cur_waypoint = 0;
    }
    auto cur_time = std::chrono::steady_clock::now();
    enemies[0].last_shot_at = cur_time;

    std::vector<Bullet> bullets;


    // todo - clamp camera to map edges
    Camera2D camera = {0};
    camera.zoom = 1.f;
    camera.offset = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    bool draw_paths = true;
    while (!WindowShouldClose()) {
      if(IsKeyDown(KEY_R)) {
       arena.grid[19][15] = !arena.grid[19][15];
      }
      float dt = GetFrameTime();
      input_state input = handle_input(camera, player);
      update(arena, dt, player, enemies,bullets, input);

      //render
      BeginDrawing();
      ClearBackground({32, 38, 48, 255});
      camera.target = {player.world_pos.x,player.world_pos.y};
      BeginMode2D(camera);
      //  todo - stop drawing the grid every frame, draw it once into a texture,
      draw_grid(arena);
      for (Bullet &b : bullets) { DrawCircle(b.world_pos.x, b.world_pos.y, b.size, BLACK);}
      Vec2 player_top_left = player.world_pos - Vec2{player.size / 2, player.size / 2};
       DrawRectangleV(*reinterpret_cast<Vector2 *>(&player_top_left),{player.size, player.size}, BLUE);
      for(int i = 0; i < enemies.size(); i++){
        auto enemy = enemies[i];
        Vec2 enemy_top_left = enemy.world_pos - Vec2{enemy.size / 2, enemy.size / 2};
        DrawRectangleV(*reinterpret_cast<Vector2 *>(&enemy_top_left),{enemy.size, enemy.size}, colours[i]);
        DrawText(TextFormat("%i", (int) enemy.health), enemy.world_pos.x, enemy.world_pos.y, 20, GREEN);
      }

      DrawRectangleLines(player_top_left.x, player_top_left.y, player.size,player.size, RAYWHITE);
float half = player.size / 2;
float gap = 10.0f;

    Vec2 dir = { input.mouse_dir.x, input.mouse_dir.y };
    Vec2 perpendicular = { -dir.y, dir.x };

    Vec2 tip = player.world_pos + dir * (half + gap);

    Vec2 top = player.world_pos
        + dir * half
        - perpendicular * (half / 2);

    Vec2 bottom = player.world_pos
        + dir * half
        + perpendicular * (half / 2);
          DrawTriangle(
        { tip.x, tip.y },
        { top.x, top.y },
        { bottom.x, bottom.y },
        RED
    );
      if(draw_paths){
      for(Enemy& enemy : enemies){
      for(auto waypoint_pos : enemy.cur_path){
        DrawCircle(waypoint_pos.x, waypoint_pos.y, 2.2, RED);
      }
    }
  }

    EndMode2D();
    EndDrawing();
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, GREEN);
    DrawText(TextFormat("Health: %i", (int) player.health), 160, 10, 20, GREEN);
  }

  CloseWindow();
  return 0;
  }
