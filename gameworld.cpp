#include "gameworld.h"
#include "actor.h"

const int X = 0;
const int Y = 1;
const int BOTH = 2;

const int INDEX_PLAYER = 0;

//Slide variable should be moved to a private Actor variable.
const int MIN_SLIDE = 10;

//Compares Actors by their y position.
//Used when ordering Actors in the orderedActors vector, so that Actors further down on screen are drawn first.
//This allows actors behind other Actors to actually appear behind other actors.
bool compareByPosY(Actor* A, Actor* B)
{
	return A->getPosy() < B->getPosy();
}

//struct containing a alue and its absolute value.
//Used for detecting distances between sides of two collision boxes.
struct intAbs
{
	intAbs(int value1, int value2) :intVal(value1), absVal(value2) {}

	int intVal;
	int absVal;

	//structs compared by absolute values
	bool operator== (intAbs &c1)
	{
		return c1.absVal == this->absVal;
	}

	bool operator!= (intAbs &c1)
	{
		return !operator==(c1);
	}
};

//Returns the intAbs struct with the smallest absolute value
inline intAbs *deltaMin(intAbs *a, intAbs *b)
{
	if(a->absVal > b->absVal)
		return b;
	else
		return a;
}

//returns absolute value of an integer
inline int absval(int a)
{
	if(a < 0)
		return a * -1;
	else
		return a;
}

//detects if rectangles overlap, writes in edge difference values if variables are provided
bool detectOverlap(SDL_Rect *A, SDL_Rect *B, int *a = NULL, int *b = NULL, int *c = NULL, int *d = NULL)
{
	int leftA = A->x;
	int rightA = A->x + A->w;
	int topA = A->y;
	int bottomA = A->y + A->h;

	int leftB = B->x;
	int rightB = B->x + B->w;
	int topB = B->y;
	int bottomB = B->y + B->h;

	if (a != NULL)
		*a = topA - bottomB;
	if (b != NULL)
		*b = bottomA - topB;
	if (c != NULL)
		*c = leftA - rightB;
	if (d != NULL)
		*d = rightA - leftB;

	return !(bottomA <= topB || topA >= bottomB ||
		leftA >= rightB || rightA <= leftB);
}




GameWorld::GameWorld(Window* win)
	:mPlayer(NULL), mDebugOn(false), mLevel(NULL), mWindow(win), mNextPlayerSpawn(0), 
	mLoadNextLevel(false), mNextLevel("")
{
	SDL_Rect boxSize;

	boxSize.x = 10;
	boxSize.y = 610;
	boxSize.w = 1260;
	boxSize.h = 150;


	//These constants are found in the window.h file.
	//The camera dimensions are the screen's dimensions.
	mCamera.view.x = 0;
	mCamera.view.y = 0;
	mCamera.view.w = SCREEN_WIDTH;
	mCamera.view.h = SCREEN_HEIGHT;

	mPlayerSpawnPoint.x = 0;
	mPlayerSpawnPoint.y = 0;
	mPlayerSpawnPoint.w = 0;
	mPlayerSpawnPoint.h = 0;

	mDeltaTime.Start();
}

GameWorld::~GameWorld()
{
	//deleting all actors in world
	for (std::vector<Actor*>::iterator iter = actorList.begin(); iter != actorList.end(); iter++)
	{
		delete *iter;
	}
	actorList.clear();

	//deleting the level
	delete mLevel;

	delete mCharSprites;

	//do not delete the window; this will be done seperately in the source code using SDL functions.

}

//takes in a rect of a collision box and returns a rect indicating which tiles in the level are being overlapped.
//returned rect is in terms of tiles, not pixels.
//Used to detect collisions between Actors and background tiles.
SDL_Rect GameWorld::tileRangeOverlap(SDL_Rect &collision)
{
	SDL_Rect overlap;
	overlap.x = collision.x / mLevel->getTileWidth();
	overlap.w = (collision.x + collision.w)/ mLevel->getTileWidth() + 1;

	//Checking if collision box goes over the edges of the level
	if(overlap.x < 0)
		overlap.x = 0;

	if(overlap.w > mLevel->getWidth() - overlap.x)
		overlap.w = mLevel->getWidth() - overlap.x;

	overlap.y = collision.y / mLevel->getTileHeight();
	overlap.h = (collision.y + collision.h) / mLevel->getTileHeight() + 1;

	if(overlap.y < 0)
		overlap.y = 0;

	if (overlap.h > mLevel->getHeight() - overlap.y)
		overlap.h = mLevel->getHeight() - overlap.y;

	return overlap;
}

