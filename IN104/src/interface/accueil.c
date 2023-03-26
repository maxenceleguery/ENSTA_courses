

#include<dirent.h>
#include<stdio.h>
#include<stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define HEIGHT_ACCUEIL 800
#define WIDTH_ACCUEIL 1200

int initSDLAccueil(SDL_Window **window, SDL_Renderer **renderer) {
    /* Initialisation, création de la fenêtre et du renderer. */
    
    *window = SDL_CreateWindow("Transcription des notes de piano - Accueil", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WIDTH_ACCUEIL, HEIGHT_ACCUEIL, SDL_WINDOW_SHOWN);
    if(NULL == *window) {
        fprintf(stderr, "Erreur SDL_CreateWindow : %s", SDL_GetError());
        return 1;
    }
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_SOFTWARE);
    if(NULL == *renderer) {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s", SDL_GetError());
        return 1;
    }
    return 0;
}


void initAccueil(char** audioname) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
	
    int r = initSDLAccueil(&window, &renderer);
    if(r==1) {
        printf("Erreur lors de l'initialisation de l'accueil");
    }
    //Chargement des fontes pour l'écriture de texte
    TTF_Init();

    TTF_Font *font = TTF_OpenFont("fonts/LiberationMono-Regular.ttf", 24);
    if (font == NULL) {
        fprintf(stderr, "error: font not found\n");
        exit(EXIT_FAILURE);
    }

    //Lecture du dossier contenant les audios, puis affichage
    struct dirent *dir;
    DIR *d = opendir("audios/");

    char** list_audios = malloc(sizeof(char*));
    if (list_audios == NULL) {
        printf("Problème allocation mémoire : list_audios");
        free(list_audios);
        return;
    }
    SDL_Rect* list_audios_box = malloc(sizeof(SDL_Rect));
    if (list_audios_box == NULL) {
        printf("Problème allocation mémoire : list_audios_box");
        free(list_audios_box);
        return;
    }
    int nb_audios=0;

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if(dir->d_name[0]!='.') {
                //printf("%s\n", dir->d_name);
                nb_audios++;
                list_audios=realloc(list_audios, nb_audios*sizeof(char*));
                list_audios[nb_audios-1]=dir->d_name;

                SDL_SetRenderDrawColor(renderer, 255,255,255,0);   
                SDL_Rect Message_rect_contour = {8,10+(nb_audios-1)*60,30*strlen(dir->d_name)+4,54};

                SDL_RenderDrawRect(renderer, &Message_rect_contour);

                list_audios_box=realloc(list_audios_box, nb_audios*sizeof(SDL_Rect));
                list_audios_box[nb_audios-1]=Message_rect_contour;

                SDL_Color text_color = {127,127,127};
                SDL_Surface* surface = TTF_RenderText_Solid(font, dir->d_name, text_color);  
                SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_Rect Message_rect = {10,8+(nb_audios-1)*60,30*strlen(dir->d_name),50};

                SDL_RenderCopy(renderer, texture, NULL, &Message_rect);

                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }
        }
        closedir(d);
    }
    
    SDL_RenderPresent(renderer);


    //Gestion des évènements utilisateur, interaction avec les boutons des audios
    SDL_Event event;
    SDL_bool quit = SDL_FALSE;
    

    while(!quit) {
        SDL_WaitEvent(&event);
        if(event.type == SDL_QUIT)
            quit = SDL_TRUE;
        else if(event.type == SDL_MOUSEBUTTONUP) {
            if(event.button.button == SDL_BUTTON_LEFT) {
                SDL_Point p = {event.button.x,event.button.y};
                for(int i = 0; i<nb_audios;i++) {
                    if(SDL_PointInRect(&p,&(list_audios_box[i]))) {
                        *audioname=list_audios[i];
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        free(list_audios);
                        free(list_audios_box);
                        return;
                    }
                }
           }
        }
        SDL_Delay(20);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    free(list_audios);
    free(list_audios_box);
    *audioname=".";
    return;
}
