#include "pathfinding.h"
#include <algorithm>

int heuristic(int start, int target, int cols) {
    // manhattan distance = | x1 - x2 | + |y1 - y2|
    int row1 = start / cols;
    int col1 = start % cols;
    int row2 = target / cols;
    int col2 = target % cols;

    return std::abs(row1 - row2) + std::abs(col1 - col2);
};

std::vector<int> build_path(std::vector<int>& parents, int node) {
    std::vector<int> path;
    // assumes default parents -1
    while (node != -1) {
        path.push_back(node);
        node = parents[node];
    }
    std::reverse(path.begin(), path.end());
    return path;
};

std::vector<std::pair<int, int>> neighbours(Arena& arena, int tile) {
    std::vector<std::pair<int, int>> neighbour_tiles;

    auto [row, col] = arena.tile_to_row_col(tile);
    int tile1 = arena.flatten(row + 1, col);
    int tile2 = arena.flatten(row - 1, col);
    int tile3 = arena.flatten(row, col + 1);
    int tile4 = arena.flatten(row, col - 1);

    // update cost based on ter1in type
    if (arena.isWalkable(tile1)) {
        neighbour_tiles.push_back({arena.terrain_cost(row + 1, col), tile1});
    }
    if (arena.isWalkable(tile2)) {

        neighbour_tiles.push_back({arena.terrain_cost(row - 1, col), tile2});
    }
    if (arena.isWalkable(tile3)) {

        neighbour_tiles.push_back({arena.terrain_cost(row, col + 1), tile3});
    }
    if (arena.isWalkable(tile4)) {
        neighbour_tiles.push_back({arena.terrain_cost(row, col - 1), tile4});
    }

    return neighbour_tiles;
}

// here - im working with flattened indices, even though my array is 2D.
std::vector<int> astar(Arena& arena, int start_node, int target_node) {
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>>
        fringe;

    std::vector<int> parents(arena.rows * arena.cols, -1);
    std::unordered_map<int, int> cost_so_far;

    fringe.push({heuristic(start_node, target_node, arena.cols), start_node});
    parents[start_node] = -1;
    cost_so_far[start_node] = 0;

    while (!fringe.empty()) {
        auto [priority, node] = fringe.top();
        fringe.pop();

        if (priority != cost_so_far[node] + heuristic(node, target_node, arena.cols)) {
            continue; // there can be >1 pairs with the same node, but different
                      // costs. this confirms its still the best one, else its skipped
        }

        if (node == target_node) {
            return build_path(parents, node);
        }

        for (auto [edge_cost, candidate] : neighbours(arena, node)) {
            auto [row, col] = arena.tile_to_row_col(candidate);
            // if(arena.isWall(row,col)){
            //   continue;
            // }

            int new_cost = edge_cost + cost_so_far.at(node); // g(n);

            auto it = cost_so_far.find(candidate);
            if (it == cost_so_far.end() || new_cost < it->second) {
                cost_so_far[candidate] = new_cost;
                parents[candidate] = node;
                int new_priority = new_cost + heuristic(candidate, target_node,
                                                        arena.cols); // total cost = g(n) + h(n)
                fringe.push({new_priority, candidate});
            }
        }
    }

    return {};
}

std::vector<int> dijkstra(Arena& arena, int start_node, int target_node) {
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>>
        fringe;

    std::vector<int> parents(arena.rows * arena.cols, -1);
    std::unordered_map<int, int> cost_so_far;

    fringe.push({0, start_node});
    parents[start_node] = -1;
    cost_so_far[start_node] = 0;

    while (!fringe.empty()) {
        auto [priority, node] = fringe.top();
        fringe.pop();

        if (priority != cost_so_far[node]) {
            continue; // there can be >1 pairs with the same node, but different
                      // costs. this confirms its still the best one, else its skipped
        }

        if (node == target_node) {
            return build_path(parents, node);
        }

        for (auto [edge_cost, candidate] : neighbours(arena, node)) {
            auto [row, col] = arena.tile_to_row_col(candidate);
            // if(arena.isWall(row,col)){
            //   continue;
            // }

            int new_cost = edge_cost + cost_so_far.at(node); // g(n);

            auto it = cost_so_far.find(candidate);
            if (it == cost_so_far.end() || new_cost < it->second) {
                cost_so_far[candidate] = new_cost;
                parents[candidate] = node;
                int new_priority = new_cost; // total cost = g(n)
                fringe.push({new_priority, candidate});
            }
        }
    }

    return {};
}

std::vector<int> neighbours_unweighted(Arena& arena, int tile) {
    std::vector<int> neighbour_tiles;

    auto [row, col] = arena.tile_to_row_col(tile);
    int tile1 = arena.flatten(row + 1, col);
    int tile2 = arena.flatten(row - 1, col);
    int tile3 = arena.flatten(row, col + 1);
    int tile4 = arena.flatten(row, col - 1);
    if (arena.isWalkable(tile1)) {
        neighbour_tiles.push_back(tile1);
    }
    if (arena.isWalkable(tile2)) {
        neighbour_tiles.push_back(tile2);
    }
    if (arena.isWalkable(tile3)) {
        neighbour_tiles.push_back(tile3);
    }
    if (arena.isWalkable(tile4)) {
        neighbour_tiles.push_back(tile4);
    }

    return neighbour_tiles;
}

std::vector<int> BFS(Arena& arena, int start_node, int target_node) {
    std::queue<int> frontier;
    std::vector<int> parents(arena.cols * arena.rows, -1);
    std::vector<int> visited(arena.cols * arena.rows, 0);
    visited[start_node] = 1;
    parents[start_node] = -1;
    frontier.push(start_node);

    while (!frontier.empty()) {
        int node = frontier.front();
        frontier.pop();

        if (node == target_node) {
            return build_path(parents, node);
        }
        for (int neighbour : neighbours_unweighted(arena, node)) {
            auto [row, col] = arena.tile_to_row_col(neighbour);
            // if(arena.isWall(row,col)){
            //   continue;
            // }
            if (!visited[neighbour]) {
                visited[neighbour] = true;
                frontier.push(neighbour);
                parents[neighbour] = node;
            }
        }
    }
    return {};
}