void GameWorld::correctOverlap(int A, int B, int C, int D, std::vector<Actor*>::iterator curActor, SDL_Rect* overlap)
{
	//Creating four structs for the edge differences and their absolute values. This allows for finding the minimum value
	//in terms of absolute values, yet knowing which signed integer to use to "unmove" the rectangles.

	intAbs deltaA(A, absval(A));
	intAbs deltaB(B, absval(B));
	intAbs deltaC(C, absval(C));
	intAbs deltaD(D, absval(D));

	intAbs *minY = deltaMin(&deltaA, &deltaB);
	intAbs *minX = deltaMin(&deltaC, &deltaD);
	intAbs *minimum = deltaMin(minX, minY);

	//The gameworld will unmove the actor on each axis seperately if a collision occurs. If a collision is still occuring
	//after the unmove on a certain axis, the gameworld "unmove the unmove", or restore the actor to its initial movement.

	//if the collision occured on the y axis
	if (minimum == &deltaA || minimum == &deltaB)
	{
		(*curActor)->unMove(Y, minimum->intVal);
		if (detectOverlap((*curActor)->getCollisionBox(), overlap))
			(*curActor)->unMove(Y, -1 * minimum->intVal);
		else if (minX->absVal < MIN_SLIDE && (*curActor)->getVelx() == 0)
		{
			if (minX->intVal >= 0)
				(*curActor)->unMove(X, minimum->absVal);
			else if (minX->intVal < 0)
				(*curActor)->unMove(X, -1 * minimum->absVal);
		}
	}

	//if the collision occured on the x axis
	if (minimum == &deltaC || minimum == &deltaD)
	{
		(*curActor)->unMove(X, minimum->intVal);
		if (detectOverlap((*curActor)->getCollisionBox(), overlap))
			(*curActor)->unMove(X, -1 * minimum->intVal);
		else if (minY->absVal < MIN_SLIDE && (*curActor)->getVely() == 0)
		{
			if (minY->intVal >= 0)
				(*curActor)->unMove(Y, minimum->absVal);
			else if (minY->intVal < 0)
				(*curActor)->unMove(Y, -1 * minimum->absVal);
		}
	}

	//Because deltaMin() returns the first argument if the arguments are equal,
	//an Actor colliding exactly on the corner  of a collision box will be pushed along the x axis.
	//For tiles arranged horizontally, this causes internal edge collision as the Actor is first 
	//pushed into the first tile, unMoved, and the moved back away.
	//This is fixed for background tiles, assuming the Actor's collision box is smaller or equal to the dimensions of the tile.
	//For larger enemies, try using multiple tile sized collision boxes.
}

