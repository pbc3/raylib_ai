#include "fsm.h"
#include "enemy.h"

void EnemyStateMachine::update(Enemy& agent, float dt, Arena& arena, Player& player, std::vector<Bullet>& bullets)
{
    switch (state)
    {
        // PATROL -> SPOTTED? - > CHASE
        // CHASE - > IN RANGE ? -> ENGAGE
        // CHASE -> FOLLOW PATH -> ARRIVED AT LAST SEEN -> NOONE THERE -> PATROL
        // ENGAGE -> OUT OF RANGE? - > CHASE
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
            if(agent.perceptions.spotted_player){

                Vec2 direction = agent.perceptions.last_seen_player - agent.world_pos;

                direction.normalise();
                agent.forward = direction;

                int tile = arena.vec2_to_tile(agent.perceptions.last_seen_player);

                // if(tile != agent.goal_tile){
                //     agent.replan(arena,agent.perceptions.last_seen_player);
                // }
            }
            //else
           MovementState chase_result =  agent.follow_path(dt,arena);
           if(chase_result == MovementState::ARRIVED && !agent.perceptions.spotted_player){
                change_state(agent,arena,EnemyState::PATROL);
           }
            break;
        }

        case EnemyState::ENGAGE:
            // check perceptions for transitions
            if (!agent.perceptions.in_range || agent.perceptions.obstructed_los)
            {
                change_state(agent, arena, EnemyState::CHASE);
                return;
            }
            agent.engage_player(arena, dt, player, bullets);
            break;  
    }

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
		}
}

void EnemyStateMachine::change_state(Enemy& agent, Arena& arena, EnemyState next_state) {
		on_exit(agent);
		state = next_state;
		on_enter(agent, arena);
}