#include "actor.h"

const int ANIM_DOWN = 0;
const int ANIM_LEFT = 1;
const int ANIM_RIGHT = 2;
const int ANIM_UP = 3;
const int ANIM_TOPLEFT = 4;
const int ANIM_BOTLEFT = 5;
const int ANIM_TOPRIGHT = 6;
const int ANIM_BOTRIGHT = 7;

const int MAX_FRAMES = 4;
const int WALK_ANIMATION_TICKS = 180;

//Actor speed is in pixels per second.
const int VELOCITY = 65;

Tile::Tile(GameWorld *World, SDL_Texture* sprite, float x, float y, SDL_Rect* srcClip)
	:mPosx(x), mPosy(y), mWorld(World)
{
	//set sprite texture
	mSprite = sprite;

	//if no source clip provided, full image is set to position (0,0); otherwise, clip is used
	if (srcClip == NULL)
	{
		mSpriteClip.x = 0;
		mSpriteClip.y = 0;
		SDL_QueryTexture(mSprite, NULL, NULL, &mSpriteClip.w, &mSpriteClip.h);
	}
	else
		mSpriteClip = *srcClip;
}

Tile::~Tile()
{
	//sprite sheet to be deleted seperately, by the Level or the GameWorld.
}

void Tile::drawAnimate(Window &window)
{
	//use window to draw objects
	//sprites will be drawn relative to the camera view
	window.Draw(mSprite, mPosx - mWorld->getCamera()->view.x, mPosy - mWorld->getCamera()->view.y, &mSpriteClip);
}

Actor::Actor(GameWorld *World, SDL_Texture* sprite, float x, float y, SDL_Rect* srcClip, SDL_Rect* collisionBox, int colX, int colY)
	:Tile(World, sprite, x, y, srcClip), mVelx(0), mVely(0), mColBoxX(colX), mColBoxY(colY), mAnimState(0), mAnimFrame(0), mNextAnimState(0)
{
	//if no collision box provided, full image is set to collision box; otherwise, collision box is used
	if (collisionBox == NULL)
	{
		mCollisionBox.x = x;
		mCollisionBox.y = y;
		mCollisionBox.w = getSpriteClip()->w;
		mCollisionBox.h = getSpriteClip()->h;
	}
	else
		mCollisionBox = *collisionBox;

	mAnimTimer.Start();
}

Actor::~Actor()
{
}

void Actor::moveLogic()
{
}

void Actor::move(float ticks)
{
	//limits acceleration
	if (getVelx() > VELOCITY)
		setVelx(VELOCITY);
	if (getVelx() < -1 * VELOCITY)
		setVelx(-1 * VELOCITY);
	if (getVely() > VELOCITY)
		setVely(VELOCITY);
	if (getVely() < -1 * VELOCITY)
		setVely(-1 * VELOCITY);

	//move position
	setPosx(getPosx() + getVelx() * ticks / 1000.f);
	setPosy(getPosy() + getVely() * ticks / 1000.f);

	//moves collision box to correct position
	getCollisionBox()->x = getPosx() + getColx();
	getCollisionBox()->y = getPosy() + getColy();
}

void Actor::unMove(int axis, float value)
{
	//(2 means both)

	//move position (0 means X)
	if (axis == 0 || axis == 2)
	{
		setPosx(getPosx() - value);
		getCollisionBox()->x = getPosx() + getColx();
	}
	//(1 means Y)
	if (axis == 1 || axis == 2)
	{
		setPosy(getPosy() - value);
		getCollisionBox()->y = getPosy() + getColy();
	}
}

void Actor::drawRect(Window &window)
{
	SDL_Rect colBox = mCollisionBox;
	colBox.x -= getWorld()->getCamera()->view.x;
	colBox.y -= getWorld()->getCamera()->view.y;

	colBox.x *= SIZE_FACTOR;
	colBox.y *= SIZE_FACTOR;
	colBox.w *= SIZE_FACTOR;
	colBox.h *= SIZE_FACTOR;

	window.Draw(window.getGreen(), colBox);
}

