#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

int loadAudio(char* filename, Uint8** pwavBuffer, SDL_AudioDeviceID* deviceId);
int freeAudio(SDL_AudioDeviceID deviceId, Uint8 *wavBuffer);
void playMusic(SDL_Renderer *renderer, SDL_Rect* touches, char* filename, char* audioname);

#endif
