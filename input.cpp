#include "input.h"
#include "player.h"

InputState handle_input(Camera2D camera, Player& player) {
    InputState input{};
    input.move_up = IsKeyDown(KEY_W);
    input.move_down = IsKeyDown(KEY_S);
    input.move_left = IsKeyDown(KEY_A);
    input.move_right = IsKeyDown(KEY_D);
    input.firing = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    input.interract = IsKeyPressed(KEY_E);
    // aim direction
    Vector2 screen_mouse_pos = GetMousePosition();
    Vector2 world_mouse_pos = GetScreenToWorld2D(screen_mouse_pos, camera);
    input.mouse_dir = (*reinterpret_cast<Vec2*>(&world_mouse_pos) - player.world_pos).normalised();
    return input;
}