void Actor::animate()
{
	isMoving = true;		//indicates if actor is moving in this frame
	frame3 = false;		//indicates if the "3rd frame" is being shown (same as frame 1)
	//frames animate as: 0, 1, 2, 1, 0, 1, 2, 1, 0 ...

	//If direction is changed, change animation state immediately
	//RIGHT
	if (getVelx() > 0 && getVely() == 0)
		mNextAnimState = ANIM_RIGHT;

	//LEFT
	else if (getVelx() < 0 && getVely() == 0)
		mNextAnimState = ANIM_LEFT;

	//UP
	else if (getVelx() == 0 && getVely() < 0)
		mNextAnimState = ANIM_UP;

	//DOWN
	else if (getVelx() == 0 && getVely() > 0)
		mNextAnimState = ANIM_DOWN;

	//BOTTOM RIGHT
	else if (getVelx() > 0 && getVely() > 0)
		mNextAnimState = ANIM_BOTRIGHT;

	//BOTTOM LEFT
	else if (getVelx() < 0 && getVely() > 0)
		mNextAnimState = ANIM_BOTLEFT;

	//TOP LEFT
	else if (getVelx() < 0 && getVely() < 0)
		mNextAnimState = ANIM_TOPLEFT;

	//TOP RIGHT
	else if (getVelx() > 0 && getVely() < 0)
		mNextAnimState = ANIM_TOPRIGHT;

	else
		isMoving = false;


	//If next animation state is different from previous, change immediately (facing different direction)
	if (mNextAnimState != mAnimState)
		mAnimState = mNextAnimState;

	//Otherwise, if it is the "third frame", set frame3 to true
	if (mAnimFrame == 3)
		frame3 = true;

	//If the actor begins to move, immediately start playing walking animation.
	if (isMoving && mWasNotMoving)
	{
		mAnimFrame = 2;
		mAnimTimer.Restart();
		mWasNotMoving = false;
	}

	//Change animation frame every couple of ticks
	bool nextFrame = mAnimTimer.Ticks() > WALK_ANIMATION_TICKS;
	if (nextFrame)
	{
		mAnimTimer.Restart();

		//RIGHT
		if (getVelx() > 0 && getVely() == 0)
		{
			mAnimState = ANIM_RIGHT;
			mAnimFrame++;
		}

		//LEFT
		else if (getVelx() < 0 && getVely() == 0)
		{
			mAnimState = ANIM_LEFT;
			mAnimFrame++;
		}

		//UP
		else if (getVelx() == 0 && getVely() < 0)
		{
			mAnimState = ANIM_UP;
			mAnimFrame++;
		}

		//DOWN
		else if (getVelx() == 0 && getVely() > 0)
		{
			mAnimState = ANIM_DOWN;
			mAnimFrame++;
		}

		//BOTTOM RIGHT
		else if (getVelx() > 0 && getVely() > 0)
		{
			mAnimState = ANIM_BOTRIGHT;
			mAnimFrame++;
		}

		//BOTTOM LEFT
		else if (getVelx() < 0 && getVely() > 0)
		{
			mAnimState = ANIM_BOTLEFT;
			mAnimFrame++;
		}

		//TOP LEFT
		else if (getVelx() < 0 && getVely() < 0)
		{
			mAnimState = ANIM_TOPLEFT;
			mAnimFrame++;
		}

		//TOP RIGHT
		else if (getVelx() > 0 && getVely() < 0)
		{
			mAnimState = ANIM_TOPRIGHT;
			mAnimFrame++;
		}

		if (mAnimFrame == 3)
			frame3 = true;
	}

	//use window to draw objects
	//sprites will be drawn relative to the camera view

	//Reset animation upon standstill
	if (getVelx() == 0 && getVely() == 0)
	{
		mAnimFrame = 1;
		mWasNotMoving = true;
	}


	//Using frame variable so that frame 3 shows same image as frame 1
	//Allows for mAnimFrame variable to move from 0-3 freely
	mAnimFrame = mAnimFrame % MAX_FRAMES;
	
}

