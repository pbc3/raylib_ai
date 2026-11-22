#include "bullet.h"

void spawn_bullet(std::vector<Bullet>& bullets, Vec2 spawn_pos, Vec2 direction, Team team){
  Bullet b;
  b.world_pos = spawn_pos;
  b.velocity = direction * b.bullet_speed;
  b.expired = false;
  b.team = team;
  b.size = 5.0f;
  bullets.push_back(b);
}