#include "bullet.h"

void spawn_bullet(std::vector<Bullet> &bullets, Vec2 spawn_pos, Vec2 direction,
                  Team team) {
  Bullet b;
  b.world_pos = spawn_pos;
  b.velocity = direction * b.bullet_speed;
  b.expired = false;
  b.team = team;
  b.size = 5.0f;
  bullets.push_back(b);
}
bool bullet_hit(Bullet &bullet, Entity &entity) {
  Rectangle r;
  float half_size = entity.size / 2;
  r.x = entity.world_pos.x - half_size;
  r.y = entity.world_pos.y - half_size;
  r.width = half_size * 2;
  r.height = half_size * 2;
  return CheckCollisionCircleRec(
      *reinterpret_cast<Vector2 *>(&bullet.world_pos), bullet.size, r);
};
