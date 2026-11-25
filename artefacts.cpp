#include "artefacts.h"

std::vector<int> generate_artefacts(Arena& arena, Player& player){
    // return from a vector of tile locations for the artefacts
    // to_do - store random generator inside arena, construct it on creation, and give me a Vec2 random_location function, also rename iswall to is obstacle
    // basic first - just make them reachable from the player, and not a wall
    float max_x = arena.cols * TILE_SIZE;
    float max_y = arena.rows * TILE_SIZE;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist_x(0.f,max_x);
    std::uniform_real_distribution<float> dist_y(0.f,max_y);
    
    std::vector<int> locations;
    while(locations.size() <= 18){
        Vec2 candidate = {dist_x(gen), dist_y(gen)};
        int tile = arena.vec2_to_tile(candidate);
        // reject walls & obstacles
        if(arena.isWall(tile)){
            continue;
        }

        // can the player reach it? is this needed? 
        int player_tile = arena.vec2_to_tile(player.world_pos);
        auto path = astar(arena,player_tile,tile);
        if(path.empty()){
            continue;
        }
        locations.push_back(tile);
    }
    return locations;
}
