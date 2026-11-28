
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
#include "game_level.h"
#include "pathfinding.h"
#include "player.h"
#include "renderer.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int ARENA_WIDTH = SCREEN_WIDTH / TILE_SIZE;   // screen x / tile
constexpr int ARENA_HEIGHT = SCREEN_HEIGHT / TILE_SIZE; // screen y / tile

int main() {

    const auto csv = std::filesystem::path(GetApplicationDirectory()) / "assets" / "arena.csv";
    Arena arena(csv.string());
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GameAI");
    //   SetTargetFPS(144);
    // gameplay setup
    GameLevel level{.arena = arena};
    level.restart_level();
    // debug = enemy paths and FOV visualisation
    bool draw_debug = false;

    Camera2D camera = {0};
    camera.zoom = 1.25f;
    camera.offset = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};

    const auto arena_tex =
        std::filesystem::path(GetApplicationDirectory()) / "assets" / "arena.png";
    const auto closed_door_tex =
        std::filesystem::path(GetApplicationDirectory()) / "assets" / "door_close.png";

    Renderer renderer;
    renderer.load_textures(arena_tex.string().c_str(), closed_door_tex.string().c_str());

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_R)) {
            level.restart_level();
        }
        if (IsKeyPressed(KEY_B)) {
            draw_debug = !draw_debug;
        }
        if (IsKeyPressed(KEY_Q)) {
            spawn_enemy_A(level.enemies, arena, level.player);
        }
        float dt = GetFrameTime();
        InputState input = handle_input(camera, level.player);
        // update(arena, dt, player, enemies, bullets, input, artefact_tiles,
        // open_door_tiles);
        if (level.player.dead || level.enemies.empty()) {
            level.restart_level();
        }
        level.update(arena, dt, level, input);
        if (level.player.crit_timer > 0.f) {
            level.player.crit_timer -= dt;
        }
        camera.target = {level.player.world_pos.x, level.player.world_pos.y};
        // render
        renderer.render(level, camera, input.mouse_dir, draw_debug);
    }

    CloseWindow();
    return 0;
}
