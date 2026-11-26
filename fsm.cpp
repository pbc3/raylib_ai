#include "fsm.h"
#include "arena.h"
#include "enemy.h"
#include "math_utils.h"
#include <vector>


void EnemyStateMachine::update(Enemy& agent, float dt, Arena& arena, Player& player, std::vector<Bullet>& bullets)
{
    auto health_below_threshold = [&](float agent_health, float threshold) -> bool{
        return agent_health <= threshold;
    };

    switch (state)
    {
        // PATROL -> SPOTTED? - > CHASE
        // CHASE - > IN RANGE ? -> ENGAGE
        // CHASE -> FOLLOW PATH -> ARRIVED AT LAST SEEN -> NOONE THERE -> PATROL
        // ENGAGE -> OUT OF RANGE? - > CHASE
        // ALL STATES ABOVE - > HEALTH < THRESHOLD ? -> TAKE COVER
        // TAKE->COVER - AT TARGET? -> HEAL STATE
        // HEAL -> HEALED TO 80? -> CHECK TRANSITIONS AND SWITCH. possibly check for danger close transtions before healing?
        case EnemyState::PATROL:
            // check perceptions for transitions
            if (agent.perceptions.spotted_player)
            {
                // if(agent.perceptions.in_range){
                //     change_state(agent, arena, EnemyState::ENGAGE);
                // }
                change_state(agent, arena, EnemyState::CHASE, player);
                return;
            }
            if(health_below_threshold(agent.health, cover_threshold)){
                change_state(agent,arena,EnemyState::TAKE_COVER, player);
                return;
            }
            //else - patrol. all patrol logic is contained in this func
            agent.patrol(arena,dt);
            break;

        case EnemyState::CHASE:{
            // check perceptions for transitions
        if(health_below_threshold(agent.health, cover_threshold)){
                change_state(agent,arena,EnemyState::TAKE_COVER, player);
                return;
            }
        if (agent.perceptions.in_range && !agent.perceptions.obstructed_los)
            {
                change_state(agent, arena, EnemyState::ENGAGE,player);
                return;
            }

            //else
        if(agent.perceptions.spotted_player){
                Vec2 direction = agent.perceptions.last_seen_player - agent.world_pos;
                direction.normalise();
                agent.forward = direction;

                int tile = arena.vec2_to_tile(agent.perceptions.last_seen_player);

                 if(tile != agent.goal_tile && replan_timer >= 1.0){
                    agent.replan(arena,agent.perceptions.last_seen_player);
                    replan_timer = 0.f;
                }
                else{
                    replan_timer+=dt;
                }
            }
           MovementState chase_result =  agent.follow_path(dt,arena);
           if((chase_result == MovementState::ARRIVED || chase_result == MovementState::BLOCKED || chase_result == MovementState::NO_PATH) 
           && !agent.perceptions.spotted_player){
                // possibly rotate fov 360 to  "observe" once its arrived
                // this currently makes it twitch backwards...
                change_state(agent,arena,EnemyState::PATROL, player);
           }
            break;
        }

        case EnemyState::ENGAGE:{
            // check perceptions for transitions
            if (!agent.perceptions.in_range || agent.perceptions.obstructed_los)
            {
                change_state(agent, arena, EnemyState::CHASE, player);
                return;
            }
            if(health_below_threshold(agent.health, cover_threshold)){
                change_state(agent,arena,EnemyState::TAKE_COVER,  player);
                return;
            }
                agent.engage_player(arena, dt, player, bullets);
            break;  
        }
        case EnemyState::TAKE_COVER:{
          
            MovementState move_res = agent.follow_path(dt, arena);
            if(move_res == MovementState::ARRIVED){
                // if(!still_providing_cover(cover_position)){
                //     find_cover(arena,agent,player); // try again
                // }
                change_state(agent,arena,EnemyState::HEAL, player);
            }
            // else - not at cover yet, tick over...
            break;
        }
        case EnemyState::HEAL:{
            // lerp heal slowly back up to 60-80, transition to..? idk, check transition conditons like normal
            
            if(agent.health > 80){
                change_state(agent, arena, EnemyState::PATROL,player);
                return;
            }
            agent.health += agent.health * dt;
            heal_timer += dt;
            break;
        }
    }

};


