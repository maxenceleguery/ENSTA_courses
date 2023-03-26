#ifndef __ACCUEIL_H__
#define __ACCUEIL_H__

#include<dirent.h>
#include<stdio.h>
#include<stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

int initSDLAccueil(SDL_Window **window, SDL_Renderer **renderer);
char* initAccueil();

#endif
