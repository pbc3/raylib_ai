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


  constexpr int SCREEN_WIDTH = 1280;
  constexpr int SCREEN_HEIGHT = 720;
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
  bool valid_movement(Arena& arena, Vec2 requested_pos, float size){
        float half_size = size / 2;
        // AABB
        Vec2 top_left = {requested_pos.x - half_size, requested_pos.y - half_size}; 
        Vec2 top_right = {requested_pos.x + half_size, requested_pos.y - half_size};
        Vec2 bottom_right = {requested_pos.x + half_size, requested_pos.y + half_size};
        Vec2 bottom_left = {requested_pos.x - half_size, requested_pos.y + half_size};
        int tl_tile = arena.vec2_to_tile(top_left);
        int tr_tile = arena.vec2_to_tile(top_right);
        int bl_tile = arena.vec2_to_tile(bottom_left);
        int br_tile = arena.vec2_to_tile(bottom_right);
        // verify horizontal movement
        if (!arena.isWall(tl_tile) && !arena.isWall(bl_tile) &&
            !arena.isWall(tr_tile) && !arena.isWall(br_tile)) {
          return true;
        }
      return false;
      }
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

     
      // world_pos is the centre of the entity
      //  calculate 4 corners from a centre pos
      float half_size = player.size / 2;
      // new horizontal movement  
      float new_x = player.world_pos.x;
      new_x += candidate.x * player.velocity.x * dt;
      Vec2 requested_movement = {new_x, player.world_pos.y};
      if(valid_movement(arena,requested_movement,player.size)){
         player.world_pos.x = new_x;
      }
      // Vec2 top_left = {new_x - half_size, player.world_pos.y - half_size}; 
      // Vec2 top_right = {new_x + half_size, player.world_pos.y - half_size};
      // Vec2 bottom_right = {new_x + half_size, player.world_pos.y + half_size};
      // Vec2 bottom_left = {new_x - half_size, player.world_pos.y + half_size};
      // // calculate AABB tiles
      // int tl_tile = arena.vec2_to_tile(top_left);
      // int tr_tile = arena.vec2_to_tile(top_right);
      // int bl_tile = arena.vec2_to_tile(bottom_left);
      // int br_tile = arena.vec2_to_tile(bottom_right);


      // // verify horizontal movement
      // if (!arena.isWall(tl_tile) && !arena.isWall(bl_tile) &&
      //     !arena.isWall(tr_tile) && !arena.isWall(br_tile)) {
      //   player.world_pos.x = new_x;
      // }
      // new vertical movement
      float new_y = player.world_pos.y;
      new_y += candidate.y * player.velocity.y * dt;
      requested_movement.x = player.world_pos.x;
      requested_movement.y = new_y;
      if(valid_movement(arena,requested_movement,player.size)){
         player.world_pos.y = new_y;
      }
      // top_left = {player.world_pos.x - half_size, new_y - half_size}; 
      // top_right = {player.world_pos.x + half_size, new_y - half_size};
      // bottom_right = {player.world_pos.x + half_size, new_y + half_size};
      // bottom_left = {player.world_pos.x - half_size, new_y +   half_size};
      // // calculate AABB tiles for intended vertical movement
      // tl_tile = arena.vec2_to_tile(top_left);
      // tr_tile = arena.vec2_to_tile(top_right);
      // bl_tile = arena.vec2_to_tile(bottom_left);
      // br_tile = arena.vec2_to_tile(bottom_right);
      // // verify vertical movement
      //  if (!arena.isWall(tl_tile) && !arena.isWall(bl_tile) &&
      //     !arena.isWall(tr_tile) && !arena.isWall(br_tile)) {
      //   player.world_pos.y = new_y;
      // }
      
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
    player.world_pos = {100,100};
    player.velocity = {350, 350};
    player.size = 25.f;


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
      // if(IsKeyDown(KEY_R)) {
      //  reload csv
      // }
      float dt = GetFrameTime();
      input_state input = handle_input(camera, player);
      update(arena, dt, player, enemy, input);

      //render
      BeginDrawing();
      ClearBackground({32, 38, 48, 255});
      camera.target = {player.world_pos.x,player.world_pos.y};
      BeginMode2D(camera);
      //  todo - stop drawing the grid every frame, draw it once into a texture,
      draw_grid(arena);
      for (Bullet &b : bullets) { DrawCircle(b.world_pos.x, b.world_pos.y, 3, RED);}
      Vec2 player_top_left = player.world_pos - Vec2{player.size / 2, player.size / 2};
      Vec2 enemy_top_left = enemy.world_pos - Vec2{enemy.size / 2, enemy.size / 2};
      DrawRectangleV(*reinterpret_cast<Vector2 *>(&player_top_left),{player.size, player.size}, BLUE);
      DrawRectangleV(*reinterpret_cast<Vector2 *>(&enemy_top_left),{enemy.size, enemy.size}, BLACK);
      DrawRectangleLines(player_top_left.x, player_top_left.y, player.size,player.size, RAYWHITE);
      for(auto waypoint_pos : enemy.cur_path){
        DrawCircle(waypoint_pos.x, waypoint_pos.y, 2.2, RED);
      }

    EndMode2D();
    EndDrawing();
    DrawText(TextFormat("FPS: %i", GetFPS()), 10, 10, 20, GREEN);
    DrawText(TextFormat("Health: %i", (int) player.health), 160, 10, 20, GREEN);
  }

  CloseWindow();
  return 0;
  }
