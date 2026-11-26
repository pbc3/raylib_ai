#pragma once
#include "raylib.h"
#include "arena.h"
#include "math_utils.h"
#include "player.h"
#include "pathfinding.h"



struct Artefact{
    Vec2 world_pos;
    bool collected;
};

struct ArtefactSet{
    Texture2D tex;
    std::array<Artefact,3> artefacts;
};

std::vector<Artefact> generate_artefacts(Arena& arena, Player& player);