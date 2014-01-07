#include "SDL.h"
#undef main
#include "SDL_mixer.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

#include "timer.h"
#include "window.h"
#include "actor.h"	
#include "gameworld.h"
#include "GameState.h"

#include <iostream>
#include <string>
#include <sstream>

const std::string MUSIC = "music.mp3";
const std::string MAP = "MyMap.tmx";
const std::string PAUSE = "pause.png";
const std::string CHARACTER = "character.png";
const std::string BACKGROUND = "background.jpg";

//max framerate
const int FRAMES_PER_SECOND = 60;

//tile dimensions for the character
const int CHAR_TILE_WIDTH = 26;
const int CHAR_TILE_HEIGHT = 53;
const int CHAR_ALPHA = 0;

int SDL_main(int argc, char* argv[])
{
	//initialize window
	Window Win;
	try {
		Win.Init("The Game");
	}
	catch (const std::runtime_error &e){
		std::cout << e.what() << std::endl;
		Win.Quit();
		return -1;
	}
	//Do not do anything to window until it has been initialized!

	//Set non-border window
	SDL_SetWindowBordered(Win.getWindow(), SDL_FALSE);

	//Open audio
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		return false;
	Mix_Music* music = Mix_LoadMUS(MUSIC.c_str());
	Mix_PlayMusic(music, -1);


	//load player image
	SDL_Texture* bg = Win.LoadImage(BACKGROUND);
	SDL_Texture* pause = Win.LoadImage(PAUSE);

	//create world
	GameWorld World(&Win);

	//open character files
	World.openCharTiles(CHARACTER, CHAR_TILE_WIDTH, CHAR_TILE_HEIGHT, CHAR_ALPHA);

	//open map (must be done after initializing window)
	try {
		World.openMap(MAP);
	}
	catch (const std::runtime_error &e){
		std::cout << e.what() << std::endl;
		Win.Quit();
		return -1;
	}

	Player* player = World.getPlayer();

	//set timers for fps counter
	Timer fps;
	Timer fpsUpdate;
	int frame = 0;
	bool frameCap = true;

	fpsUpdate.Start();

	//Create game states
	StateManager manager(&World);
	FieldState field(&manager);
	PauseState menu(&manager, pause);
	manager.getStates()->push_back(&field);

	//Main Loop
	bool quit = false;
	int loadState = SAME_STATE;
	SDL_Event event;
	while(!quit)
	{
		fps.Start();
		while(SDL_PollEvent(&event))
		{
			//if the state was changed, load the next state
			loadState = manager.HandleEvents(event,	quit);
			switch (loadState)
			{
				case FIELD_STATE:
					manager.getStates()->back()->Pause();
					manager.getStates()->pop_back();
					manager.getStates()->back()->Resume();
					break;
				case MENU_STATE:
					manager.getStates()->back()->Pause();
					manager.changeState(&menu);
					break;
				case SAME_STATE:
					break;
			}
		}

		//update logic, reset movement-based timer
		manager.Update();
		manager.getWorld()->getTime()->Restart();

		//draw objects to screen
		SDL_RenderClear(Win.getRenderer());
		manager.Draw();
		SDL_RenderPresent(Win.getRenderer());
	}

	//close things
	Mix_FreeMusic(music);
	Mix_CloseAudio();

	SDL_DestroyTexture(bg);
	return 0;
}