struct Compare{
    bool operator()(const std::pair<float,Vec2>& a,const std::pair<float,Vec2>& b) const {
        return a.first < b.first;
    }
};

Vec2 EnemyStateMachine::find_cover(Arena& arena, Enemy& agent, Player& player){
    // maxheap - scored_positions
    std::priority_queue<std::pair<float,Vec2>, std::vector<std::pair<float, Vec2>>, Compare> scored_positions;

    for(int tile : cover_positions){
        // discard walls, a* does this already?
        Vec2 pos = arena.tile_to_world(tile);
        if(!valid_movement(arena, pos, agent.size)){
            continue;
        }

        int cover_tile = arena.vec2_to_tile(pos);
        int agent_tile = arena.vec2_to_tile(agent.world_pos);
        auto path = astar(arena,agent_tile,cover_tile);

        if(path.empty()){
            continue;
        }
        
        // raycast, does it block the line of sight?
        float step = 1.0f;
        Ray_ ray;
        Vec2 to_player = player.world_pos - pos;
        float dist = to_player.length();
        ray.origin = pos;
        to_player.normalise();
        ray.direction = to_player;
        bool los_blocked = false;
        float score = 0.f;

        for(float i = 0; i < dist; i+=step){
            Vec2 at = ray.at(i);
            if(arena.isWall(arena.vec2_to_tile(at))){
                los_blocked = true;
            }
        }
        if(!los_blocked){
            continue;
        }
      
        for(int waypoint : path){
            bool exposed = true;
            Ray_ ray_from_waypoint;
            ray_from_waypoint.origin = arena.tile_to_world(waypoint);
            Vec2 waypoint_to_player = player.world_pos - ray_from_waypoint.origin;
            float distance = waypoint_to_player.length();
            waypoint_to_player.normalise();
            ray_from_waypoint.direction = waypoint_to_player;
            Vec2 at{0,0};
            for(float i = 0; i < distance; i+=step){
                at = ray_from_waypoint.at(i);
                if(arena.isWall(arena.vec2_to_tile(at))){
                    exposed = false;
                    break;
                }
         }   
         if(exposed){
            score-=1;
         }
        }

        // inverse for scoring. shorter paths = higher score
        score += (1.0 / path.size());
        // draw circle around tile, if player is within that tile, score lower, if not, score higher
        scored_positions.push({score,pos});
    }
    return scored_positions.empty() ? arena.tile_to_world(cover_positions[0]) : scored_positions.top().second;
};



void EnemyStateMachine::on_enter(Enemy& agent, Arena& arena, Player& player){
		// enter new state
			switch(state){
			case EnemyState::PATROL:
			// enter
            agent.velocity = {150,150};
            agent.replan(arena,agent.patrol_targets[agent.current_patrol_idx]);
			break;
			case EnemyState::CHASE:
			// enter
            agent.velocity = {350,350};
            agent.replan(arena,agent.perceptions.last_seen_player);
			break;
			case EnemyState::ENGAGE:
			// enter
			break;
            case EnemyState::TAKE_COVER:{
            agent.velocity = {400,400}; // speeed boost to run to cover
            Vec2 cover_spot = find_cover(arena,agent, player);
            agent.replan(arena,cover_spot);
            }
		}
};



void EnemyStateMachine::on_exit(Enemy& agent){
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
            
            case EnemyState::TAKE_COVER:{
                break;
            }
            case EnemyState::HEAL:{
                heal_timer = 0;
                break;
            }
		}
}

void EnemyStateMachine::change_state(Enemy& agent, Arena& arena, EnemyState next_state, Player& player) {
		on_exit(agent);
		state = next_state;
		on_enter(agent, arena, player);
}


/*
    artefacts!!

*/

