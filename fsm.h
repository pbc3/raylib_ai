
/*
    agent starts with a state.
    each frame, we call the current states on_update() function, passing in the current controller?



*/


struct Blackboard{
    
}
struct StateMachine;
struct State {
	virtual void update(StateMachine& controller) = 0;
	virtual void on_enter(StateMachine& controller) = 0;
	virtual void on_exit(StateMachine& controller) = 0;
};
struct StateMachine {

	State* m_State = nullptr;
	virtual ~StateMachine() = default;

	virtual void update() {
		m_State->update(*this);
	}


	virtual void change_state(State* newState) {
		// call this from State::Update()
		// exit if already has a state
		if (m_State) {
			m_State->on_exit(*this);
		}
		m_State = newState;
		m_State->on_enter(*this);
	}
};