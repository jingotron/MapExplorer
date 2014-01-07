#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <vector>
#include "SDL.h"
#undef main

#include "gameworld.h"

#include "window.h"
#include "actor.h"

const int SAME_STATE = 0;
const int INTRO_STATE = 1;
const int FIELD_STATE = 2;
const int EVENT_STATE = 3;
const int MENU_STATE = 4;

//Pausing gameplay is done using a stack of states handled by a state manager.

class StateManager;

class GameState
{
public:
	GameState(StateManager* manager)
		:mStateManager(manager) 
	{
		//determines whether this state should be drawn to screen
		mShouldDraw = true;
	}
	virtual ~GameState() {};

	virtual void Pause() {};
	virtual void Resume() {};

	//Handles events
	//bool &quit is passed as the exit condition from the game loop
	virtual int HandleEvents(SDL_Event &event, bool &quit) { return 0; };

	//updates game logic
	virtual void Update() {};

	//draws objects to screen
	virtual void Draw() {};

	bool shouldDraw() { return mShouldDraw; }
	StateManager* getManager() { return mStateManager; }

	void setDraw(bool draw) { mShouldDraw = draw; }

private:
	//pointer to the state manager
	StateManager* mStateManager;

	//boolean indicating whether state should be drawn to screen
	bool mShouldDraw;
};


//state for player walking around on map
class FieldState: public GameState
{
public:
	FieldState(StateManager* manager)
		:GameState(manager)
	{
		;
	}
	virtual ~FieldState() {};

	virtual void Pause();
	virtual void Resume(SDL_Event &event);

	virtual int HandleEvents(SDL_Event &event, bool &quit);
	virtual void Update();
	virtual void Draw();
};

//state for paused gameplay
class PauseState: public GameState
{
public:
	PauseState(StateManager* manager, SDL_Texture* pause)
		:GameState(manager), mPause(pause)
	{
	}

	virtual ~PauseState() {};

	virtual void Pause() {};
	virtual void Resume() {};

	virtual int HandleEvents(SDL_Event &event, bool &quit);
	virtual void Update();
	virtual void Draw();

private:
	SDL_Texture* mPause;
};


class StateManager
{
public:
	StateManager(GameWorld* world)
		:mWorld(world) {}
	~StateManager() {};

	int HandleEvents(SDL_Event &event, bool &quit) { return mStates.back()->HandleEvents(event, quit); }
	void Update() { mStates.back()->Update(); }
	void Draw() 
	{ 
		//Statemanager draws all layers that should be drawn
		for(int i = 0; i < mStates.size(); i++)
		{
			if(mStates[i]->shouldDraw())
				mStates[i]->Draw();
		}
	}

	void changeState(GameState* state, bool myDrawStatus = true)
	{
		mStates.push_back(state);
		state->setDraw(myDrawStatus);
	}

	std::vector<GameState*>* getStates() { return &mStates; }
	GameWorld* getWorld() { return mWorld; }

private:
	std::vector<GameState*> mStates;
	GameWorld* mWorld;
};

#endif