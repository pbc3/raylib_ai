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
                change_state(agent, arena, EnemyState::CHASE);
                return;
            }
            if(health_below_threshold(agent.health, cover_threshold)){
                change_state(agent,arena,EnemyState::TAKE_COVER);
                return;
            }
            //else - patrol. all patrol logic is contained in this func
            agent.patrol(arena,dt);
            break;

        case EnemyState::CHASE:{
            // check perceptions for transitions
        if (agent.perceptions.in_range && !agent.perceptions.obstructed_los)
            {
                change_state(agent, arena, EnemyState::ENGAGE);
                return;
            }
            if(health_below_threshold(agent.health, cover_threshold)){
                change_state(agent,arena,EnemyState::TAKE_COVER);
                return;
            }
            if(agent.perceptions.spotted_player){

                Vec2 direction = agent.perceptions.last_seen_player - agent.world_pos;

                direction.normalise();
                agent.forward = direction;

                int tile = arena.vec2_to_tile(agent.perceptions.last_seen_player);

                 if(tile != agent.goal_tile){
                     agent.replan(arena,agent.perceptions.last_seen_player);
                }
            }
            //else
           MovementState chase_result =  agent.follow_path(dt,arena);
           if((chase_result == MovementState::ARRIVED || chase_result == MovementState::BLOCKED || chase_result == MovementState::NO_PATH) 
           && !agent.perceptions.spotted_player){
                // possibly rotate fov 360 to  "observe" once its arrived
                // this currently makes it twitch backwards...
                change_state(agent,arena,EnemyState::PATROL);
           }
            break;
        }

        case EnemyState::ENGAGE:{
            // check perceptions for transitions
            if (!agent.perceptions.in_range || agent.perceptions.obstructed_los)
            {
                change_state(agent, arena, EnemyState::CHASE);
                return;
            }
            if(health_below_threshold(agent.health, cover_threshold)){
                change_state(agent,arena,EnemyState::TAKE_COVER);
                return;
            }
            agent.engage_player(arena, dt, player, bullets);
            break;  
        }
        case EnemyState::TAKE_COVER:{
          
            MovementState move_res = agent.follow_path(dt, arena);
            if(move_res == MovementState::ARRIVED){
                change_state(agent,arena,EnemyState::HEAL);
            }
            // else - not at cover yet, tick over...
            break;
        }
        case EnemyState::HEAL:{
            // lerp heal slowly back up to 60-80, transition to..? idk, check transition conditons like normal
            
            if(agent.health > 80){
                change_state(agent, arena, EnemyState::PATROL);
                return;
            }
            agent.health+=0.1;
            heal_timer += dt;
            break;
        }
    }

};


Vec2 find_cover(Arena& arena, Enemy& agent){
    // maxheap - scored_positions

    std::vector<Vec2> cover_positions; //  pre defined

    for(const Vec2 pos : cover_positions){
        if(!valid_movement(arena, pos, agent.size)){
            continue;
        }
        int cover_tile = arena.vec2_to_tile(pos);
        int agent_tile = arena.vec2_to_tile(agent.world_pos);
        auto path = astar(arena,agent_tile,cover_tile);
        if(path.empty()){
            continue;
        }
        // path length, score shorter paths better?

        // distance scoring


    }

    return scored_positions.front();
};

void EnemyStateMachine::on_enter(Enemy& agent, Arena& arena){
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
            // Vec2 cover_spot = find_cover(arena,enemy)
            // agent.replan(arena,cover_spot);
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

            case EnemyState::HEAL{
                heal_timer = 0;
            }
		}
}

void EnemyStateMachine::change_state(Enemy& agent, Arena& arena, EnemyState next_state) {
		on_exit(agent);
		state = next_state;
		on_enter(agent, arena);
}