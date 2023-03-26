#ifndef __CLAVIER_H__
#define __CLAVIER_H__
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

void setWindowColor(SDL_Renderer *renderer, SDL_Color color);
void setBackgroundColor(SDL_Renderer *renderer, SDL_Color color);
void setTouchColor(SDL_Renderer *renderer, SDL_Rect* touches, int n, SDL_Color color);
void resetTouchColor(SDL_Renderer *renderer, SDL_Rect* touches, int n);
void resetKeyboard(SDL_Renderer *renderer, SDL_Rect* touches);
void testSetTouchColor(SDL_Renderer *renderer, SDL_Rect* touches);
SDL_Rect* initKeyboard(SDL_Renderer *renderer, SDL_Color colorTouch, SDL_Rect *touches);
int initSDLClavier(SDL_Window **window, SDL_Renderer **renderer);

#endif
