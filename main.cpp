#include <raylib.h>
#include <iostream>
#include <vector>
#include <algorithm>



struct Vec2 {
	float x, y;

	float length() {
		return std::hypot(x, y);
	}
	void normalise() {
		float len = length();
		//if (IsNearlyZero(len)) {
		//	return;
		//}
		if (!len) {
			return;
		}
		this->x /= len;
		this->y /= len;
	}
};
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <charconv>

std::vector<int> parse_ints_from_line(std::string_view line) {
	std::vector<int> vals;

	while (!line.empty()) {
		while (!line.empty() &&
			(std::isspace(static_cast<unsigned char>(line.front())) ||
				std::isalpha(static_cast<unsigned char>(line.front())) ||
				(std::ispunct(static_cast<unsigned char>(line.front())) && line.front() != '-')))
		{
			line.remove_prefix(1);
		}

		if (line.empty()) {
			break;
		}

		int value;
		auto [ptr, errorcode] = std::from_chars(line.data(), line.data() + line.size(), value);

		if ((errorcode != std::errc())) {
			// TODO - HANDLE
			break;
		}
		vals.push_back(value);

		// advance the ptr past what we just parsed
		line.remove_prefix(ptr - line.data());
	}
	return vals;
}
std::optional<std::vector<std::vector<int>>> parse_csv(std::string path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		return std::nullopt;
	}

	std::vector<std::vector<int>> grid;

	//std::string total;
	////while (std::getline(file, line)) {

	////	total += line + "\n";
	////}


	int row_idx = 0;
	std::string line;

	while (std::getline(file, line)) {
		// take all the numbers
		std::vector<int> tiles = parse_ints_from_line(line);
		//grid[row_idx] = tiles;
		grid.push_back(tiles);
		row_idx++;
	}
	return grid;

}

//std::vector<char> load_file(std::string_view path) {
//
//	// read file size
//
//	// virtualloc that ..? 
//
//	// return
//}

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 704;
constexpr int TILE_SIZE = 32;
constexpr int ARENA_WIDTH = SCREEN_WIDTH / TILE_SIZE; // screen x / tile
constexpr int ARENA_HEIGHT = SCREEN_HEIGHT / TILE_SIZE; // screen y / tile
constexpr Color GRID_COLOUR = { 48, 59, 72, 255 };

// tile centring, possibly needed.
//constexpr int OFFSET_X = (SCREEN_WIDTH - ARENA_WIDTH * TILE_SIZE) / 2;
//constexpr int OFFSET_Y = (SCREEN_HEIGHT - ARENA_HEIGHT * TILE_SIZE) / 2;



void draw_arena() {
	for (int i = 0; i < ARENA_HEIGHT; i++) {
		for (int j = 0; j < ARENA_WIDTH; j++) {
			// x = column * 50
			// y = row * 50
			int x = j * TILE_SIZE;
			int y = i * TILE_SIZE;
			
			DrawRectangleLines(x, y, TILE_SIZE, TILE_SIZE, BLUE);
		}
	}
	return;
}
struct Arena {
	
