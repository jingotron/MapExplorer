A TMX-based map explorer that uses Tiled maps and SDL 2.0.

The window.cpp and timer.cpp files are modified versions of code from Twinklebear's tutorials.

Custom assets have to be added in. The top of the main.cpp file will have constant string values that have all the filenames.

The TMX map has to be a certain format:

//------------------------------------------FORMAT---------------------------------------------------------------
	//- tiles that are solid will have property name = "solid" and either value "0" or "1" indicating soldness.
	//- one image per tileset.
	//- first tileset is the background tileset.
	//- an object layer "playerSpawn" with a rectangle indicating spawn location.
	//- to make player not go off edges of map, add invisible solid tiles around border.
	//- For parallax background: In map properties, add the property "parallaxBg" and the image file name as its value.

	//Layer format(these are all tile layers)

	//over layer 2
	//over layer, with non-solid tiles indicating things above actors
	//actor layer
	//background layer 2, with solid/passable tiles for objects with transparency
	//background layer 1, with solid/passable tiles

	//Character sprite sheets are stored in a single image. This is similar to sprites used in RPGMaker XP/VX/ACE, with additional diagonal directions.
	//		  step1    stand    step2
	//down
	//left
	//right
	//up
	//topleft
	//botleft
	//topright
	//botright
