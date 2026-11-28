#include "player.h"
#include "arena.h"

float& Player::take_damage(float base_dmg) {
    // probablistic damage
    // 2d6
    int damage_modifer = GetRandomValue(1, 6) + GetRandomValue(1, 6) - 7;
    base_dmg = std::max(0.f, base_dmg + damage_modifer);

    // 10 percent chance of a critical hit
    int crit = GetRandomValue(1, 100);
    if (crit <= 10) {
        crit_timer = 1.0f;
        crit_hit = true;
        base_dmg += GetRandomValue(1, 8);
        last_dmg = base_dmg;
    } else {
        crit_hit = false;
    }

    health = std::max(0.f, health - base_dmg);

    if (health <= 0) {
        dead = true;
    }

    return health;
};

void update_player(Arena& arena, Player& player, InputState input, float dt) {
    Vec2 candidate{};
    if (input.move_down) {
        // candidate.y += player.velocity.y * dt;
        candidate.y += 1;
    }
    if (input.move_up) {
        // candidate.y -= player.velocity.y * dt;
        candidate.y -= 1;
    }
    if (input.move_left) {
        candidate.x -= 1;
    }
    if (input.move_right) {
        candidate.x += 1;
    }
    // normalise it so we get even movement across diagonal,
    candidate.normalise();
    int tile = arena.vec2_to_tile(player.world_pos);
    float speed_modifier = arena.terrain_speed_modifier(tile);

    // new horizontal movem ent
    float new_x = player.world_pos.x;
    new_x += candidate.x * player.velocity.x * speed_modifier * dt;
    Vec2 requested_movement = {new_x, player.world_pos.y};
    if (valid_movement(arena, requested_movement, player.size)) {
        player.world_pos.x = new_x;
    }
    // new vertical movement
    float new_y = player.world_pos.y;
    new_y += candidate.y * player.velocity.y * speed_modifier * dt;
    requested_movement.x = player.world_pos.x;
    requested_movement.y = new_y;
    if (valid_movement(arena, requested_movement, player.size)) {
        player.world_pos.y = new_y;
    }
}