void Actor::draw(Window &window)
{
	int frame = mAnimFrame;

	if (frame3)
		frame = 1;


	SDL_Rect animClip = { getSpriteClip()->x + (frame * getWorld()->getCharSprites()->tileW), getSpriteClip()->y + (mAnimState * getWorld()->getCharSprites()->tileH), 
		getWorld()->getCharSprites()->tileW, getWorld()->getCharSprites()->tileH };

	window.Draw(getSprite(), getPosx() - getWorld()->getCamera()->view.x, getPosy() - getWorld()->getCamera()->view.y, &animClip);
}

Player::Player(GameWorld* World, SDL_Texture* sprite, float x, float y, SDL_Rect* srcClip, SDL_Rect* collisionBox, int colX, int colY)
:Actor(World, sprite, x, y, srcClip, collisionBox, colX, colY)
{
}

Player::~Player()
{
}

void Player::handleInput() 
{
	/*const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	//if key is pressed, velocity increased
	if(event.type == SDL_KEYDOWN)
	{
		switch(event.key.keysym.sym)
		{
		case SDLK_UP:
			setVely(-1 * VELOCITY);
			break;
		case SDLK_DOWN:
			setVely(VELOCITY);
			break;
		case SDLK_LEFT:
			setVelx(-1 * VELOCITY);
			break;
		case SDLK_RIGHT:
			setVelx(VELOCITY);
			break;
		case SDLK_RETURN:
			setPosx(getWorld()->getPlayerSpawnPoint()->x);
			setPosy(getWorld()->getPlayerSpawnPoint()->y);
			setVelx(0);
			setVely(0);
			break;
		}

		//If opposite keys are pressed, prevents drifting motion (doesn't work)
		if (event.key.keysym.sym == SDLK_RIGHT && event.key.keysym.sym == SDLK_LEFT)
			setVelx(0);

		if (event.key.keysym.sym == SDLK_UP && event.key.keysym.sym == SDLK_DOWN)
			setVely(0);

	}

	//if key is depressed, velocity returns to 0
	if(event.type == SDL_KEYUP)
	{
		switch(event.key.keysym.sym)
		{
		case SDLK_UP:
			//if(getVely() < 0)
			//	setVely(0);
			if(getVely() < 0)
				setVely(getVely() + VELOCITY);
			break;
		case SDLK_DOWN:
			//if (getVely() > 0)
			//	setVely(0);
			if (getVely() > 0)
				setVely(getVely() - VELOCITY);
			break;
		case SDLK_LEFT:
			//if (getVelx() < 0)
			//	setVelx(0);
			if (getVelx() < 0)
				setVelx(getVelx() + VELOCITY);
			break;
		case SDLK_RIGHT:
			//if (getVelx() > 0)
			//	setVelx(0);
			if (getVelx() > 0)
				setVelx(getVelx() - VELOCITY);
			break;
		}
	}*/
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	//UP
	if (keystate[SDL_SCANCODE_UP])
		setVely(-1 * VELOCITY);
	else if (getVely() < 0)
		setVely(getVely() + VELOCITY);

	//DOWN
	if (keystate[SDL_SCANCODE_DOWN])
		setVely(VELOCITY);
	else if (getVely() > 0)
		setVely(getVely() - VELOCITY);

	//RIGHT
	if (keystate[SDL_SCANCODE_RIGHT])
		setVelx(VELOCITY);
	else if (getVelx() > 0)
		setVelx(getVelx() - VELOCITY);

	//LEFT
	if (keystate[SDL_SCANCODE_LEFT])
		setVelx(-1 * VELOCITY);
	else if (getVelx() < 0)
		setVelx(getVelx() + VELOCITY);

	//RETURN
	if (keystate[SDL_SCANCODE_RETURN])
	{
		setPosx(getWorld()->getPlayerSpawnPoint()->x);
		setPosy(getWorld()->getPlayerSpawnPoint()->y);
		setVelx(0);
		setVely(0);
	}

	//If opposite keys are pressed, no movement
	if (keystate[SDL_SCANCODE_UP] && keystate[SDL_SCANCODE_DOWN])
		setVely(0);

	if (keystate[SDL_SCANCODE_LEFT] && keystate[SDL_SCANCODE_RIGHT])
		setVelx(0);
}

