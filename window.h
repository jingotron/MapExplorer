#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <memory>

#if defined(_MSC_VER)
#include "SDL.h"
#undef main
#elif defined(__clang__)
#include <SDL2/SDL.h>
#else
#include <SDL2/SDL.h>
#endif


//This factor is considered for any drawing functions and camera movement. It will scale up everything drawn to the screen.
const int SIZE_FACTOR = 2;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

/**
*  Window management class, provides a simple wrapper around
*  the SDL_Window and SDL_Renderer functionalities
*/
class Window {
public:
	//Initialize SDL, setup the window and renderer
	//@param title The window title
	void Init(std::string title = "Window", int width = SCREEN_WIDTH, int height = SCREEN_HEIGHT);
	void Quit();

	Window();
	~Window();

    /**
    *  Draw a SDL_Texture to the screen at dstRect with various other options
    *  @param tex The SDL_Texture to draw
    *  @param dstRect The destination position and width/height to draw the texture with
    *  @param clip The clip to apply to the image, if desired
    *  @param angle The rotation angle to apply to the texture, default is 0
    *  @param xPivot The x coordinate of the pivot, relative to (0, 0) being center of dstRect
    *  @param yPivot The y coordinate of the pivot, relative to (0, 0) being center of dstRect
    *  @param flip The flip to apply to the image, default is none
    */
    void Draw(SDL_Texture *tex, SDL_Rect &dstRect, SDL_Rect *clip = NULL,
                     float angle = 0.0, int xPivot = 0, int yPivot = 0,
                     SDL_RendererFlip flip = SDL_FLIP_NONE);

	void Draw(SDL_Texture *tex, int x = 0, int y = 0, SDL_Rect *clip = NULL,
                     float angle = 0.0, int xPivot = 0, int yPivot = 0,
                     SDL_RendererFlip flip = SDL_FLIP_NONE);
    /**
    *  Loads an image directly to texture using SDL_image's
    *  built in function IMG_LoadTexture
    *  @param file The image file to load
    *  @return SDL_Texture* to the loaded texture
    */
    SDL_Texture* LoadImage(const std::string &file);
    /**
    *  Generate a texture containing the message we want to display
    *  @param message The message we want to display
    *  @param fontFile The font we want to use to render the text
    *  @param color The color we want the text to be
    *  @param fontSize The size we want the font to be
    *  @return An SDL_Texture* to the rendered message
    */
    SDL_Texture* RenderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize);
    ///Clear the renderer
    void Clear();
    ///Present the renderer, ie. update screen
    void Present();
    ///Get the window's box
    SDL_Rect Box();

	SDL_Window* getWindow() const {return mWindow;}
	SDL_Renderer* getRenderer() const {return mRenderer;}
	SDL_Rect getBox() const {return mBox;}
	SDL_Texture* getGreen() const {return mColor;}

private:
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    SDL_Rect mBox;
	SDL_Texture* mColor;
};

#endif