	int cols;
	int rows;
	std::vector<std::vector<int>> grid;
	int map_height;
	int map_width;
	// each row has cols entries. row = y, col = x.
	Arena(std::string csv_path) {
		auto res = parse_csv(csv_path);
		if (!res.has_value()) {
			// error
			throw std::exception("csv parse failed");
		}
		// assume rows and cols?
		auto _grid = res.value();
		// todo - copy? 
		grid = _grid;
		int r = grid.size();
		int c = grid[r - 1].size(); // assume size is even across all  rows..

		cols = c; rows = r;
		map_height = rows * TILE_SIZE;
		map_width = cols * TILE_SIZE;

	}
	Arena(int cols_, int rows_) : cols(cols_), rows(rows_), grid(rows, std::vector<int>(cols, 0)) {
		map_height = rows * TILE_SIZE;
		map_width = cols * TILE_SIZE;
	};
	int tile(int r, int c) {
		if (r >= rows || r < 0 || c >= cols || c < 0) {
			//__debugbreak();
			return -1;
		}
		return grid[r][c];
	}
	bool isWall(int _row, int _col) {
		int tile_idx = tile(_row, _col);
		return tile_idx == 1 ? true : false;
	}
	//bool isObstacle(int tile_idx) {
	//	return tile_idx in set of obstacles
	//}
	//std::vector<std::vector<int>> grid(22, std::vector<int>(40, 0));
};
void draw_grid(Arena& arena) {
	for (int i = 0; i < arena.rows; i++) {
		for (int j = 0; j < arena.cols; j++) {
			// x = column * 50
			// y = row * 50
			int x = j * TILE_SIZE;
			int y = i * TILE_SIZE;
			if (arena.isWall(i, j)) {
				DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, { 100, 125, 150, 255 });
			}
			//if (arena.grid[i][j] == 1) {
			//	DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, { 100, 125, 150, 255 });
			//}
		}
	}

	// <= because we want to draw edges
	for (int i = 0; i <= arena.rows; i++) {
		DrawLine(0, i * TILE_SIZE,arena.map_width, i * TILE_SIZE,GRID_COLOUR);
	}

	for (int j = 0; j <= arena.cols; j++) {
		DrawLine(j * TILE_SIZE, 0,j * TILE_SIZE, arena.map_height,GRID_COLOUR);
	}

	DrawRectangleLines(0, 0, arena.map_width, arena.map_height, { 141, 169, 196, 255 });
}

struct input_state {
	bool move_up;
	bool move_down;
	bool move_left;
	bool move_right;
};
input_state handle_input() {

	//bool IsKeyPressed(int key);                             // Check if key has been pressed once
	//bool IsKeyPressedRepeat(int key);                       // Check if key has been pressed again
	//bool IsKeyDown(int key);                                // Check if key is being pressed
	//bool IsKeyReleased(int key);                            // Check if key has been released once
	//bool IsKeyUp(int key);                                  // Check if key is NOT being pressed
	//int GetKeyPressed(void);                                // Get key pressed (keycode), call it multiple times for keys queued, returns 0 when the queue is empty
	//int GetCharPressed(void);                               // Get char pressed (unicode), call it multiple times for chars queued, returns 0 when the queue is empty
	//const char* GetKeyName(int key);                        // Get name of a QWERTY key on the current keyboard layout (eg returns string 'q' for KEY_A on an AZERTY keyboard)
	input_state input{ false,false,false,false };
	if (IsKeyDown(KEY_W)) {
		//player_pos.y -= 1;
		input.move_up = true;
	}
	if (IsKeyDown(KEY_A)) {
		//player_pos.x -= 1;
		input.move_left = true;
	}
	if (IsKeyDown(KEY_S)) {
		//player_pos.y += 1;
		input.move_down = true;
	}
	if (IsKeyDown(KEY_D)) {
		//player_pos.x += 1;
		input.move_right = true;
	}
	return input;
}

struct Player {
	Vec2 world_pos;
	Vec2 velocity;
	float size;
};
#include <cassert>

