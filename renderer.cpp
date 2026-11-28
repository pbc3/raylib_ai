#include "renderer.h"
#include "game_level.h"
#include <algorithm>
#include <cmath>

const char* get_state(EnemyState state) {
    return ENEMY_STATE_STRINGS[static_cast<int>(state)];
};

Color get_colour(EnemyState state) {
    return ENEMY_STATE_COLOURS[(int)state];
}

void draw_enemies(Arena& arena, std::vector<Enemy>& enemies, bool debug) {
    int colour_idx = 0;
    for (Enemy& enemy : enemies) {
        draw_enemy_info(enemy);
        Vec2 enemy_top_left = centre_to_top_left(enemy.world_pos, enemy.size);
        DrawRectangleV(*reinterpret_cast<Vector2*>(&enemy_top_left), {enemy.size, enemy.size},
                       ENEMY_STATE_COLOURS[colour_idx]);
        colour_idx++;
        if (debug) {
            draw_fov_cone(enemy, arena);
            for (auto waypoint_pos : enemy.cur_path) {
                DrawCircle(waypoint_pos.x, waypoint_pos.y, 2.2, RED);
            };
        }
    }
};

void Renderer::render(GameLevel& level, Camera2D& camera, Vec2 mouse_dir, bool debug) {
    BeginDrawing();
    ClearBackground({100, 125, 150, 255});
    //  camera.target = {level.player.world_pos.x, level.player.world_pos.y};
    BeginMode2D(camera);
    DrawTexture(arena, 0, 0, WHITE);
    Vec2 player_top_left =
        level.player.world_pos - Vec2{level.player.size / 2, level.player.size / 2};

    if (level.player.crit_timer > 0.f) {
        DrawText(TextFormat("--%.0f", level.player.last_dmg), level.player.world_pos.x,
                 level.player.world_pos.y, 25, RED);
    }
    DrawRectangleV(*reinterpret_cast<Vector2*>(&player_top_left),
                   {level.player.size, level.player.size}, BLUE);
    if (debug) {
        draw_grid(level.arena);
    }
    draw_enemies(level.arena, level.enemies, debug);
    DrawRectangleLines(player_top_left.x, player_top_left.y, level.player.size, level.player.size,
                       RAYWHITE);
    draw_player_triangle(level.player, mouse_dir);
    draw_enemy_triangles(level.enemies);
    DrawText(TextFormat("health:%f", level.player.health), 0, 30, 25, RED);

    draw_artefacts(level.artefact_tiles);
    draw_closed_doors(*this, level.arena, level.door_tiles);
    for (Bullet& b : level.bullets) {
        DrawCircle(b.world_pos.x, b.world_pos.y, b.size, BLACK);
    }
    EndMode2D();
    DrawText(TextFormat("FPS:%i", GetFPS()), 0, 50, 25, RED);

    EndDrawing();
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
    if (enemies.empty())
        return;
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

void draw_closed_doors(Renderer& renderer, Arena& arena, std::vector<int>& open_tiles) {
    for (int open_tile : open_tiles) {
        Vec2 world_pos = arena.tile_to_world(open_tile);
        Vec2 top_left = centre_to_top_left(world_pos, 32);
        DrawTexture(renderer.closed_door, top_left.x, top_left.y, WHITE);
    }
}

void draw_health_bars(Enemy& enemy) {
    static float width = 40.f;
    static float height = 6.f;
    float x = enemy.world_pos.x - width / 2;
    float y = enemy.world_pos.y - enemy.size / 2 - 12.f;
    float health_fill = std::clamp(enemy.health / 100.f, 0.f, 1.f);
    DrawRectangleRec({x, y, width, height}, DARKGRAY);
    DrawRectangleRec({x, y, width * health_fill, height}, GREEN);
    DrawRectangleLines(x, y, width, height, BLACK);
}

void draw_enemy_info(Enemy& enemy) {
    static float width = 40.f;
    static float height = 6.f;
    float x = enemy.world_pos.x - width / 2;
    float y = enemy.world_pos.y - enemy.size / 2 - 12.f;
    float health_fill = std::clamp(enemy.health / 100.f, 0.f, 1.f);
    DrawRectangleRec({x, y, width, height}, DARKGRAY);
    DrawRectangleRec({x, y, width * health_fill, height}, GREEN);
    DrawRectangleLines(x, y, width, height, BLACK);
    const char* state = get_state(enemy.fsm.state);
    Color state_colour = get_colour(enemy.fsm.state);
    DrawText(TextFormat("%s ", state), x, y - 20, 20, state_colour);
    DrawText(TextFormat("ID : %i", enemy.id), x, y - 40, 25, RED);
}

// for debug only
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
        }
    }

    for (int i = 0; i <= arena.rows; i++) {
        DrawLine(0, i * TILE_SIZE, arena.map_width, i * TILE_SIZE, GRID_COLOUR);
    }

    for (int j = 0; j <= arena.cols; j++) {
        DrawLine(j * TILE_SIZE, 0, j * TILE_SIZE, arena.map_height, GRID_COLOUR);
    }

    DrawRectangleLines(0, 0, arena.map_width, arena.map_height, {141, 169, 196, 255});
};