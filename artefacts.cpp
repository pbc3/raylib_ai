#include "artefacts.h"

std::array<ArtefactSet, 6> generate_artefacts(Arena& arena, Player& player,
                                              std::vector<Enemy>& enemies) {
    std::array<const char*, 6> texture_names = {"assets/diamond.png", "assets/potion.png",
                                                "assets/skull.png",   "assets/stave.png",
                                                "assets/book.png",    "assets/scroll.png"};
    std::array<ArtefactSet, 6> artefact_sets;

    // first - generate locations
    auto locations = generate_artefact_locations(arena, player, enemies);

    // setup sets for each type of artefact
    int outer_idx = 0;
    int inner_idx = 0;
    for (ArtefactSet& artefact_set : artefact_sets) {
        auto tex = LoadTexture(texture_names[outer_idx]);
        artefact_set.tex = tex;
        for (int i = 0; i < 3; i++) {
            Artefact af{};
            af.world_pos = locations[inner_idx];
            inner_idx++;
            artefact_set.artefacts[i] = af;
        }
        outer_idx++;
    }
    return artefact_sets;
}

std::vector<Vec2> generate_artefact_locations(Arena& arena, Player& player,
                                              std::vector<Enemy>& enemies) {
    std::vector<Vec2> locations;
    while (locations.size() < 18) {
        Vec2 candidate = arena.random_pos();
        int tile = arena.vec2_to_tile(candidate);
        candidate = arena.tile_to_world(tile); // centre it
        // reject walls & obstacles
        if (!arena.isSuitableArtefact(tile)) {
            continue;
        }
        // minimum spacing

        bool safe_loc = true;
        for (Vec2 loc : locations) {
            if (loc.distance(candidate) < 128.f) {
                safe_loc = false;
                break;
            }
        }
        if (!safe_loc) {
            continue;
        }

        // dont spawn a artefact next to walls
        if (neighbours_contain_obstacles(arena, tile)) {
            continue;
        }

        // dont spawn within a distance of the players spawn
        Vec2 to_player = player.world_pos - candidate;
        if (to_player.length() < 128.f) {
            continue;
        }
        // dont spawn on a enemy
        for (const Enemy& enemy : enemies) {
            if (candidate.distance(enemy.world_pos) < 128.f) {
                safe_loc = false;
                break;
            }
        }
        if (!safe_loc) {
            continue;
        }

        // accessibility verification
        int player_tile = arena.vec2_to_tile(player.world_pos);
        auto path = astar(arena, player_tile, tile);
        if (path.empty()) {
            continue;
        }

        locations.push_back(candidate);
    }
    return locations;
}

bool neighbours_contain_obstacles(Arena& arena, int tile) {
    auto [row, col] = arena.tile_to_row_col(tile);
    int tile1 = arena.flatten(row + 1, col);
    int tile2 = arena.flatten(row - 1, col);
    int tile3 = arena.flatten(row, col + 1);
    int tile4 = arena.flatten(row, col - 1);
    if (!arena.isSuitableArtefact(tile1) || !arena.isSuitableArtefact(tile2) ||
        !arena.isSuitableArtefact(tile3) || !arena.isSuitableArtefact(tile4)) {
        return true;
    }
    return false;
}