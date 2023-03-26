/* Maxence Leguéry & Sam Gadoin */

#include <stdio.h>
#include <stdlib.h>

#include "interface/accueil.h"
#include "interface/clavier.h"
#include "interface/convert_frequence.h"
#include "interface/audio.h"
#include "analyse/getNotes.h"
#include "analyse/getAudioFile.h"

#define fichierTampon "notes.dat"

int main(int argc, char *argv[]) {
    if(0 != SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "Erreur SDL_Init : %s", SDL_GetError());
        return 1;
    }

    char* audioname = malloc(sizeof(char*));
    initAccueil(&audioname);
    
    if(!strcmp(audioname,".") || audioname==NULL) {
          goto Quit2;
    }

    audioname=getFilePathWithName(audioname);

    writeNotesInFile(audioname,fichierTampon);

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    int r = initSDLClavier(&window, &renderer);
    
    if(r==1) {
        printf("Erreur lors de l'initialisation de la librairie SDL");
        goto Quit;
    }
    
    //Dessin du clavier de piano; il y a 52 touches blanches
    SDL_Color noir = {0, 0, 0, 0};
    SDL_Rect *touches = malloc(88 * sizeof(SDL_Rect));
    touches = initKeyboard(renderer, noir, touches);

    playMusic(renderer, touches, fichierTampon, audioname);

    free(touches);
    
    remove(fichierTampon);

Quit:
    if(NULL != renderer)
        SDL_DestroyRenderer(renderer);
    if(NULL != window)
        SDL_DestroyWindow(window);
Quit2:
    SDL_Quit();
    return 0;
}
