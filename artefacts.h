#pragma once
#include "raylib.h"
#include "arena.h"
#include "math_utils.h"
#include "player.h"
#include "enemy.h"
#include "pathfinding.h"

struct Artefact{
    Vec2 world_pos;
    bool collected;
};

struct ArtefactSet{
    Texture2D tex;
    std::array<Artefact,3> artefacts;
};

std::array<ArtefactSet,6> generate_artefacts(Arena& arena, Player& player,std::vector<Enemy>& enemies);
std::vector<Vec2> generate_artefact_locations(Arena& arena, Player& player, std::vector<Enemy>& enemies);
bool neighbours_contain_obstacles(Arena& arena, int tile);
