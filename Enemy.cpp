#include "enemy.h"
#include "arena.h"

void Enemy::replan(Arena& arena, Vec2 player_pos){
    if(player_tile != arena.vec2_to_tile(player_pos)){
        auto new_path = astar(arena, arena.vec2_to_tile(world_pos),arena.vec2_to_tile(player_pos));
        std::vector<Vec2> wp;
        if(!new_path.empty()){
        for(int p : new_path){
              wp.push_back(arena.tile_to_world(p)); 
        }
        cur_path = wp;
        player_tile = arena.vec2_to_tile(player_pos);
        cur_waypoint=0;
        return;
        }
        cur_path = {};
      }
}
void Enemy::follow_path(float dt, Arena& arena, Vec2 player_pos){

    if(cur_waypoint >= cur_path.size()){
    //   return world_pos;
     replan(arena,player_pos);
     return; 
    }
    if(cur_path.empty()){
        return;
    }
    Vec2 target_pos = cur_path[cur_waypoint];
    float dist = world_pos.distance(target_pos);
    // reached target
    if(dist < threshold){
      cur_waypoint++;
      replan(arena,player_pos);
      return;
    //   return world_pos;
    }
    Vec2 dir = target_pos - world_pos;
    dir.normalise();

    float movement_step = velocity.x * dt;
    // Vec2 intended = dir * velocity.x * dt;
    // stop overshooting the target, clamp.
    if(movement_step >= dist && valid_movement(arena, target_pos, size)){
      world_pos = target_pos;
      cur_waypoint++;
      replan(arena,player_pos);
      return;
    }
    
   Vec2 new_mov =  world_pos + dir *movement_step;
   if(!valid_movement(arena,new_mov,size)){
        replan(arena,player_pos);
        return;
   }
    world_pos = new_mov;

    // world_pos += dir * movement_step;
  }

  float& Enemy::take_damage(float dmg){
    health -= dmg;
    if(health <= 0){
        dead = true;
    }
   return health;
  }