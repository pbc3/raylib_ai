
#include "raylib.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

#include "arena.h"
#include "artefacts.h"
#include "bullet.h"
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

Vec2 centre_to_top_left(Vec2 centre_coords, float size) {
    float half_s = size / 2;
    float x = centre_coords.x - half_s;
    float y = centre_coords.y - half_s;
    return {x, y};
};

std::vector<Color> colours{WHITE, RED};

bool bullet_hit(Bullet& bullet, Entity& entity) {
    Rectangle r;
    float half_size = entity.size / 2;
    r.x = entity.world_pos.x - half_size;
    r.y = entity.world_pos.y - half_size;
    r.width = half_size * 2;
    r.height = half_size * 2;
    return CheckCollisionCircleRec(*reinterpret_cast<Vector2*>(&bullet.world_pos), bullet.size, r);
};

Entity* hit_entity(Player& player, std::vector<Enemy>& enemies, Bullet& bullet) {
    // loop over each enemy, and then the player. see if the bullet collides
    // check enemies
    if (bullet.team == Team::PLAYER) {
        for (Enemy& enemy : enemies) {
            if (bullet_hit(bullet, enemy)) {
                bullet.expired = true;
                return &enemy;
            }
        }
    } else {
        // check whether bullet hit player
        if (bullet_hit(bullet, player)) {
            bullet.expired = true;
            return &player;
        }
    }
    return nullptr;
}

struct input_state {
    bool move_up;
    bool move_down;
    bool move_left;
    bool move_right;
    bool firing;
    bool interract;
    Vec2 mouse_dir;
};

input_state handle_input(Camera2D camera, Player& player) {
    input_state input{};
    input.move_up = IsKeyDown(KEY_W);
    input.move_down = IsKeyDown(KEY_S);
    input.move_left = IsKeyDown(KEY_A);
    input.move_right = IsKeyDown(KEY_D);
    input.firing = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    input.interract = IsKeyPressed(KEY_E);
    // aim direction
    Vector2 screen_mouse_pos = GetMousePosition();
    Vector2 world_mouse_pos = GetScreenToWorld2D(screen_mouse_pos, camera);
    input.mouse_dir = (*reinterpret_cast<Vec2*>(&world_mouse_pos) - player.world_pos).normalised();
    return input;
}

