
#include "raylib.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "artefacts.h"
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



  // todo - factor these bools into a single enum, with helper functions like
  // is_move_up etc


struct input_state {
  bool move_up;
  bool move_down;
  bool move_left;
  bool move_right;
  bool firing;
  Vec2 mouse_dir;
};


input_state handle_input(Camera2D camera, Player &player) {
  input_state input{};
  input.move_up = IsKeyDown(KEY_W);
  input.move_down = IsKeyDown(KEY_S);
  input.move_left = IsKeyDown(KEY_A);
  input.move_right = IsKeyDown(KEY_D);
  input.firing = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  // aim direction
  Vector2 screen_mouse_pos = GetMousePosition();
  Vector2 world_mouse_pos = GetScreenToWorld2D(screen_mouse_pos, camera);
  input.mouse_dir =(*reinterpret_cast<Vec2 *>(&world_mouse_pos) - player.world_pos).normalised();
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

      int tile = arena.flatten(i,j);
      DrawText(TextFormat("%i", (int) arena.flatten(i,j)), x, y, 3, GREEN);

      if (arena.isWall(i, j)) {
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, {100, 125, 150, 255});
        continue;
      }
      if(arena.isWater(tile)){
        // DrawTexture
        DrawText("W", x,y,25,RED);
        continue;
      }
      if(arena.isObstacle(tile)){
        // DrawTexture
        DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, {100, 125, 150, 255});
        continue;
      }
      if(arena.isDoor(tile)){
        // DrawTexture 
      }

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


