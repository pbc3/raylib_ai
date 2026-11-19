#include "enemy.h"


void Enemy::follow_path(float dt){
    if(cur_waypoint >= cur_path.size()){
      return;
    }

    Vec2 target_pos = cur_path[cur_waypoint];
    float dist = world_pos.distance(target_pos);
    // reached target
    if(dist < threshold){
      cur_waypoint++;
      return;
    }
    Vec2 dir = target_pos - world_pos;
    dir.normalise();


    float movement_step = velocity.x * dt;
    // Vec2 intended = dir * velocity.x * dt;
    // stop overshooting the target, clamp..
    if(movement_step >= dist){
      world_pos = target_pos;
      cur_waypoint++;
      return;
    }
    
    world_pos += dir * movement_step;
    // world_pos.x += dir.x * velocity.x* dt;
    // world_pos.y += dir.y * velocity.y* dt;
  }