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
std::vector<std::vector<int>> grid(22, std::vector<int>(40, 0));
void draw_grid() {
	for (int i = 0; i < ARENA_HEIGHT; i++) {
		for (int j = 0; j < ARENA_WIDTH; j++) {
			// x = column * 50
			// y = row * 50
			int x = j * TILE_SIZE;
			int y = i * TILE_SIZE;

			if (grid[i][j] == 1) {
				DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, { 100, 125, 150, 255 });
			}
		}
	}

	// <= because we want to draw edges
	for (int i = 0; i <= ARENA_HEIGHT; i++) {
		DrawLine(0, i * TILE_SIZE,SCREEN_WIDTH, i * TILE_SIZE,GRID_COLOUR);
	}

	for (int j = 0; j <= ARENA_WIDTH; j++) {
		DrawLine(j * TILE_SIZE, 0,j * TILE_SIZE, SCREEN_HEIGHT,GRID_COLOUR);
	}

	DrawRectangleLines(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 141, 169, 196, 255 });
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

void update(float dt, Player& player, input_state input) {

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


	std::cout << candidate.x << " " << candidate.y << "\n";
	float new_x = player.world_pos.x;
	new_x += candidate.x * player.velocity.x * dt;
	float new_y = player.world_pos.y;
	new_y += candidate.y * player.velocity.y * dt;
	int tile_x = static_cast<int>(std::floor(static_cast<float>(new_x / TILE_SIZE)));
	int tile_y = static_cast<int>(std::floor(static_cast<float>(new_y / TILE_SIZE)));

	Rectangle box{
		candidate.x, candidate.x + player.size,
		candidate.y, candidate.y + player.size
	};
	
	if (new_y > SCREEN_HEIGHT-1 || new_x> SCREEN_WIDTH-1 || new_x < 0 || new_y < 0) {
		return;
	}
		
	if (grid[tile_y][tile_x] == 1) {
		std::cout << "NOT ALLOWED, tile x :" << tile_x << " tile y: " << tile_y << "\n";
		return;
	}
	//std::cout << new_x << " " << new_y << "\n";
	player.world_pos.x = new_x;
	player.world_pos.y = new_y;

	//std::cout << "new x : " << new_x << " new y : " << new_y << "\n";

	// calculate what new position would be

	// see if allowed

	// allow or deny

	// player AABB

}


int main() {
	SetConfigFlags(FLAG_VSYNC_HINT);

	for (int i = 0; i < 40; i++) {
		grid[21][i] = 1;
		grid[0][i] = 1;
	}
	for (int i = 0; i < 22; i++) {
		grid[i][39] = 1;
		grid[i][0] = 1;
	}
	for (int i = 0; i < 22; i++) {
		if (i == 14 || i == 13) { continue; }
		grid[i][15] = 1;
	}
	
	grid[21][20] = 0;
	grid[21][21] = 0;
	grid[21][22] = 0;

	//std::vector<char> arena = load_file("assets\\arena.csv");
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GameAI");

	float player_size = 32;
	Player player;
	player.world_pos = { 100,100 };
	player.velocity = { 350,350 };
	player.size = 32.f;

	Camera2D camera = { 0 };
	camera.zoom = 1.f;

	Vec2 rect{ 20,40 };
	while (!WindowShouldClose()) {

		auto dt = GetFrameTime();
		// handle_input()
		input_state input = handle_input();
		// update()
		update(dt, player, input);
		//Draw

		BeginDrawing();
		ClearBackground({ 32, 38, 48, 255 });
		BeginMode2D(camera);
		//DrawRectangleLines(50,50, TILE_SIZE, TILE_SIZE, BLUE);
		//draw_arena();
		draw_grid();
		DrawRectangleV(*reinterpret_cast<Vector2*>(&player.world_pos), { player_size,player_size }, BLUE);
		//DrawRectangle(player.world_pos.x, player.world_pos.y, player_size, player_size, BLUE);/**/
		DrawRectangleLines(player.world_pos.x, player.world_pos.y, player_size, player_size, RAYWHITE);
		//int l=;
		//int r;
		//int t;
		//int b;
		//int width = r - l;
		////int height = t - b;
		//DrawRectangleLines(width, height, player_size, player_size, RAYWHITE);
		//DrawRectangle(100, 100, 600, 400, { 100, 100, 100, 255 });
		EndMode2D();
		EndDrawing();
	}

	CloseWindow();
	return 0;	
}