void update(Arena& arena, float dt, Player& player, input_state input) {

	// intended movement
	Vec2 candidate{};


	// combine candidate intended movement into vector [-1,1] so opposites naturally cancel to 0,
	if (input.move_down == true) {
		//candidate.y += player.velocity.y * dt;
		candidate.y += 1;
	}
	if (input.move_up == true) {
		//candidate.y -= player.velocity.y * dt;
		candidate.y -= 1;
	}
	if (input.move_left == true) {
		candidate.x -= 1;
	}
	if (input.move_right == true) {
		candidate.x += 1;
	}
	// normalise it so we get even movement across diagonal, 
	candidate.normalise();


	float new_x = player.world_pos.x;
	new_x += candidate.x * player.velocity.x * dt;
	float new_y = player.world_pos.y;
	new_y += candidate.y * player.velocity.y * dt;

	// calculate AABB tiles
	int left = static_cast<int>(std::floor(static_cast<float>(new_x / TILE_SIZE)));
	int right = static_cast<int>(std::floor(static_cast<float>((new_x+player.size-1) / TILE_SIZE)));
	int top = static_cast<int>(std::floor(static_cast<float>(new_y / TILE_SIZE)));
	int bottom = static_cast<int>(std::floor(static_cast<float>((new_y+player.size-1) / TILE_SIZE)));
	
	// assert(indicies in range)

	// bounds check should actually be done against world dimensions, not screen.
	if (new_y > arena.map_height || new_x> arena.map_width ||
		new_x <= 0 || new_y <= 0 || new_x + player.size > arena.map_width || new_y + player.size > arena.map_height) {
		return;
	}
	// check all


	//if (arena.grid[top][left] == 1 || grid[top][right] == 1 || grid[bottom][left] == 1 || grid[bottom][right] == 1) {
	//	return;
	//}
	if (arena.isWall(top, left) || arena.isWall(top, right) || arena.isWall(bottom, left) || arena.isWall(bottom, right)) {
		return;
	}
	//if (grid[tile_y][tile_x] == 1 || grid[tile_y1][tile_x1] == 1) {
	//	std::cout << "NOT ALLOWED, tile x :" << tile_x << " tile y: " << tile_y << "\n";
	//	return;
	//}
	player.world_pos.x = new_x;	
	player.world_pos.y = new_y;

}

//std::optional<std::vector<std::vector<int>>> parse_csv(std::string path) {
//	std::ifstream file(path);
//	if (file.is_open()) {
//		return std::nullopt;
//	}
//
//	std::vector<std::vector<int>> grid;
//
//	std::string line;
//	while(std::getline(file, line)) {
//		line += "\n";
//	}
//
//	return line;
//
//}

int main() {
	//auto str = parse_csv("arena.csv");
	//auto g = str.value();

	SetConfigFlags(FLAG_VSYNC_HINT);

	// assets/arena/arena.csv
	Arena arena("arena.csv");
	//Arena arena(44,24);
	//for (int i = 0; i < 40; i++) {
	//	arena.grid[21][i] = 1;
	//	arena.grid[0][i] = 1;
	//}
	//for (int i = 0; i < 22; i++) {
	//	arena.grid[i][39] = 1;
	//	arena.grid[i][0] = 1;
	//}
	//for (int i = 0; i < 22; i++) {
	//	if (i == 14 || i == 13) { continue; }
	//	arena.grid[i][15] = 1;
	//}
	//
	//arena.grid[21][20] = 0;
	//arena.grid[21][21] = 0;
	//arena.grid[21][22] = 0;

	//std::vector<char> arena = load_file("assets\\arena.csv");
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GameAI");

	Player player;
	player.world_pos = { 100,100 };
	player.velocity = { 350,350 };
	player.size = 30.f;

	Camera2D camera = { 0 };
	camera.zoom = 1.f;

	Vec2 rect{ 20,40 };
	while (!WindowShouldClose()) {

		auto dt = GetFrameTime();
		// handle_input()
		input_state input = handle_input();
		// update()
		update(arena,dt, player, input);
		//Draw
		//camera.target = *reinterpret_cast<Vector2*>(&player.world_pos);
		BeginDrawing();
		ClearBackground({ 32, 38, 48, 255 });
		BeginMode2D(camera);
		//DrawRectangleLines(50,50, TILE_SIZE, TILE_SIZE, BLUE);
		//draw_arena();
		draw_grid(arena);
		DrawRectangleV(*reinterpret_cast<Vector2*>(&player.world_pos), { player.size,player.size }, BLUE);
		//DrawRectangle(player.world_pos.x, player.world_pos.y, player_size, player_size, BLUE);/**/
		DrawRectangleLines(player.world_pos.x, player.world_pos.y, player.size, player.size, RAYWHITE);

		//DrawRectangleLines(width, height, player_size, player_size, RAYWHITE);
		//DrawRectangle(100, 100, 600, 400, { 100, 100, 100, 255 });
		EndMode2D();
		EndDrawing();
	}

	CloseWindow();
	return 0;	
}