void draw_debu(Arena& arena){
  Color zone_colors[] = {
    GREEN, BLUE, PURPLE
};

for (int i = 0; i < arena.tile_to_zone.size(); i++)
{
    int zone = arena.tile_to_zone[i];
  
    if (zone == -1)
        continue;

    int row = i / arena.cols;
    int col = i % arena.cols;
  
    // DrawRectangle(
    //     col * TILE_SIZE,
    //     row * TILE_SIZE,
    //     TILE_SIZE,
    //     TILE_SIZE,
    //     zone_colors[zone]
    // );

    DrawText(TextFormat("%i",zone),col*TILE_SIZE,row*TILE_SIZE,TILE_SIZE,RED);
}
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
          enemy.update(arena,player,dt,bullets);
        }
      // for(Enemy& enemy : enemies){
      //   enemy.perceptions.update(&enemy,arena,player);
      //   if(enemy.perceptions.spotted_player){
      //     enemy.replan(arena,enemy.perceptions.last_seen_player);
      //   }
      //   else{
      //     // rotate.. do this in patrol, after the transitions checks, before? moving to a location. 
      //     enemy.forward = {-enemy.forward.y,enemy.forward.x};
      //   }
      //   enemy.follow_path(dt,arena);
      // }


      {
      // bullets
      if (input.firing) {
        spawn_bullet(bullets, player.world_pos, input.mouse_dir, Team::PLAYER);
      }

      // auto cur_time = std::chrono::steady_clock::now();
      // auto cooldown_interval = std::chrono::seconds(3);
      // if(!enemies.empty()){
      // if(cur_time - enemies[0].last_shot_at >= cooldown_interval){
      //   // shoot again after cooldown
      //   Vec2 direction = player.world_pos - enemies[0].world_pos;
      //   direction.normalise();
     
      //   spawn_bullet(bullets,enemies[0].world_pos,direction,Team::ENEMY);
      //   enemies[0].last_shot_at = cur_time;
      // }}




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

void draw_fov_cone(Enemy& enemy,Arena& arena){
  float half_fov =  enemy.perceptions.view_angle_rads / 2.f;
  float dir_ang = std::atan2f(enemy.forward.y,enemy.forward.x);
  std::vector<Vector2> points;
  points.push_back({enemy.world_pos.x,enemy.world_pos.y});
  for(float angle = -half_fov; angle <= half_fov; angle+=0.05f){
    Vector2 point = {enemy.world_pos.x + std::cosf(dir_ang + angle) * enemy.perceptions.max_distance,
     enemy.world_pos.y + std::sinf(dir_ang + angle) * enemy.perceptions.max_distance
    };
// todo - raymarch and check if a wall is there, then stop drawing the line at that point
    points.push_back(point);
  }

DrawLineV(
    {enemy.world_pos.x, enemy.world_pos.y},
    {
        enemy.world_pos.x + enemy.forward.x * enemy.perceptions.max_distance,
        enemy.world_pos.y + enemy.forward.y * enemy.perceptions.max_distance
    },
    RED
);
 DrawTriangleFan(points.data(), (int)points.size(), Fade(YELLOW, 0.3f));
 DrawTriangleStrip(points.data(), (int)points.size(), Fade(YELLOW, 0.3f));
}
constexpr const char* ENEMY_STATE_STRINGS[] = {
    "PATROL",
    "CHASE",
    "TAKE COVER",
    "HEAL",
    "ENGAGE",
    "GUARD",
    "INVESTIGATE",
    "FLEE",
    "RETURN TO POST"
};

const char* get_state(EnemyState state){
  return ENEMY_STATE_STRINGS[std::bit_cast<int>(state)];
};

void draw_artefacts(Arena& arena, std::vector<Artefact>& artefact_tiles){
      for(Artefact artefact_tile : artefact_tiles){
        Vec2 world_loc = (artefact_tile.world_pos);
        DrawCircle(world_loc.x,world_loc.y,3,BLUE);
      }
};

void spawn_enemy_A(std::vector<Enemy>& enemies, Arena& arena, Player& player){
    Vec2 random_spawn = arena.random_pos();
    Enemy enemy{};
    enemy.world_pos = random_spawn;
    enemy.velocity = {350,350};
    enemy.size = 30.f;
    enemy.dead = false; 
    enemy.health = 100;
    enemy.perceptions.spotted_player = false;
    enemy.forward = {1.0f,0.f};
    enemy.type = EnemyType::CHASER;
    enemy.setup_patrol_paths(arena);
    enemy.fsm.on_enter(enemy, arena,player);
    enemies.push_back(enemy);
}
void spawn_enemy_B(std::vector<Enemy>& enemies, Arena& arena, Player& player){
    for(int i = 0; i < 3; i++){
    Enemy enemy{};
    enemy.guard_zone = i;
    enemy.world_pos = arena.tile_to_world(enemy.guard_zones[enemy.guard_zone]);
    enemy.velocity = {350,350};
    enemy.size = 30.f;
    enemy.dead = false; 
    enemy.health = 100;
    enemy.perceptions.spotted_player = false;
    enemy.forward = {1.0f,0.f};
    enemy.type = EnemyType::GUARD;
    // enemy.setup_patrol_paths(arena);
    enemy.fsm.state = EnemyState::GUARD;
    enemy.fsm.on_enter(enemy, arena,player);
    enemies.push_back(enemy);
    }

}

  int main() {

    SetTargetFPS(120);

    // get path realtive to build-dir
    const auto csv = std::filesystem::path(GetApplicationDirectory()) / "assets" / "map_binary.csv";
    // std::string csv = R"(C:\dev\gai_cwk\assets\arena.csv)";
    // csv = csvv.string();
    // auto cs = ".assets\arena.csv";
    Arena arena(csv.string());


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GameAI");

    //gameplay setup
    Player player{};
    player.world_pos = {550,150};
    player.velocity = {550, 550};
    player.size = 25.f;
    player.health = 100.f;
    player.team = Team::PLAYER;

    std::vector<Enemy> enemies;
    // todo - factor into a spawn enemy function, then factor that all into a setup_game() including the enemy, initial path, and the spawning of the
    // needed stuff for section 2

    spawn_enemy_A(enemies,arena,player);
    enemies[0].setup_guard_zones(arena);
    // enemies.push_back(enemy1);
    
    spawn_enemy_B(enemies,arena,player);

    // setup artefacts
    std::vector<Artefact> artefact_tiles = generate_artefacts(arena,player);


    auto cur_time = std::chrono::steady_clock::now();
    enemies[0].last_shot_at = cur_time;

    std::vector<Bullet> bullets;

    // debug = enemy paths and FOV visualisation
    bool draw_debug = false;

    // todo - clamp camera to map edges
    Camera2D camera = {0};
    camera.zoom = 1.f;
    camera.offset = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    while (!WindowShouldClose()) {



      if(IsKeyPressed(KEY_R)) {
       arena.grid[25][15] = !arena.grid[25][15];
      }


      if(IsKeyPressed(KEY_B)) {
        draw_debug = !draw_debug;
      }

      if(IsKeyPressed(KEY_Q)){
         spawn_enemy_A(enemies,arena,player);
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
      draw_artefacts(arena,artefact_tiles);
      for (Bullet &b : bullets) { DrawCircle(b.world_pos.x, b.world_pos.y, b.size, BLACK);}
      Vec2 player_top_left = player.world_pos - Vec2{player.size / 2, player.size / 2};
       DrawRectangleV(*reinterpret_cast<Vector2 *>(&player_top_left),{player.size, player.size}, BLUE);
      for(int i = 0; i < enemies.size(); i++){
        auto& enemy = enemies[i];
        Vec2 enemy_top_left = enemy.world_pos - Vec2{enemy.size / 2, enemy.size / 2};
        DrawRectangleV(*reinterpret_cast<Vector2 *>(&enemy_top_left),{enemy.size, enemy.size}, colours[(i + 1) % colours.size()]);
        DrawText(TextFormat("%i", (int) enemy.health), enemy.world_pos.x+enemy.size/2, enemy.world_pos.y-50, 40, GREEN);
        DrawText(TextFormat("%s", get_state(enemy.fsm.state)), enemy.world_pos.x+enemy.size/2, enemy.world_pos.y-10, 40, GREEN);
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
      // draw_debu(arena);
      if(draw_debug){
      for(Enemy& enemy : enemies){
      draw_fov_cone(enemy,arena);
      for(auto waypoint_pos : enemy.cur_path){
        DrawCircle(waypoint_pos.x, waypoint_pos.y, 2.2, RED);
      }
    }
  }

    EndMode2D();
    EndDrawing();
    if (!enemies.empty()) {
    const Enemy& e = enemies[0];
    DrawText(TextFormat( "STATE: %d | GOAL: %d | WP: %d/%d | SEEN: %d | RANGE: %d"
      ,static_cast<int>(e.fsm.state),e.goal_tile,e.cur_waypoint,static_cast<int>(e.cur_path.size()),e.perceptions.spotted_player,e.perceptions.in_range), 10, 10, 10, GREEN);
}
  }

  CloseWindow();
  return 0;
  }