void draw_grid(Arena& arena) {
    for (int i = 0; i < arena.rows; i++) {
        for (int j = 0; j < arena.cols; j++) {
            // x = column * 50
            // y = row * 50
            int x = j * TILE_SIZE;
            int y = i * TILE_SIZE;

            int tile = arena.flatten(i, j);

            if (arena.isWall(i, j)) {
                DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, {100, 125, 150, 255});
                continue;
            }
            if (arena.isWater(tile)) {
                // DrawTexture
                continue;
            }
            if (arena.isObstacle(tile)) {
                // DrawTexture
                // DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, {100, 125, 150, 255});
                continue;
            }
            // if(arena.isDoor(tile)){
            //   // DrawTexture
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

    DrawRectangleLines(0, 0, arena.map_width, arena.map_height, {141, 169, 196, 255});
};

void update_player(Arena& arena, Player& player, input_state input, float dt) {
    Vec2 candidate{};
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
    if (valid_movement(arena, requested_movement, player.size)) {
        player.world_pos.x = new_x;
    }
    // new vertical movement
    float new_y = player.world_pos.y;
    new_y += candidate.y * player.velocity.y * dt;
    requested_movement.x = player.world_pos.x;
    requested_movement.y = new_y;
    if (valid_movement(arena, requested_movement, player.size)) {
        player.world_pos.y = new_y;
    }
}
void update_bullets(Arena& arena, std::vector<Bullet>& bullets, Player& player,
                    std::vector<Enemy>& enemies, float dt) {
    for (Bullet& b : bullets) {
        Vec2 new_bulet_pos = b.world_pos + b.velocity * dt;
        // check whether bullet is out of bounds
        if (valid_movement(arena, new_bulet_pos, b.size)) {
            b.world_pos = new_bulet_pos;
            // check whether bullet hit enemy or player
            Entity* hit_target = hit_entity(player, enemies, b);

            if (hit_target) {
                // if(hit_target == &player){
                //   player.take_damage()
                // }
                hit_target->take_damage(BULLET_DAMAGE);
            }
        } else {
            b.expired = true;
        }
    }
}
inline void update_enemies(Arena& arena, std::vector<Bullet>& bullets, Player& player,
                           std::vector<Enemy>& enemies, float dt) {
    // enemy update
    for (Enemy& enemy : enemies) {
        enemy.update(arena, player, dt, bullets);
    }
}
void remove_dead_entities(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies) {
    std::erase_if(bullets, [](Bullet& b) { return b.expired; });

    std::erase_if(enemies, [](Enemy& e) { return e.dead; });
}
void update_artefacts(std::array<ArtefactSet, 6>& artefact_sets, Player& player) {
    // artefact collection
    for (ArtefactSet& af_set : artefact_sets) {
        for (Artefact& af : af_set.artefacts) {
            if (player.world_pos.distance(af.world_pos) < 25.f) {
                af.collected = true;
            }
        }
    }
}
void update(Arena& arena, float dt, Player& player, std::vector<Enemy>& enemies,
            std::vector<Bullet>& bullets, input_state input,
            std::array<ArtefactSet, 6>& artefact_sets, std::vector<int>& closed_door_tiles) {
    update_player(arena, player, input, dt);
    update_enemies(arena, bullets, player, enemies, dt);
    // spawn bullets
    if (input.firing) {
        spawn_bullet(bullets, player.world_pos, input.mouse_dir, Team::PLAYER);
    }
    if (input.interract) {
        arena.close_door(arena.vec2_to_tile(player.world_pos), closed_door_tiles);
    }
    update_bullets(arena, bullets, player, enemies, dt);
    remove_dead_entities(bullets, enemies);
    update_artefacts(artefact_sets, player);
}

void draw_fov_cone(Enemy& enemy, Arena& arena) {
    float half_fov = enemy.perceptions.view_angle_rads / 2.f;
    float dir_ang = std::atan2f(enemy.forward.y, enemy.forward.x);
    std::vector<Vector2> points;
    points.push_back({enemy.world_pos.x, enemy.world_pos.y});
    for (float angle = -half_fov; angle <= half_fov; angle += 0.05f) {
        Vector2 point = {
            enemy.world_pos.x + std::cosf(dir_ang + angle) * enemy.perceptions.max_distance,
            enemy.world_pos.y + std::sinf(dir_ang + angle) * enemy.perceptions.max_distance};
        // todo - raymarch and check if a wall is there, then stop drawing the line at that point
        points.push_back(point);
    }

    DrawLineV({enemy.world_pos.x, enemy.world_pos.y},
              {enemy.world_pos.x + enemy.forward.x * enemy.perceptions.max_distance,
               enemy.world_pos.y + enemy.forward.y * enemy.perceptions.max_distance},
              RED);
    DrawTriangleFan(points.data(), (int)points.size(), Fade(YELLOW, 0.3f));
    DrawTriangleStrip(points.data(), (int)points.size(), Fade(YELLOW, 0.3f));
}
constexpr const char* ENEMY_STATE_STRINGS[] = {"PATROL",      "CHASE",  "TAKE COVER",
                                               "HEAL",        "ENGAGE", "GUARD",
                                               "INVESTIGATE", "FLEE",   "RETURN TO POST"};

const char* get_state(EnemyState state) {
    return ENEMY_STATE_STRINGS[std::bit_cast<int>(state)];
};

void draw_artefacts(std::array<ArtefactSet, 6>& artefact_sets) {
    for (const auto& af_set : artefact_sets) {
        for (int i = 0; i < 3; i++) {
            const Artefact& af = af_set.artefacts[i];
            if (af.collected) {
                continue;
            }
            Vec2 pos = af.world_pos;
            Vec2 top_left = centre_to_top_left(pos, 32.f);
            DrawTexture(af_set.tex, top_left.x, top_left.y, WHITE);
        }
    }
};

void spawn_enemy_A(std::vector<Enemy>& enemies, Arena& arena, Player& player) {
    Vec2 random_spawn = arena.random_pos();
    Enemy enemy{};
    enemy.world_pos = random_spawn;
    enemy.velocity = {350, 350};
    enemy.size = 30.f;
    enemy.dead = false;
    enemy.health = 100;
    enemy.perceptions.spotted_player = false;
    enemy.forward = {1.0f, 0.f};
    enemy.type = EnemyType::CHASER;
    enemy.setup_patrol_paths(arena);
    enemy.fsm.on_enter(enemy, arena, player);
    enemies.push_back(enemy);
}
void spawn_enemy_B(std::vector<Enemy>& enemies, Arena& arena, Player& player) {
    for (int i = 0; i < 3; i++) {
        Enemy enemy{};
        enemy.guard_zone = i;
        enemy.world_pos = arena.tile_to_world(enemy.guard_zones[enemy.guard_zone]);
        enemy.velocity = {350, 350};
        enemy.size = 30.f;
        enemy.dead = false;
        enemy.health = 100;
        enemy.perceptions.spotted_player = false;
        enemy.forward = {1.0f, 0.f};
        enemy.type = EnemyType::GUARD;
        enemy.current_flee_zone = enemy.guard_zone + 1;
        // enemy.setup_patrol_paths(arena);
        enemy.fsm.state = EnemyState::GUARD;
        enemy.fsm.on_enter(enemy, arena, player);
        enemies.push_back(enemy);
    }
}

Texture2D closed_door;

void draw_player_triangle(Player& player, Vec2 mouse_dir) {
    float half = player.size / 2;
    float gap = 10.0f;
    Vec2 orthogonal = {-mouse_dir.y, mouse_dir.x};
    Vec2 tip = player.world_pos + mouse_dir * (half + gap);
    Vec2 top = player.world_pos + mouse_dir * half - orthogonal * (half / 2);
    Vec2 bottom = player.world_pos + mouse_dir * half + orthogonal * (half / 2);
    DrawTriangle({tip.x, tip.y}, {top.x, top.y}, {bottom.x, bottom.y}, RED);
}

void draw_enemy_triangles(std::vector<Enemy>& enemies) {
    float half = enemies[0].size / 2;
    float gap = 10.f;
    for (Enemy& enemy : enemies) {
        Vec2 orth = {-enemy.forward.y, enemy.forward.x};
        Vec2 tip = enemy.world_pos + enemy.forward * (half + gap);
        Vec2 top = enemy.world_pos + enemy.forward * half - orth * (half / 2);
        Vec2 bottom = enemy.world_pos + enemy.forward * half + orth * (half / 2);
        DrawTriangle({tip.x, tip.y}, {top.x, top.y}, {bottom.x, bottom.y}, BLACK);
    }
}

void draw_closed_doors(Arena& arena, std::vector<int>& open_tiles) {
    for (int open_tile : open_tiles) {
        Vec2 world_pos = arena.tile_to_world(open_tile);
        Vec2 top_left = centre_to_top_left(world_pos, 32);
        DrawTexture(closed_door, top_left.x, top_left.y, WHITE);
    }
}
int main() {
    SetTargetFPS(120);
    // get path realtive to build-dir
    const auto csv = std::filesystem::path(GetApplicationDirectory()) / "assets" / "map_binary.csv";
    Arena arena(csv.string());
    const auto arena_tex =
        std::filesystem::path(GetApplicationDirectory()) / "assets" / "arena.png";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GameAI");

    // gameplay setup
    Player player{};
    player.world_pos = {50, 150};
    player.velocity = {550, 550};
    player.size = 25.f;
    player.health = 1000.f;
    player.team = Team::PLAYER;

    std::vector<Enemy> enemies;
    spawn_enemy_A(enemies, arena, player);
    spawn_enemy_A(enemies, arena, player);
    enemies[0].setup_guard_zones(arena);

    spawn_enemy_B(enemies, arena, player);

    // setup artefacts
    std::array<ArtefactSet, 6> artefact_tiles = generate_artefacts(arena, player, enemies);

    auto cur_time = std::chrono::steady_clock::now();
    enemies[0].last_shot_at = cur_time;

    std::vector<Bullet> bullets;

    // debug = enemy paths and FOV visualisation
    bool draw_debug = false;

    // todo - clamp camera to map edges
    Camera2D camera = {0};
    camera.zoom = 1.f;
    camera.offset = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

    auto tex = LoadTexture(arena_tex.string().c_str());
    const auto closed_door_tex =
        std::filesystem::path(GetApplicationDirectory()) / "assets" / "door_close.png";
    closed_door = LoadTexture(closed_door_tex.string().c_str());
    std::vector<int> open_door_tiles;
    float draw_hit_timer = 0.0f;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            arena.grid[25][15] = !arena.grid[25][15];
        }
        if (IsKeyPressed(KEY_B)) {
            draw_debug = !draw_debug;
        }
        if (IsKeyPressed(KEY_Q)) {
            spawn_enemy_A(enemies, arena, player);
        }

        float dt = GetFrameTime();
        input_state input = handle_input(camera, player);
        update(arena, dt, player, enemies, bullets, input, artefact_tiles, open_door_tiles);
        if (player.crit_timer > 0.f) {
            player.crit_timer -= dt;
        }
        // render
        BeginDrawing();
        ClearBackground({0, 0, 0, 0});
        camera.target = {player.world_pos.x, player.world_pos.y};
        BeginMode2D(camera);

        //  todo - stop drawing the grid every frame, draw it once into a texture,
        DrawTexture(tex, 0, 0, {WHITE});
        draw_artefacts(artefact_tiles);
        draw_closed_doors(arena, open_door_tiles);
        for (Bullet& b : bullets) {
            DrawCircle(b.world_pos.x, b.world_pos.y, b.size, BLACK);
        }
        Vec2 player_top_left = player.world_pos - Vec2{player.size / 2, player.size / 2};

        if (player.crit_hit) {
            std::cout << "CRIT";
        }
        if (player.crit_timer > 0.f) {
            DrawText(TextFormat("--%.0f", player.last_dmg), player.world_pos.x, player.world_pos.y,
                     25, RED);
        }

        DrawRectangleV(*reinterpret_cast<Vector2*>(&player_top_left), {player.size, player.size},
                       BLUE);
        for (int i = 0; i < enemies.size(); i++) {
            auto& enemy = enemies[i];
            Vec2 enemy_top_left = enemy.world_pos - Vec2{enemy.size / 2, enemy.size / 2};
            DrawRectangleV(*reinterpret_cast<Vector2*>(&enemy_top_left), {enemy.size, enemy.size},
                           colours[(i + 1) % colours.size()]);
            DrawText(TextFormat("%i", (int)enemy.health), enemy.world_pos.x + enemy.size / 2,
                     enemy.world_pos.y - 50, 40, GREEN);
            DrawText(TextFormat("%s", get_state(enemy.fsm.state)),
                     enemy.world_pos.x + enemy.size / 2, enemy.world_pos.y - 10, 40, GREEN);
        }
        DrawRectangleLines(player_top_left.x, player_top_left.y, player.size, player.size,
                           RAYWHITE);
        draw_player_triangle(player, input.mouse_dir);
        draw_enemy_triangles(enemies);
        if (draw_debug) {
            draw_grid(arena);
            for (Enemy& enemy : enemies) {
                draw_fov_cone(enemy, arena);
                for (auto waypoint_pos : enemy.cur_path) {
                    DrawCircle(waypoint_pos.x, waypoint_pos.y, 2.2, RED);
                }
            }
        }
        std::cout << player.health << "\n";
        EndMode2D();

        EndDrawing();
        DrawText(TextFormat("health:%f", player.health), 0, 30, 25, RED);
        if (!enemies.empty()) {
            const Enemy& e = enemies[0];
            DrawText(TextFormat("STATE: %d | GOAL: %d | WP: %d/%d | SEEN: %d | RANGE: %d",
                                static_cast<int>(e.fsm.state), e.goal_tile, e.cur_waypoint,
                                static_cast<int>(e.cur_path.size()), e.perceptions.spotted_player,
                                e.perceptions.in_range),
                     10, 10, 10, GREEN);
        }
    }

    CloseWindow();
    return 0;
}
