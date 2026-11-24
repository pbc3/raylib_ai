#include "enemy.h"
#include "arena.h"

void Enemy::update(Arena& arena, Player& player, float dt, std::vector<Bullet>& bullets){
    // sense
    perceptions.update(this, arena,player);
    // think & act
    fsm.update(*this,dt,arena,player,bullets);
}
void Enemy::replan(Arena& arena, Vec2 goal_pos){
      current_goal_pos = goal_pos;
 
      int tile = arena.vec2_to_tile(current_goal_pos);
      goal_tile = tile;
      auto new_path = astar(arena, arena.vec2_to_tile(world_pos),tile);

      cur_path.clear(); 
      cur_waypoint = 0;

      if(new_path.empty()){
        return;
      }

      for(int p : new_path){
        cur_path.push_back(arena.tile_to_world(p));
      }
}



MovementState Enemy::follow_path(float dt, Arena& arena){
    if(cur_path.empty()){
      return  MovementState::NO_PATH;
    }

    if(cur_waypoint >= cur_path.size()){
      return MovementState::ARRIVED;
    }


    Vec2 target_pos = cur_path[cur_waypoint];
    float dist = world_pos.distance(target_pos);

    // reached target
    if(dist <= threshold){
      cur_waypoint++;
      // check if we have reached the waypoint, so we can handle it this tick rather than next
      if(cur_waypoint >= cur_path.size()){
        return MovementState::ARRIVED;
      }
      return MovementState::MOVING;
    }
    Vec2 dir = target_pos - world_pos;
    dir.normalise();
    // forward = dir; // change forward direction to look in the direciton of the next movement, so vision cone sees "in front"
    move_direction = dir;
    // stop overshooting the target, clamp.
    float movement_step = std::min(velocity.x * dt,dist);
    Vec2 new_mov =  world_pos + dir *movement_step;
    if(!valid_movement(arena,new_mov,size)){
          return MovementState::BLOCKED;
    }

     world_pos = new_mov;
     return MovementState::MOVING;
  }

  float& Enemy::take_damage(float dmg){
    health -= dmg;
    if(health <= 0){
        dead = true;
    }
   return health;
  }

void Enemy::setup_patrol_paths(Arena& arena){
    float max_x = arena.cols * TILE_SIZE;
    float max_y = arena.rows * TILE_SIZE;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist_x(0.f,max_x);
    std::uniform_real_distribution<float> dist_y(0.f,max_y);

  
    while(patrol_targets.size() <= 15){
      Vec2 random_target = {dist_x(gen), dist_y(gen)};
      int tile = arena.vec2_to_tile(random_target);
      if(arena.isWall(tile)){
        continue;
      }
      patrol_targets.push_back(arena.tile_to_world(tile));
      // int enemy_tile = arena.vec2_to_tile(world_pos);
      // // else - create path to it
      // auto path = astar(arena,enemy_tile,tile);
      // auto world = tile_path_to_vec2(arena,path);
    }

}
void Enemy::patrol(Arena& arena, float dt){
    // random floats within 0,0 - tile_size * no_cols, tile_size * no_rows
    // check if you can get a path (path ! empty)
    // add, until i have 5 paths

    if(patrol_targets.empty()){
      // should not happen
      throw std::runtime_error("patrol paths are empty");
    }

    if(current_patrol_idx >= patrol_targets.size()){
      // generate new paths
      setup_patrol_paths(arena);
      // OR just wrap around:
      // current_patrol_idx = (current_patrol_idx + 1) % patrol_targets.size();
      current_patrol_idx = 0;
    }

    MovementState movement_res = follow_path(dt,arena);
    if(movement_res == MovementState::ARRIVED || movement_res == MovementState::BLOCKED || movement_res == MovementState::NO_PATH){
     current_patrol_idx = (current_patrol_idx + 1) % patrol_targets.size();
    replan(arena, patrol_targets[current_patrol_idx]);
    }
    forward = move_direction;
}


 void Perceptions::update(Enemy* agent, Arena& arena, Player& player){
    // just vision for now.  
    // shoot raycasts from enemy position. first will do all sides, then will pick a front facing dir
    spotted_player = false;
    in_range = false;
    obstructed_los = true;


       

    // 1 - check whether player is within range
    Vec2 to_player = player.world_pos - agent->world_pos;
    // todo - use squared distance instead
    float dist = to_player.length();
   
    // in range?
   const float ENGAGE_RANGE = 150.f;
   if(dist < ENGAGE_RANGE){
    in_range = true;
   }
   

    if(dist > max_distance){
      return; // dont think i should return
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
    float step = 8.0f;
    for(float i = 0; i < dist; i+=step){
      at = ray.at(i);
      if(arena.isWall(arena.vec2_to_tile(at))){
        return;
      }
    }
    obstructed_los = false;
   spotted_player = true;
   last_seen_player = player.world_pos;

  
  };



void Enemy::engage_player(Arena& arena, float dt, Player& player, std::vector<Bullet>& bullets){
      auto cur_time = std::chrono::steady_clock::now();
      auto cooldown_interval = std::chrono::milliseconds(500);
        Vec2 direction = player.world_pos -world_pos;
        direction.normalise();
        forward = direction; // this is for debug, show the aim line / forward line
      if(cur_time - last_shot_at >= cooldown_interval){
        // shoot again after cooldown
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