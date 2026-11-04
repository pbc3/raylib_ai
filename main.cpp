#include <raylib.h>
#include <iostream>
#include <vector>
#include <algorithm>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

struct Vec2 {
	float x, y;
};


int main() {
	InitWindow(800, 600, "GameAI");

	Vec2 rect{ 20,40 };
	while (!WindowShouldClose()) {


		// handle_input()
	
		// update()
		
		//Draw
		BeginDrawing();
		DrawRectangle(rect.x,rect.y,20, 20, BLUE);
		//Render();
		ClearBackground(RAYWHITE);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}