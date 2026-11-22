#include "enemy.h"
#include "arena.h"

void Enemy::update(Arena& arena, Player& player, float dt){
    // sense
    perceptions.update(this, arena,player);
    // think & act
    fsm.update(*this,dt);
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

 void Perceptions::update(Enemy* agent, Arena& arena, Player& player){
    // just vision for now.  
    // shoot raycasts from enemy position. first will do all sides, then will pick a front facing dir
      spotted_player = false;

    // 1 - check whether player is within range
    Vec2 to_player = player.world_pos - agent->world_pos;
    // todo - use squared distance instead
    float dist = to_player.length();

    if(dist > 500.0f){
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
  };

  enum class EnemyState{
	PATROL, CHASE, ENGAGE
};

struct EnemyStateMachine {

	EnemyState state = EnemyState::CHASE;
	// virtual ~StateMachine() = default;
	std::vector<int> patrol_locations = {156,137,203};
	void update(Enemy& agent,float dt, Arena& arena){

		switch(state){
			case EnemyState::PATROL:
      // check perceptions for transitions
      if(agent.perceptions.spotted_player){
        change_state(agent,EnemyState::CHASE);
        return;
      }
      //else
        agent.follow_path(dt,arena);
			break;

			case EnemyState::CHASE:
        // check perceptions for transitions
        if(agent.perceptions.in_range){
          change_state(agent,EnemyState::ENGAGE);
          return;
        }
        if(agent.perceptions.)
       //else

			break;

			case EnemyState::ENGAGE:
      // check perceptions for transitions

      //else

			break;
		}
	}

	void change_state(Arena& arena, Enemy& agent, EnemyState next_state) {
		on_exit();
		state = next_state;
		on_enter();
	}

  void engage_player(Arena& arena, Enemy& agent, float dt, Player& player){
      auto cur_time = std::chrono::steady_clock::now();
      auto cooldown_interval = std::chrono::seconds(3);
      if(cur_time - agent.last_shot_at >= cooldown_interval){
        // shoot again after cooldown
        Vec2 direction = player.world_pos - agent.world_pos;
        direction.normalise();
        arena.spawn_bullet(agent.world_pos,direction,Team::ENEMY);
        agent.last_shot_at = cur_time;
      }}

  }
	void on_enter(Enemy& agent, Arena& arena){
		// enter new state
			switch(state){
			case EnemyState::PATROL:
			// enter
			break;

			case EnemyState::CHASE:
			// enter
      agent.replan(arena,agent.perceptions.last_seen_player);
			break;

			case EnemyState::ENGAGE:
			// enter
			break;
		}
	};
	void on_exit(Enemy& agent){
		// exit cur state
			switch(state){
			case EnemyState::PATROL:
			// exit
			break;

			case EnemyState::CHASE:
			// exit
			break;

			case EnemyState::ENGAGE:
			// exit
			break;
		}
	}
};


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