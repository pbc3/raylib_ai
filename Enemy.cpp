#include "enemy.h"
#include "arena.h"

void Enemy::update(Arena& arena, Player& player, float dt, std::vector<Bullet>& bullets){
    // sense
    perceptions.update(this, arena,player);
    // think & act
    fsm.update(*this,dt,arena,player,bullets);
}
void Enemy::replan(Arena& arena, Vec2 goal_pos){
    if(goal_tile != arena.vec2_to_tile(goal_pos)){
        auto new_path = astar(arena, arena.vec2_to_tile(world_pos),
        arena.vec2_to_tile(goal_pos));
        std::vector<Vec2> wp;
        if(!new_path.empty()){
          for(int p : new_path){
                wp.push_back(arena.tile_to_world(p)); 
           }
          cur_path = wp;
          goal_tile = arena.vec2_to_tile(goal_pos);
          cur_waypoint=0;
          return;
        }
        cur_waypoint = 0;
        cur_path = {};
      }
}
void Enemy::follow_path(float dt, Arena& arena){
    if(cur_waypoint >= cur_path.size()){
    //   return world_pos;
    //  replan(arena,goal_pos);
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
      // replan(arena,goal_pos);
      return;
    //   return world_pos;
    }
    Vec2 dir = target_pos - world_pos;
    dir.normalise();
    forward = dir;

    float movement_step = velocity.x * dt;
    // Vec2 intended = dir * velocity.x * dt;
    // stop overshooting the target, clamp.
    if(movement_step >= dist && valid_movement(arena, target_pos, size)){
      world_pos = target_pos;
      cur_waypoint++;
      // replan(arena,goal_pos);
      return;
    }
    
   Vec2 new_mov =  world_pos + dir *movement_step;
   if(!valid_movement(arena,new_mov,size)){
        replan(arena,perceptions.last_seen_player);
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

  void Enemy::patrol(Arena& arena, float dt){
    Vec2 cur_goal = patrol_targets[current_patrol_idx];

    if(cur_path.empty()){
      replan(arena,cur_goal);
    }
    if(cur_goal.distance(world_pos) <= 10.f){
        // reached current patrol goal
        current_patrol_idx++;
        if(current_patrol_idx >= patrol_targets.size()){
            current_patrol_idx = 0;
        }
        cur_goal = patrol_targets[current_patrol_idx];
        replan(arena,cur_goal);
    }
    // move toward cur goal... or do nothing, because we are already moving?
    follow_path(dt,arena);
}


 void Perceptions::update(Enemy* agent, Arena& arena, Player& player){
    // just vision for now.  
    // shoot raycasts from enemy position. first will do all sides, then will pick a front facing dir
    spotted_player = false;
    in_range = false;

    // 1 - check whether player is within range
    Vec2 to_player = player.world_pos - agent->world_pos;
    // todo - use squared distance instead
    float dist = to_player.length();

    if(dist > max_distance){
      return;
    }
    if(dist < 0.0001){
      spotted_player = true;
      last_seen_player = player.world_pos;
      return;
    }
    
    // 2 - check whether player is inside vision cone
    // cone = forward dir dot to_player
    to_player.normalise();
    float dot = agent->forward.dot(to_player);
    // todo - do this offline
    float half_angle = std::cosf(view_angle_rads / 2);

    if(dot < half_angle){
      //not inside vision cone, return
      return; 
    }
    // 3 - check whether agent can see player directly, no walls in between
    Ray_ ray;
    ray.origin = agent->world_pos;
    ray.direction = to_player;
    
    // raymarch
    Vec2 at = {};
    float step = 2.0f;
    for(float i = 0; i < dist; i+=step){
      at = ray.at(i);
      if(arena.isWall(arena.vec2_to_tile(at))){
        return;
      }
    }
   spotted_player = true;
   last_seen_player = player.world_pos;
   
   // in range?
   const float ENGAGE_RANGE = 350.f;
   if(dist < ENGAGE_RANGE){
    in_range = true;
   }
  
  };



void Enemy::engage_player(Arena& arena, float dt, Player& player, std::vector<Bullet>& bullets){
      auto cur_time = std::chrono::steady_clock::now();
      auto cooldown_interval = std::chrono::milliseconds(500);
      if(cur_time - last_shot_at >= cooldown_interval){
        // shoot again after cooldown
        Vec2 direction = player.world_pos -world_pos;
        direction.normalise();
        forward = direction; // this is for debug, show the aim line / forward line
        spawn_bullet(bullets,world_pos,direction,Team::ENEMY);
        last_shot_at = cur_time;
      }
    }


  /*
  Enemy A State Machine
  States:
  patrol
  chase last known pos
  engage if in range
  
  transitions:
  patrol -> chase if spotted
  chase -> engage if spotted
  chase -> new chase position if updated pos, so check if u can see the enemy in chase
  engage -> patrol if enemy dead


  actions inside each state:
  patrol - rotate forward direction,so it "surveys" the area. also draw the view cone. rotate it slowly, rather than per tick.
  pick a new path inside the rotated forward dir and move to it. each tick check if we reached that path. if so, calc new path, else, move forward.
  chase - move to last seen pos
  engage - while can_see -> shoot at the players direction
  */

// struct EnemyAStateMachine : public StateMachine{
//   Enemy* agent;
// };

// struct PatrolState : public State{
//   virtual void update(StateMachine& controller) override{
//     // check if spotted

//     if()

//     // if no transition, continue patrol
//   }
//   virtual void on_enter(StateMachine& controller) override{

//     // find a position to patrol to, start moving
//   }
// };