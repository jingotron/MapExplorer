#include <string>
#include <stdexcept>
#include <memory>

#if defined(_MSC_VER)
#include <SDL.h>
#undef main
#include <SDL_image.h>
#include <SDL_ttf.h>
#elif defined(__clang__)
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#endif

#include "window.h"

Window::Window()
	:mWindow(NULL), mRenderer(NULL)
{
}

void Window::Init(std::string title, int width, int height)
{
    //initialize all SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		throw std::runtime_error("SDL Init Failed");
    if (TTF_Init() == -1)
		throw std::runtime_error("TTF Init Failed");

    //Setup our window size
    mBox.x = 0;
    mBox.y = 0;
    mBox.w = width;
    mBox.h = height;

    //Create our window
    mWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        mBox.w, mBox.h, /*SDL_WINDOW_FULLSCREEN*/SDL_WINDOW_SHOWN);
    if (mWindow == NULL)
        throw std::runtime_error("Failed to create window");

    //Create the renderer
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (mRenderer == NULL)
        throw std::runtime_error("Failed to create renderer");

	mColor = LoadImage("green.png");
}

void Window::Quit()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
    TTF_Quit();
    SDL_Quit();
}

Window::~Window()
{
	Window::Quit();
}
void Window::Draw(SDL_Texture *tex, SDL_Rect &dstRect, SDL_Rect *clip, float angle, 
                  int xPivot, int yPivot, SDL_RendererFlip flip)
{
    //Convert pivot pos from relative to object's top-left corner to be relative to its center
    xPivot += dstRect.w / 2;
    yPivot += dstRect.h / 2;
    //SDL expects an SDL_Point as the pivot location
    SDL_Point pivot = { xPivot, yPivot };
    //Draw the texture
    SDL_RenderCopyEx(mRenderer, tex, clip, &dstRect, angle, &pivot, flip);
}

void Window::Draw(SDL_Texture *tex, int x, int y, SDL_Rect *clip, float angle, 
                  int xPivot, int yPivot, SDL_RendererFlip flip)
{
	SDL_Rect dstRect;
	dstRect.x = x;
	dstRect.y = y;

	if(clip == NULL)
		SDL_QueryTexture(tex, NULL, NULL, &dstRect.w, &dstRect.h);
	else
	{
		dstRect.w = clip->w;
		dstRect.h = clip->h;
	}

    //Convert pivot pos from relative to object's top-left corner to be relative to its center
    xPivot += dstRect.w / 2;
    yPivot += dstRect.h / 2;
    //SDL expects an SDL_Point as the pivot location
    SDL_Point pivot = { xPivot, yPivot };
    //Draw the texture

	//TEST CODE/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	dstRect.x *= SIZE_FACTOR;
	dstRect.y *= SIZE_FACTOR;
	dstRect.w *= SIZE_FACTOR;
	dstRect.h *= SIZE_FACTOR;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    SDL_RenderCopyEx(mRenderer, tex, clip, &dstRect, angle, &pivot, flip);
}

SDL_Texture* Window::LoadImage(const std::string &file){
    SDL_Texture* tex = NULL;
    tex = IMG_LoadTexture(mRenderer, file.c_str());
    if (tex == NULL)
        throw std::runtime_error("Failed to load image: " + file + IMG_GetError());
    return tex;
}
SDL_Texture* Window::RenderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize){
    //Open the font
    TTF_Font *font = NULL;
    font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if (font == NULL)
        throw std::runtime_error("Failed to load font: " + fontFile + TTF_GetError());

    //Render the message to an SDL_Surface, as that's what TTF_RenderText_X returns
    SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(mRenderer, surf);
    //Clean up unneeded stuff
    SDL_FreeSurface(surf);
    TTF_CloseFont(font);

    return texture;
}
void Window::Clear(){
    SDL_RenderClear(mRenderer);
}
void Window::Present(){
    SDL_RenderPresent(mRenderer);
}
SDL_Rect Window::Box(){
    //Update mBox to match the current window size
    SDL_GetWindowSize(mWindow, &mBox.w, &mBox.h);
    return mBox;
}