//BUG: OBJECT CAN TUNNEL BETWEEN TWO OBJECTS IF IT CAN ALMOST FIT INBETWEEN. NOT A MAJOR PROBLEM(?)
void GameWorld::correctCollision(std::vector<Actor*>::iterator curActor)
{
	int A, B, C, D;		//four values representing edge differences. These will determine how far to move a rectangle back
	//if it collides, in order to allow movement of rectangles right up to each other's edges.
	//In addition, the smallest edge difference will provide the direction in which the rectangle was
	//most likely moving when a collision was caused, due to the minuteness of frame to frame movement.

	//A = topA - bottomB
	//B = bottomA - topB
	//C = leftA - rightB
	//D = rightA - leftB

	//checking collision against other Actors
	for(std::vector<Actor*>::iterator iter = actorList.begin(); iter != actorList.end(); iter++)
	{
		if (iter != curActor && detectOverlap((*curActor)->getCollisionBox(), (*iter)->getCollisionBox(), &A, &B, &C, &D))
			correctOverlap(A, B, C, D, curActor, (*iter)->getCollisionBox());
	}

	//NOTE: THIS CAN BE DONE FOR LARGER ENEMIES
	//NUM_TILES_COLLIDE = ACTOR_SIZE / TILE_SIZE + 1
	//Merge collision box on the side corresponding to the direction of the velocity on that axis.

	//checking collision against background tiles, assuming actor is never larger than the background tiles.
	//Thus, the Actor will at most overlap two background tiles along either axis.
	SDL_Rect overlap = tileRangeOverlap(*(*curActor)->getCollisionBox());
	for(int y = overlap.y; y < overlap.y + overlap.h; y++)
	{
		bool left, right, both;

		int x = overlap.x;

		//Checking for off map cases for left tile
		if(mLevel->getWidth() <= x)
			left = false;
		else
			left = mLevel->getSolidGid()->find(mLevel->getTileMap()[x][y]) != mLevel->getSolidGid()->end()
			|| mLevel->getSolidGid()->find(mLevel->getTileMap2()[x][y]) != mLevel->getSolidGid()->end();

		//Checking for off map cases for right tile
		if(mLevel->getWidth() <= x + 1)
			right = false;
		else
			right = mLevel->getSolidGid()->find(mLevel->getTileMap()[x + 1][y]) != mLevel->getSolidGid()->end()
			|| mLevel->getSolidGid()->find(mLevel->getTileMap2()[x + 1][y]) != mLevel->getSolidGid()->end();


		both = left && right;

		//if both, the collision boxes of the two tiles are merged into one. This prevents internal edge collision.
		if(both)
		{
			SDL_Rect collideTile = { x * mLevel->getTileWidth(), y * mLevel->getTileHeight(), 2 * mLevel->getTileWidth(), mLevel->getTileHeight() };
			if (detectOverlap((*curActor)->getCollisionBox(), &collideTile, &A, &B, &C, &D))
				correctOverlap(A, B, C, D, curActor, &collideTile);
		}
		else if(left)
		{
			SDL_Rect collideTile = {x * mLevel->getTileWidth(), y * mLevel->getTileHeight(), mLevel->getTileWidth(), mLevel->getTileHeight()};
			if (detectOverlap((*curActor)->getCollisionBox(), &collideTile, &A, &B, &C, &D))
				correctOverlap(A, B, C, D, curActor, &collideTile);
		}
			
		else if (right)
		{
			SDL_Rect collideTile = { (x+1) * mLevel->getTileWidth(), y * mLevel->getTileHeight(), mLevel->getTileWidth(), mLevel->getTileHeight() };
			if (detectOverlap((*curActor)->getCollisionBox(), &collideTile, &A, &B, &C, &D))
					correctOverlap(A, B, C, D, curActor, &collideTile);
		}

		else
		{
			;
		}
	}
	
	//fixes camera movement bug for collision detection
	//if (*curActor == mPlayer)
	mPlayer->setCamera();
}

void GameWorld::moveActors()
{
	//moves all actors, does collision correction
	for (std::vector<Actor*>::iterator iter = actorList.begin(); iter != actorList.end(); iter++)
	{
		(*iter)->moveLogic();
		(*iter)->move(mDeltaTime.Ticks());
		correctCollision(iter);
	}

}


void GameWorld::drawActors()
{
	//Making sure to draw Actors in correct order, from lowest on screen to highest.
	orderedActorList = actorList;
	std::sort(orderedActorList.begin(), orderedActorList.end(), compareByPosY);

	//draw all actors to screen
	for (std::vector<Actor*>::iterator iter = orderedActorList.begin(); iter != orderedActorList.end(); iter++)
	{
		(*iter)->animate();
		(*iter)->draw(*mWindow);
		if (mDebugOn)
			(*iter)->drawRect(*mWindow);
	}
}

