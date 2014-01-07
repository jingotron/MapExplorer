#ifndef ACTOR_H
#define ACTOR_H

#include <string>
#include "window.h"
#include "gameworld.h"
#include "timer.h"


#if defined(_MSC_VER)
#include <SDL.h>
#elif defined(__clang__)
#include <SDL2/SDL.h>
#else
#include <SDL2/SDL.h>
#endif

class Tile
{
public:

	Tile(GameWorld* World, SDL_Texture* sprite, float x = 0, float y = 0, SDL_Rect* srcClip = NULL);
	
	virtual ~Tile();

	//Draws Tile at designated position.
	//Draws relative to camera view. (mCamera is part of GameWorld class)
	virtual void drawAnimate(Window &window);

	//get private variables
	SDL_Texture* getSprite() { return mSprite; }
	SDL_Rect* getSpriteClip() { return &mSpriteClip; }
	GameWorld* getWorld() { return mWorld; }
	float getPosx() const { return mPosx; }
	float getPosy() const { return mPosy; }

	//set private variables
	void setPosx(float value) { mPosx = value; }
	void setPosy(float value) { mPosy = value; }

private:

	float mPosx, mPosy;				 //sprite position
	SDL_Texture* mSprite;            //sprite sheet texture 
	SDL_Rect mSpriteClip;			 //sprite clip rect
	GameWorld *mWorld;				 //game world

};

class Actor : public Tile
{
public:
	Actor(GameWorld* World, SDL_Texture* sprite, float x = 0, float y = 0, SDL_Rect* srcClip = NULL, SDL_Rect* collisionBox = NULL, int colX = 0, int colY = 0);
	virtual ~Actor();

	virtual void moveLogic();

	//Move actor based on velocity.
	virtual void move(float ticks);

	//Unmove actors due to collision detection; should only be used by detectCollision().
	virtual void unMove(int axis, float value);

	//Draw collision box at actor position, relative to camera view. (mCamera is part of GameWorld class)
	void drawRect(Window &window);

	//Detect collisions against another collision box. 
	//Usually called by the GameWorld against all other actors.
	bool detectCollision(SDL_Rect &B);

	//get private variables
	SDL_Rect* getCollisionBox() { return &mCollisionBox; }
	float getVelx() const { return mVelx; }
	float getVely() const { return mVely; }
	int getColx() const { return mColBoxX; }
	int getColy() const { return mColBoxY; }
	int getAnimFrame() const { return mAnimFrame; }
	int getAnimState() const { return mAnimState; }

	//set private variables
	void setVelx(float value) { mVelx = value; }
	void setVely(float value) { mVely = value; }
	int setAnimFrame(int frame) { mAnimFrame = frame; }
	int setAnimState(int status) { mAnimState = status; }

	//animate moving actors
	virtual void animate();
	virtual void draw(Window& window);

private:
	float mVelx, mVely;				 //x and y velocities
	SDL_Rect mCollisionBox;          //collision box rectangle
	int mColBoxX, mColBoxY;			 //collision box offset position relative to sprite position
	int mAnimFrame, mAnimState, mNextAnimState; //ints indicating which frame and direction the sprite should be animated in
	Timer mAnimTimer;				 //timer for animation
	bool mWasNotMoving;				 //boolean indicating whether the actor was not moving last frame
	bool isMoving;					 //boolean indicating whether the actor is moving this frame
	bool frame3;					 //boolean indicating whether the sprite is in "frame 3", which is the same as frame 1 (not frame 0)
};

class Player: public Actor
{
public:
	Player(GameWorld* World, SDL_Texture* sprite, float x = 0, float y = 0, SDL_Rect* srcClip = NULL, SDL_Rect* collisionBox = NULL, int colX = 0, int colY = 0);
	virtual ~Player();

	//Handles player input.
	void handleInput();

	//Moves player.
	virtual void move(float ticks);

	//Centers camera on player.
	void setCamera();

private:
};

#endif