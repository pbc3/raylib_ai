#pragma once

#include "arena.h"
#include "artefacts.h"
#include "bullet.h"
#include "enemy.h"
#include "math_utils.h"
#include "player.h"
#include "raylib.h"

#include <array>
#include <bit>
#include <filesystem>
#include <vector>
constexpr const char* ENEMY_STATE_STRINGS[] = {"PATROL",      "CHASE",  "TAKE COVER",
                                               "HEAL",        "ENGAGE", "GUARD",
                                               "INVESTIGATE", "FLEE",   "RETURN TO POST"};

constexpr Color ENEMY_STATE_COLOURS[] = {YELLOW, ORANGE, SKYBLUE, GREEN,  RED,
                                         YELLOW, GOLD,   PURPLE,  SKYBLUE};

constexpr std::array<Color, 3> GUARD_COLOURS = {RED, GOLD, BLACK};
constexpr std::array<Color, 2> CHASER_COLOURS = {LIME, PURPLE};

Color get_colour(EnemyState state);
const char* get_state(EnemyState state);
struct GameLevel;
constexpr Color GRID_COLOUR = {48, 59, 72, 255};
struct Renderer {
    Texture2D arena;
    Texture2D closed_door;
    void load_textures(const char* arena_path, const char* door_path) {
        arena = LoadTexture(arena_path);
        closed_door = LoadTexture(door_path);
        SetTextureFilter(arena, TEXTURE_FILTER_BILINEAR);
        SetTextureFilter(closed_door, TEXTURE_FILTER_BILINEAR);
    }
    void render(GameLevel& level, Camera2D& camera, Vec2 mouse_dir, bool debug);
};

void draw_grid(Arena& arena);
void draw_closed_doors(Renderer& renderer, Arena& arena, std::vector<int>& open_tiles);
void draw_artefacts(std::array<ArtefactSet, 6>& artefact_sets);
void draw_player_triangle(Player& player, Vec2 mouse_dir);
void draw_enemy_triangles(std::vector<Enemy>& enemies);
void draw_health_bars(Enemy& enemy);
void draw_enemy_info(Enemy& enemy);
void draw_fov_cone(Enemy& enemy, Arena& arena);
void draw_enemies(Arena& arena, std::vector<Enemy>& enemies, bool debug);

void render(GameLevel& level, Camera2D camera, Vec2 mouse_dir, bool debug);
