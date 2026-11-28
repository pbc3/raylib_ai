#include "game_level.h"

void remove_dead_entities(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies) {
    std::erase_if(bullets, [](Bullet& b) { return b.expired; });
    std::erase_if(enemies, [](Enemy& e) { return e.dead; });
};

void update_enemies(Arena& arena, std::vector<Bullet>& bullets, Player& player,
                    std::vector<Enemy>& enemies, float dt) {
    // enemy update
    for (Enemy& enemy : enemies) {
        enemy.update(arena, player, dt, bullets);
    }
}
Entity* hit_entity(Player& player, std::vector<Enemy>& enemies, Bullet& bullet) {
    // loop over each enemy, and then the player. see if the bullet collides
    // check enemies
    if (bullet.team == Team::PLAYER) {
        for (Enemy& enemy : enemies) {
            if (bullet_hit(bullet, enemy)) {
                bullet.expired = true;
                return &enemy;
            }
        }
    } else {
        // check whether bullet hit player
        if (bullet_hit(bullet, player)) {
            bullet.expired = true;
            return &player;
        }
    }
    return nullptr;
}

void update_bullets(Arena& arena, std::vector<Bullet>& bullets, Player& player,
                    std::vector<Enemy>& enemies, float dt) {
    for (Bullet& b : bullets) {
        Vec2 new_bulet_pos = b.world_pos + b.velocity * dt;
        // check whether bullet is out of bounds
        if (valid_movement(arena, new_bulet_pos, b.size)) {
            b.world_pos = new_bulet_pos;
            // check whether bullet hit enemy or player
            Entity* hit_target = hit_entity(player, enemies, b);

            if (hit_target) {
                // if(hit_target == &player){
                //   player.take_damage()
                // }
                hit_target->take_damage(BULLET_DAMAGE);
            }
        } else {
            b.expired = true;
        }
    }
}

void spawn_enemy_A(std::vector<Enemy>& enemies, Arena& arena, Player& player) {
    Vec2 random_spawn = arena.random_pos();
    while (random_spawn.distance(player.world_pos) < 128.f) {
        random_spawn = arena.random_pos();
    }
    Enemy enemy{};
    enemy.world_pos = random_spawn;
    enemy.velocity = {210, 210};
    enemy.size = 26.f;
    enemy.dead = false;
    enemy.health = 100;
    enemy.perceptions.engage_range = GetRandomValue(100, 220);
    enemy.perceptions.spotted_player = false;
    enemy.forward = {1.0f, 0.f};
    enemy.type = EnemyType::CHASER;
    enemy.setup_patrol_paths(arena);
    enemy.fsm.on_enter(enemy, arena, player);
    enemies.push_back(enemy);
}
void spawn_enemy_B(std::vector<Enemy>& enemies, Arena& arena, Player& player) {
    for (int i = 0; i < 3; i++) {
        Enemy enemy{};
        enemy.id = i + 2;
        enemy.guard_zone = i;
        enemy.world_pos = arena.tile_to_world(enemy.guard_zones[enemy.guard_zone]);
        enemy.velocity = {160, 160};
        enemy.size = 26.f;
        enemy.dead = false;
        enemy.perceptions.engage_range = GetRandomValue(100, 220);
        enemy.health = 100;
        enemy.perceptions.spotted_player = false;
        enemy.forward = {1.0f, 0.f};
        enemy.type = EnemyType::GUARD;
        enemy.current_flee_zone = enemy.guard_zone;
        // enemy.setup_patrol_paths(arena);
        enemy.fsm.state = EnemyState::GUARD;
        enemy.fsm.on_enter(enemy, arena, player);
        enemies.push_back(enemy);
    }
}

void GameLevel::update(Arena& arena, float dt, GameLevel& level, InputState input) {
    update_player(arena, level.player, input, dt);
    update_enemies(arena, level.bullets, level.player, level.enemies, dt);
    // spawn bullets
    if (input.firing) {
        spawn_bullet(level.bullets, level.player.world_pos, input.mouse_dir, Team::PLAYER);
    }
    if (input.interract) {
        arena.close_door(level.player.world_pos, level.player.size, level.door_tiles,
                         level.enemies);
    }
    update_bullets(arena, level.bullets, level.player, level.enemies, dt);
    remove_dead_entities(level.bullets, level.enemies);
    update_artefacts(level.artefact_tiles, level.player);
}

void GameLevel::restart_level() {
    enemies.clear();
    bullets.clear();
    for (int tile : door_tiles) {
        auto [row, col] = arena.tile_to_row_col(tile);
        arena.grid[row][col] = TerrainType::OPEN_DOOR;
    }
    door_tiles.clear();

    door_tiles.clear();

    player.world_pos = {50, 150};
    player.velocity = {310, 310};
    player.size = 24.f;
    player.health = 250.f;
    player.team = Team::PLAYER;
    player.dead = false;

    spawn_enemy_A(enemies, arena, player);
    spawn_enemy_A(enemies, arena, player);
    spawn_enemy_B(enemies, arena, player);
    enemies[0].setup_guard_zones(arena);
    enemies[0].id = 0;
    enemies[1].id = 1;

    // setup artefacts
    artefact_tiles = generate_artefacts(arena, player, enemies);
    auto cur_time = std::chrono::steady_clock::now();
    enemies[0].last_shot_at = cur_time;
};