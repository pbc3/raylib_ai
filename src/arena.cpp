#include "arena.h"
#include <exception>
#include <stdexcept>

Arena::Arena(const std::string& csv_path) {
	auto res = parse_csv(csv_path);
	if (!res.has_value()) {
		// error
		throw std::runtime_error("csv parse failed");
	}
	if (auto& vec = res.value(); vec.empty()) {
		// empty vec, handle?
		throw std::logic_error("grid vector is empty");
	}

	//grid = res.value();

	grid = std::move(res.value());

	int r = grid.size();
	int c = grid[r - 1].size(); // assume size is even across all  rows..

	cols = c; rows = r;
	map_height = rows * TILE_SIZE;
	map_width = cols * TILE_SIZE;
}


bool Arena::isWall(int _row, int _col) {
	int tile_val = tile(_row, _col);
	return tile_val != 0 ? true : false;
}
int Arena::tile(int r, int c) {
	if (r >= rows || r < 0 || c >= cols || c < 0) {
		//__debugbreak();
		return -1; // treat oob as a wall
	}
	return grid[r][c];
}


