#pragma once
#include "utils.h"

constexpr int TILE_SIZE = 32;


struct Arena {
	int cols;
	int rows;
	std::vector<std::vector<int>> grid;
	int map_height;
	int map_width;
	// each row has cols entries. row = y, col = x.
	Arena(const std::string& csv_path);
	Arena(int cols_, int rows_) : cols(cols_), rows(rows_), grid(rows, std::vector<int>(cols, 0)), map_height(rows* TILE_SIZE), map_width(cols* TILE_SIZE) {};
	int tile(int r, int c);
	bool isWall(int _row, int _col);
	//bool isObstacle(int tile_idx) {
	//	return tile_idx in set of obstacles
	//}
};

