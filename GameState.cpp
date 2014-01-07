#include "GameState.h"

void FieldState::Pause()
{
	//Sets player velocities to zero
	getManager()->getWorld()->getPlayer()->setVelx(0);
	getManager()->getWorld()->getPlayer()->setVely(0);
}

void FieldState::Resume(SDL_Event &event)
{
	//Resumes getting input
	getManager()->getWorld()->getPlayer()->handleInput();
}

int FieldState::HandleEvents(SDL_Event &event, bool &quit)
{
	//set next state to be the same state
	int nextState = SAME_STATE;

	if (event.type == SDL_QUIT)
		quit = true;
	if (event.type == SDL_KEYDOWN)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_ESCAPE:     
			//press escape to quit
			quit = true;
			break;
		case SDLK_c:
			//press c to toggle collision boxes
			getManager()->getWorld()->toggleColBox();
			break;
		case SDLK_0:
			//press 0 to set to fullscreen
			SDL_SetWindowFullscreen(getManager()->getWorld()->getWin()->getWindow(), SDL_WINDOW_FULLSCREEN);
			break;
		case SDLK_SPACE:
			//press space to change to pause state
			nextState = MENU_STATE;
			break;
		default:
			break;
		}
	}

	//handle input
	getManager()->getWorld()->getPlayer()->handleInput();
	return nextState;
}




void FieldState::Update()
{
	//Logic
	if (getManager()->getWorld()->shouldLoadNextLevel())
		getManager()->getWorld()->openMap(getManager()->getWorld()->getNextLevel());
	//When the mLoadNextLevel trigger is activated, the caller should also change the mNextLevel and mNextPlayerSpawn variables.

	getManager()->getWorld()->moveActors();
}

void FieldState::Draw()
{
	GameWorld* World = getManager()->getWorld();

	//Render
	World->parallaxBg();
	World->drawBackground(World->getLevel()->getTileMap());
	World->drawBackground(World->getLevel()->getTileMap2());
	World->drawActors();
	World->drawBackground(World->getLevel()->getOverMap());
	World->drawBackground(World->getLevel()->getOverMap2());
}

int PauseState::HandleEvents(SDL_Event &event, bool &quit)
{
	int nextState = SAME_STATE;

	if (event.type == SDL_QUIT)
		quit = true;
	if (event.type == SDL_KEYDOWN)
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_SPACE:
			nextState = FIELD_STATE;
			break;
		case SDLK_ESCAPE:
			quit = true;
			break;
		}
	}
	return nextState;
}

void PauseState::Update()
{
}

void PauseState::Draw()
{
	getManager()->getWorld()->getWin()->Draw(mPause);
}