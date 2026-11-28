#pragma once
#include "arena.h"
#include "math_utils.h"
#include <queue>
#include <unordered_map>
#include <vector>

int heuristic(int start, int target, int cols);
std::vector<int> build_path(std::vector<int> &parents, int node);
std::vector<std::pair<int, int>> neighbours(Arena &arena, int tile);
std::vector<int> astar(Arena &arena, int start_node, int target_node);
std::vector<int> BFS(Arena &arena, int start_node, int target_node);
std::vector<int> dijkstra(Arena &arena, int start_node, int target_node);
std::vector<int> neighbours_unweighted(Arena &arena, int tile);
std::vector<std::pair<int, int>> neighbours(Arena &arena, int tile);
