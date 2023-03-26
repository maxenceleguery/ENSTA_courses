/* Maxence Leguéry & Sam Gadoin */

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "clavier.h"
#include "convert_frequence.h"

int loadAudio(char* filename, Uint8** pwavBuffer, SDL_AudioDeviceID* pdeviceId) {
    //Chargement d'un audio via SDL
    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;
 
    SDL_LoadWAV(filename, &wavSpec, &wavBuffer, &wavLength);
   
    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

    int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);

    *pwavBuffer = wavBuffer;
    *pdeviceId = deviceId;

    if (success == 1) {
        printf("Erreur lors de l'enqueue d'un fichier audio\n");
        return 1;
    }

    return 0;
}

void freeAudio(SDL_AudioDeviceID deviceId, Uint8 *wavBuffer) {
    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
}

void playMusic(SDL_Renderer *renderer, SDL_Rect* touches, char* filename, char* audioname) {
    SDL_Color rouge = {255, 0, 0, 0};
    FILE *morceau = fopen(filename,"rb"); //ouverture de la liste des notes
    if (morceau == NULL) {
        printf("Erreur lors de l'ouverture de la liste des notes %s",filename);
        fclose(morceau);
        return;
    }

    //Chargement des notes du morceau
    int nb_notes;
    fscanf(morceau, "%d", &nb_notes);

    Uint8 *wavBuffer;
    SDL_AudioDeviceID deviceId;
    loadAudio(audioname, &wavBuffer, &deviceId);

    SDL_PauseAudioDevice(deviceId, 0);
    
    //Lecture du morceau
    int i = 0;
    int duration;
    char freq[5];
    while(!feof(morceau)&& i<250) {
        fscanf(morceau, "%d",&duration);

        while(!feof(morceau) && strcmp(freq,"S") != 0) {
            fscanf(morceau, "%s", freq);
            if(strcmp(freq,"S")!=0) {
                int n = getPianoKeyNumber(atoi(freq));
                setTouchColor(renderer, touches, n, rouge);
            }
        }

        SDL_Delay(duration);
        resetKeyboard(renderer, touches);

        freq[0]='N';
        i++;
    }

    freeAudio(deviceId,wavBuffer);
    fclose(morceau);
}