void Player::move(float ticks)
{
	//limits acceleration to 0
	if (getVelx() > VELOCITY)
		setVelx(VELOCITY);
	if (getVelx() < -1*VELOCITY)
		setVelx(-1*VELOCITY);
	if (getVely() > VELOCITY)
		setVely(VELOCITY);
	if (getVely() < -1*VELOCITY)
		setVely(-1*VELOCITY);

	//move position
	setPosx(getPosx() + getVelx() * ticks / 1000.f);
	setPosy(getPosy() + getVely() * ticks / 1000.f);

	//move collision box to center of sprite
	getCollisionBox()->x = getPosx() + getColx();
	getCollisionBox()->y = getPosy() + getColy();

	//places camera around player
	setCamera();
}

void Player::setCamera()
{
	//Moves camera to center on Player.
	getWorld()->getCamera()->view.x = getPosx() + (getSpriteClip()->w/2) - SCREEN_WIDTH/(2 * SIZE_FACTOR);
	getWorld()->getCamera()->view.y = getPosy() + (getSpriteClip()->h / 2) - SCREEN_HEIGHT / (2 * SIZE_FACTOR);
	getWorld()->getCamera()->velX = getVelx();
	getWorld()->getCamera()->velY = getVely();

	int levelWidth = getWorld()->getLevel()->getWidth() * getWorld()->getLevel()->getTileWidth();
	int levelHeight = getWorld()->getLevel()->getHeight() * getWorld()->getLevel()->getTileHeight();

	//If the level is larger than the screen, make sure the screen never goes off bounds.
	//If the level is smaller, center level within screen view.

	//adjust for screen width
	if(levelWidth >= SCREEN_WIDTH/SIZE_FACTOR)
	{
		if (getWorld()->getCamera()->view.x < 0)
		{
			getWorld()->getCamera()->view.x = 0;
			getWorld()->getCamera()->velX = 0;
		}

		if (getWorld()->getCamera()->view.x > levelWidth - getWorld()->getCamera()->view.w / SIZE_FACTOR)
		{
			getWorld()->getCamera()->view.x = levelWidth - getWorld()->getCamera()->view.w / SIZE_FACTOR;
			getWorld()->getCamera()->velX = 0;
		}
	}
	else
	{
		getWorld()->getCamera()->view.x = levelWidth / 2 - SCREEN_WIDTH /(2 * SIZE_FACTOR);
		getWorld()->getCamera()->velX = 0;
	}


	//adjust for screen height
	if (levelHeight >= SCREEN_HEIGHT / SIZE_FACTOR)
	{
		if (getWorld()->getCamera()->view.y < 0)
			getWorld()->getCamera()->view.y = 0;

		if (getWorld()->getCamera()->view.y > levelHeight - getWorld()->getCamera()->view.h / SIZE_FACTOR)
			getWorld()->getCamera()->view.y = levelHeight - getWorld()->getCamera()->view.h / SIZE_FACTOR;

		getWorld()->getCamera()->velY = 0;
	}
	else
	{
		getWorld()->getCamera()->view.y = levelHeight / 2 - SCREEN_HEIGHT / (2 * SIZE_FACTOR);
		getWorld()->getCamera()->velX = 0;
	}
}