#ifndef GAMEWORLD_H
#define GAMEWORLD_H

#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "SDL.h"
#undef main
#include "window.h"
#include "base64.h"
#include "timer.h"
#include "rapidxml.hpp"
#include <iostream>

class Actor;
class Player;

struct Camera
{
	Camera()
	:velX(0), velY(0)
	{
	}

	//Position is stored in this rect
	SDL_Rect view;
	int velX, velY;

};

struct Tileset
{
	Tileset(SDL_Texture* source, int fgid, int width, int height, int tileWidth, int tileHeight, int transparency)
	{
		image = source;
		firstGid = fgid;
		w = width;
		h = height;
		tileW = tileWidth;
		tileH = tileHeight;
		alpha = transparency;
	}

	~Tileset() {
		SDL_DestroyTexture(image); }

	//first global tile id of the Tileset.
	//This comes into play with multiple Tilesets.
	int firstGid;				

	//Source image of Tileset.
	SDL_Texture* image;			

	//w: Width of the Tileset in tiles, not pixels.
	//h: Height of the Tileset in tiles, not pixels.
	//tileW & tileH: Width and height of each tile in pixels.
	int w, h, tileW, tileH;			

	//value of color to be set transparent.
	int alpha;					
};

//Comparing Tilesets by their first global tiled ids.
inline bool operator> (const Tileset& A, const Tileset& B) {
	return A.firstGid > B.firstGid; }

inline bool operator<= (const Tileset&A, const Tileset& B) {
	return !(A > B); }

class Level
{
public:

	//Dynamically allocating an array of gid, for map
	Level(int width, int height, int tileW, int tileH, SDL_Texture* parallax = NULL)
		:mWidth(width), mHeight(height), mTileWidth(tileW), mTileHeight(tileH), mParallaxBg(parallax)
	{
		tileMap = new int*[mWidth];
		for(int i = 0; i < mWidth; i++)
			tileMap[i] = new int[mHeight];

		tileMap2 = new int*[mWidth];
		for (int i = 0; i < mWidth; i++)
			tileMap2[i] = new int[mHeight];

		overMap = new int*[mWidth];
		for (int i = 0; i < mWidth; i++)
			overMap[i] = new int[mHeight];

		overMap2 = new int*[mWidth];
		for (int i = 0; i < mWidth; i++)
			overMap2[i] = new int[mHeight];
	}

	//Deleting dynamically allocated array, and all Tileset data associated with the Level.
	~Level()
	{
		for (int i = 0; i < mWidth; i++)
			delete tileMap[i];
		delete tileMap;

		for (int i = 0; i < mWidth; i++)
			delete tileMap2[i];
		delete tileMap2;

		for (int i = 0; i < mWidth; i++)
			delete overMap[i];
		delete overMap;


		for (int i = 0; i < mWidth; i++)
			delete overMap2[i];
		delete overMap2;

		for(std::set<Tileset*>::iterator iter = mTileset.begin(); iter != mTileset.end(); iter++)
			delete *iter;

	}

	//Return the width of the level in tiles, not pixels.
	int getWidth() {return mWidth;}

	//Return the height of the level in tiles, not pixels.
	int getHeight() {return mHeight;}

	//Return the width of each tile in pixels.
	int getTileWidth() {return mTileWidth;}

	//Return the height of each tile in pixels.
	int getTileHeight() {return mTileHeight;}

	//Return the pointer to the first element of the tilemap.
	int** getTileMap() { return tileMap; }

	//Return pointer to the Tileset set. 
	std::set<Tileset*>* getTileSet() {return &mTileset;}

	//Return pointer to the set of solid gids.
	std::set<int>* getSolidGid() { return &mSolidGid; }

	int** getTileMap2() { return tileMap2; }

	int** getOverMap() { return overMap; }

	int** getOverMap2() { return overMap2; }

	SDL_Texture* getParallax() { return mParallaxBg; }

private:
	int mWidth, mHeight, mTileWidth, mTileHeight;
	bool mVisible;
	int** tileMap;
	int** tileMap2;
	int** overMap;
	int** overMap2;

	std::set<Tileset*> mTileset;
	std::set<int> mSolidGid;
	SDL_Texture* mParallaxBg;

};

class GameWorld
{
public:
	GameWorld(Window* win);
	~GameWorld();

	//takes in a rect of a collision box in level and returns a rect indicating the tiles which are overlapping.
	//returned rect is in terms of tiles, not pixels.
	SDL_Rect tileRangeOverlap(SDL_Rect &collision);

	//corrects actor movement based on collisions.
	void correctCollision(std::vector<Actor*>::iterator curActor);

	void moveActors();
	void drawActors();
	void drawBackground(int** tilemap);
	void parallaxBg();

	void spawnPlayer(SDL_Texture *sprite, SDL_Rect* clip = NULL, SDL_Rect* colBox = NULL, int colBoxX = 0, int colBoxY = 0, int x = 0, int y = 0);
	void spawnActor(SDL_Texture *sprite, int x = 0, int y = 0, SDL_Rect* collisionBox = NULL);

	//Toggle collision box visibility.
	void toggleColBox();

	void openCharTiles(std::string imageSource, int tileWidth, int tileHeight, int alpha = 0)
	{
		SDL_Texture* source = mWindow->LoadImage(imageSource);
		int w,h = 0;

		SDL_QueryTexture(source, NULL,  NULL, &w, &h);
		mCharSprites = new Tileset(source, 1, w, h, tileWidth, tileHeight, alpha);
	}

	SDL_Rect getCharClip(int index);

	//Opens a .TMX map file.
	void openMap(std::string title);

	Player* getPlayer() { return mPlayer; }
	Camera* getCamera() { return &mCamera; }
	Level* getLevel() { return mLevel; }
	SDL_Rect* getPlayerSpawnPoint() { return &mPlayerSpawnPoint; }
	void setPlayerSpawnPoint(SDL_Rect spawn) { setPlayerSpawnPoint(spawn); }
	void setLoadLevel(bool load) { mLoadNextLevel = load; }
	bool shouldLoadNextLevel() { return mLoadNextLevel; }
	std::string getNextLevel() { return mNextLevel; }
	void setNextLevel(std::string source) {mNextLevel = source; }
	Tileset* getCharSprites() { return mCharSprites; }
	bool getDebugInfo() { return mDebugOn; }
	Window* getWin() { return mWindow; }
	std::vector<Actor*>* getActorList() { return &actorList; }
	Timer* getTime() { return &mDeltaTime; }

private:
	std::vector<Actor*> actorList;

	//Actor list sorted by bottom most to top most position of sprite.
	//Used when ordering Actors in the orderedActors vector, so that Actors further down on screen are drawn first.
	//This allows actors behind other Actors to actually appear behind other actors.
	std::vector<Actor*> orderedActorList; 

	std::vector<Tileset*> actorSprites;

	bool mDebugOn;
	Player* mPlayer;
	Camera mCamera; //contains dimensions of screen
	Level *mLevel;
	Window *mWindow;
	int mNextPlayerSpawn;
	SDL_Rect mPlayerSpawnPoint;
	bool mLoadNextLevel;
	std::string mNextLevel;
	Tileset* mCharSprites;
	Timer mDeltaTime;


	//Corrects collision between an Actor and another collision box.
	//Assumes that there exists a collision.
	//Use this function only after using detectOverlap().

	void correctOverlap(int A, int B, int C, int D, std::vector<Actor*>::iterator curActor, SDL_Rect *overlap);
};

#endif