void GameWorld::drawBackground(int** tilemap)
{
	if(mLevel == NULL)
		return;
	if(tilemap == NULL)
		return;

	//assuming first tileset is background tiles.

	//Drawing only the tiles in view of the camera.
	SDL_Rect visibleTiles = tileRangeOverlap(mCamera.view);

	std::set<Tileset*>::iterator selectedTileset = mLevel->getTileSet()->begin();

	//For each visible tile...
	for(int y = visibleTiles.y; y <= visibleTiles.y + visibleTiles.h && y < mLevel->getHeight(); y++)
	{
		for(int x = visibleTiles.x; x <= visibleTiles.x + visibleTiles.w && x < mLevel->getWidth(); x++)
		{
			//Get the relative tile ID by the subtracting the first global id of the Tileset.
			int tileGidRelative = tilemap[x][y] - (*selectedTileset)->firstGid;

			//Finding the width of the tileset source image in tiles.
			int numTilesX = (*selectedTileset)->w / (*selectedTileset)->tileW;

			//Finding the index of the particular tile.
			int tileIndexX = tileGidRelative % numTilesX;
			int tileIndexY = tileGidRelative / numTilesX;

			//Creating the clip of the tileset source image from the previous data.
			SDL_Rect tileClip = { tileIndexX  * mLevel->getTileWidth(), tileIndexY * mLevel->getTileHeight(), (*selectedTileset)->tileW, (*selectedTileset)->tileH };

			int tileLocationX = x * mLevel->getTileWidth();
			int tileLocationY = y * mLevel->getTileHeight();

			//Drawing the tile relative to the camera.
			mWindow->Draw((*selectedTileset)->image, tileLocationX - mCamera.view.x, tileLocationY - mCamera.view.y, &tileClip);
		}
	}
}

void GameWorld::parallaxBg()
{
	if(getLevel()->getParallax() == NULL)
		return;

	float x, y;

	if (mCamera.view.y < 0)
		y = 0.5 * mCamera.view.y;
	else
		y = -0.5 * mCamera.view.y;

	if(mCamera.view.x < 0)
		x = 0.5 * mCamera.view.x;
	else
		x = -0.5 * mCamera.view.x;


	float pw = (mLevel->getTileWidth() * mLevel->getWidth() - mCamera.view.w) / 0.5 + mCamera.view.w;
	float ph = (mLevel->getTileHeight() * mLevel->getHeight() - mCamera.view.h) / 0.5 + mCamera.view.h;

	int w, h;
	SDL_QueryTexture(getLevel()->getParallax(), NULL, NULL, &w, &h);
	
	float blitX = pw / w + 2;
	float blitY = ph / h + 2;
	

	for(int n = 0; n < blitY; n++)
	{
		for(int m = 0; m < blitX; m++)
			mWindow->Draw(getLevel()->getParallax(), x + m * w, y + n * h);
	}
}

void GameWorld::spawnPlayer(SDL_Texture* sprite, SDL_Rect* clip, SDL_Rect* colBox, int colBoxX, int colBoxY, int x, int y)
{
	//spawns a new player
	Player *newPlayer = new Player(this, sprite, x, y, clip, colBox, colBoxX, colBoxY);
	actorList.push_back(newPlayer);
	mPlayer = newPlayer;
}

void GameWorld::spawnActor(SDL_Texture* sprite, int x, int y, SDL_Rect* collisionBox)
{
	//spawns a new actor
	Actor *newActor = new Actor(this, sprite, x, y, NULL, collisionBox, 0, 50);
	actorList.push_back(newActor);
}

void GameWorld::toggleColBox()
{
	mDebugOn = !mDebugOn;
}

SDL_Rect GameWorld::getCharClip(int index)
{
	int charIndexX = index % (mCharSprites->w / mCharSprites->tileW);
	int charIndexY = index / (mCharSprites->w / mCharSprites->tileW);
	SDL_Rect clip = { charIndexX * mCharSprites->tileW, charIndexY * mCharSprites->tileH, mCharSprites->tileW, mCharSprites->tileH };
	return clip;
}

