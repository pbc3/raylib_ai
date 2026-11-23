#include "fsm.h"
#include "enemy.h"

void EnemyStateMachine::update(Enemy& agent, float dt, Arena& arena, Player& player, std::vector<Bullet>& bullets)
{
    switch (state)
    {
        case EnemyState::PATROL:
            // check perceptions for transitions
            if (agent.perceptions.spotted_player)
            {
                change_state(agent, arena, EnemyState::CHASE);
                return;
            }

            //else
            // agent.forward = {-agent.forward.y, agent.forward.x};
            // agent.patrol(dt,arena);
            // agent.follow_path(dt,arena);
            agent.patrol(arena,dt);
            break;

        case EnemyState::CHASE:
            // check perceptions for transitions
            if (agent.perceptions.in_range)
            {
                change_state(agent, arena, EnemyState::ENGAGE);
                return;
            }

            if(!agent.perceptions.spotted_player && agent.goal_tile!=-1 && 
                agent.goal_tile == arena.vec2_to_tile(agent.world_pos)){
                change_state(agent,arena,EnemyState::PATROL);
                return;
            }
            //else
            agent.follow_path(dt,arena);
            break;

        case EnemyState::ENGAGE:
            // check perceptions for transitions
            if (!agent.perceptions.in_range)
            {
                change_state(agent, arena, EnemyState::CHASE);
                return;
            }

            //else
            agent.engage_pl ayer(arena, dt, player, bullets);
            break;
    }
};


void EnemyStateMachine::on_enter(Enemy& agent, Arena& arena){
		// enter new state
			switch(state){
			case EnemyState::PATROL:
			// enter
            agent.velocity = {100,100};
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