void GameWorld::openMap(std::string title)
{
	//------------------------------------------FORMAT---------------------------------------------------------------//
	//- tiles that are solid will have property name = "solid" and either value "0" or "1" indicating soldness.
	//- one image per tileset.
	//- first tileset is the background tileset.
	//- an object layer "playerSpawn" with a rectangle indicating spawn location.
	//- to make player not go off edges of map, add invisible solid tiles around border.
	//- For parallax background: In map properties, add the property "parallaxBg" and the image file name as its value.

	//Layer format:

	//over layer 2
	//over layer, with non-solid tiles indicating things above actors
	//actor layer
	//background layer 2, with solid/passable tiles for objects with transparency
	//background layer 1, with solid/passable tiles

	//Character sprite sheets are stored in a single image opened by openCharTiles().
	//		  step1    stand    step2
	//down
	//left
	//right
	//up
	//topleft
	//botleft
	//topright
	//botright

	//Look at the Tiled website to see the format of the TMX XML tree.
	//-------------------------------------------------------------------------------------------------------------------//


	//reads in file, converts to string, and passes it to a xml_document object
	//this object parses the document and converts it to a DOM tree

	std::ifstream mapFile(title);
	std::ostringstream temp;
	std::string map;

	temp << mapFile.rdbuf();
	map = temp.str();
	
	rapidxml::xml_document<> mapData;
	mapData.parse<0>(&map[0]);

	//reads the first child of the xml root node, which is the map node
	//obtains map/tile dimensions and dynamically creates a tilemap array of tile IDs
	rapidxml::xml_node<> *mapProperties = mapData.first_node();
	int levelWidth = atoi(mapProperties->first_attribute("width")->value());
	int levelHeight = atoi(mapProperties->first_attribute("height")->value());
	int tileWidth = atoi(mapProperties->first_attribute("tilewidth")->value());
	int tileHeight = atoi(mapProperties->first_attribute("tileheight")->value());


	//Opening parallax image
	rapidxml::xml_node<> *mapPropertiesExtra = mapProperties->first_node("properties");
	if(mapPropertiesExtra != NULL)
		mapPropertiesExtra = mapPropertiesExtra->first_node("property");

	std::string parallaxSource;
	SDL_Texture* parallaxBg = NULL;
	rapidxml::xml_attribute<> *parallaxAttr = NULL;
	std::string parallaxProperty;

	if(mapPropertiesExtra != NULL)
		parallaxProperty = mapPropertiesExtra->first_attribute("name")->value();
	if(parallaxProperty == "parallaxBg")
		parallaxSource = mapPropertiesExtra->first_attribute("value")->value();

	if(parallaxSource != "")
		parallaxBg = mWindow->LoadImage(parallaxSource);

	mLevel = new Level(levelWidth, levelHeight, tileWidth, tileHeight, parallaxBg);


	//Going through the DOM tree
	for (rapidxml::xml_node<> *mapInfo = mapProperties->first_node(); mapInfo != NULL; mapInfo = mapInfo->next_sibling())
	{
		std::string nodeName = mapInfo->name();

		//tilesets
		if (nodeName == "tileset")
		{
			printf("Here is a tileset.\n");

			//loads tileset image from tileset node's child, image node
			//contains source location of image
			//loads image to a SDL_Texture;

			rapidxml::xml_node<> *image = mapInfo->first_node("image");
			SDL_Texture* tilesetImage = mWindow->LoadImage(image->first_attribute("source")->value());
			if (tilesetImage == NULL)
				throw std::runtime_error("Failed to load image\n");

			//Obtains the first global ID.
			//Remember that width and height correspond to the number of tiles, not pixels.
			int fgid = atoi(mapInfo->first_attribute("firstgid")->value());
			int width = atoi(image->first_attribute("width")->value());
			int height = atoi(image->first_attribute("height")->value());
			int tileWidth = atoi(mapInfo->first_attribute("tilewidth")->value());
			int tileHeight = atoi(mapInfo->first_attribute("tileheight")->value());
			int alpha = atoi(image->first_attribute("trans")->value());

			Tileset* newTileset = new Tileset(tilesetImage, fgid, width, height, tileWidth, tileHeight, alpha);

			//pushes texture pointer onto tileset vector in mLevel
			mLevel->getTileSet()->insert(newTileset);

			//Searches the tile properties to find which tiles are solid.
			for (rapidxml::xml_node<> *tile = mapInfo->first_node("tile"); tile != NULL; tile = tile->next_sibling("tile"))
			{

				std::string solidness = tile->first_node("properties")->first_node("property")->first_attribute("name")->value();
				bool solidProperty = solidness == "solid";
				bool isSolid = atoi(tile->first_node("properties")->first_node("property")->first_attribute("value")->value());

				//Remember to add the first global id to the relative Tile id.
				if (solidProperty && isSolid)
					mLevel->getSolidGid()->insert(atoi(tile->first_attribute("id")->value()) + fgid);
			}
		}


		//layers
		else if (nodeName == "layer")
		{
			printf("Here is a layer.\n");

			//gets base64 encoded data and removes whitespaces from data
			std::string encodedData = mapInfo->first_node()->value();
			encodedData.erase(std::remove_if(encodedData.begin(), encodedData.end(), isspace), encodedData.end());

			//decodes data
			std::string tileMapData = base64_decode(encodedData);

			//erases null terminating character
			tileMapData.erase(tileMapData.end() - 1, tileMapData.end());

			//union to convert decoded chars to tile id ints
			union codeData
			{
				int i;
				char c[4];
			} gid;

			//converting data into global Ids for tiles (gid)
			//decoded data is little endian byte order
			//tiles are read in row by row to mLevel tilemap array
			for (int tileNum = 0; tileNum < mLevel->getWidth() * mLevel->getHeight(); tileNum++)
			{
				gid.c[3] = tileMapData[4 * tileNum + 3];
				gid.c[2] = tileMapData[4 * tileNum + 2];
				gid.c[1] = tileMapData[4 * tileNum + 1];
				gid.c[0] = tileMapData[4 * tileNum];

				int heightIndex = tileNum / mLevel->getWidth();
				int widthIndex = tileNum % mLevel->getWidth();

				std::string layerName = mapInfo->first_attribute("name")->value();
				if (layerName == "background")
					mLevel->getTileMap()[widthIndex][heightIndex] = gid.i;
				else if (layerName == "background2")
					mLevel->getTileMap2()[widthIndex][heightIndex] = gid.i;
				else if (layerName == "overlayer")
					mLevel->getOverMap()[widthIndex][heightIndex] = gid.i;
				else if (layerName == "overlayer2")
					mLevel->getOverMap2()[widthIndex][heightIndex] = gid.i;
			}
		}


		//object groups
		else if (nodeName == "objectgroup")
		{
			printf("Here is a object group.\n");
			std::string objectGroupName = mapInfo->first_attribute("name")->value();
			if (objectGroupName == "playerSpawn")
			{
				rapidxml::xml_node<> *playerSpawn = mapInfo->first_node("object");
				for (int n = 0; n < mNextPlayerSpawn; n++)
				{
					playerSpawn->next_sibling("object");
				}

				mPlayerSpawnPoint.x = atoi(playerSpawn->first_attribute("x")->value());
				mPlayerSpawnPoint.y = atoi(playerSpawn->first_attribute("y")->value());



				/*
				NOTE: THIS SPAWN DATA IS SPECIFIC TO THE TEST SPRITE!!!!!!!!!!!!!
				NOTE: THIS SPAWN DATA IS SPECIFIC TO THE TEST SPRITE!!!!!!!!!!!!!
				NOTE: THIS SPAWN DATA IS SPECIFIC TO THE TEST SPRITE!!!!!!!!!!!!!
				NOTE: THIS SPAWN DATA IS SPECIFIC TO THE TEST SPRITE!!!!!!!!!!!!!
				NOTE: THIS SPAWN DATA IS SPECIFIC TO THE TEST SPRITE!!!!!!!!!!!!!
				NOTE: THIS SPAWN DATA IS SPECIFIC TO THE TEST SPRITE!!!!!!!!!!!!!
				NOTE: THIS SPAWN DATA IS SPECIFIC TO THE TEST SPRITE!!!!!!!!!!!!!
				*/


				SDL_Rect colBoxDude = { 0, 27, 26, 26 };


				spawnPlayer(mCharSprites->image, &getCharClip(INDEX_PLAYER), &colBoxDude, 0, 27, mPlayerSpawnPoint.x, mPlayerSpawnPoint.y);
			}

			if (objectGroupName == "playerExit")
			{
			}
		}

		//image layers
		else if (nodeName == "imagelayer")
		{
			printf("Here is an image layer.\n");
		}

		//properties
		else if (nodeName == "properties")
		{
			printf("Here is a properties layer.\n");
			/*
			for (rapidxml::xml_node<> *properties = mapInfo->first_node(); properties != NULL; properties = properties->next_sibling())
			{
				std::string propertyName = properties->first_attribute("name")->value();
				std::string propertyValue = properties->first_attribute("value")->value();

				if (propertyName == "parallaxBg" && propertyValue != "")
					parallaxBg = mWindow->LoadImage(propertyValue);
			}*/
		}

		else
			;
	}


	mapFile.close();
	mapData.clear();
}