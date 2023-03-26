/* Maxence Leguéry & Sam Gadoin */

#include <SDL2/SDL.h>
#include <stdio.h>

#include "clavier.h"

#define nb_touches 88
#define nb_touches_blanches 52
#define longueur_touche_blanche 150
#define longueur_touche_noire 100
#define HEIGHT 200
#define WIDTH 1600

int touches_type[nb_touches] = {0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0};
int largeur_touche_blanche;
int largeur_touche_noire;

SDL_Color blanc = {255, 255, 255, 0};   
SDL_Color noir = {0, 0, 0, 0};
SDL_Color gris = {127, 127, 127, 0};
SDL_Color rouge = {255, 0, 0, 0};

void setWindowColor(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void setBackgroundColor(SDL_Renderer *renderer, SDL_Color color) {
    setWindowColor(renderer, color);
    SDL_RenderFillRect(renderer, NULL);
}

void setTouchColor(SDL_Renderer *renderer, SDL_Rect* touches, int n, SDL_Color color) {
    SDL_Rect rect = touches[n];
    setWindowColor(renderer, color);

    if(touches_type[n]==1) {
        //Coloriage d'une touche noire
        rect.x +=1;
        rect.y +=1;
        rect.w -=2;
        rect.h -=2;
        SDL_RenderFillRect(renderer,&rect);
    } else {
        //Coloriage d'une touche blanche
        SDL_Rect rect_bas = {rect.x+1,rect.y+longueur_touche_noire,rect.w-2, longueur_touche_blanche-longueur_touche_noire-1};
        SDL_RenderFillRect(renderer,&rect_bas);
        SDL_Rect rect_haut_milieu = {rect.x+largeur_touche_noire/2,rect.y+1,largeur_touche_blanche-largeur_touche_noire, longueur_touche_noire-1};
        SDL_RenderFillRect(renderer,&rect_haut_milieu);


        if(n==0 || touches_type[n-1]==0) {
            //coloriage coin haut gauche
            SDL_Rect rect_haut_gauche = {rect.x+1,rect.y+1,largeur_touche_noire/2, longueur_touche_noire};
            SDL_RenderFillRect(renderer,&rect_haut_gauche);
        }

        if(n<nb_touches-1 && touches_type[n+1]==0) {
            //coloriage coin haut droit
            SDL_Rect rect_haut_droit = {rect.x+largeur_touche_blanche-largeur_touche_noire/2-1,rect.y+1,largeur_touche_noire/2, longueur_touche_noire};
            SDL_RenderFillRect(renderer,&rect_haut_droit);
        }
    }
    SDL_RenderPresent(renderer);//Refresh de l'interface graphique
}

void resetTouchColor(SDL_Renderer *renderer, SDL_Rect* touches, int n) {
    if(touches_type[n] == 0) {
        setTouchColor(renderer, touches, n, blanc);
    } else {
        setTouchColor(renderer, touches, n, noir);
    }
}

void resetKeyboard(SDL_Renderer *renderer, SDL_Rect* touches) {
    for(int i = 0; i<nb_touches; i++) {
        resetTouchColor(renderer, touches, i);
    }
}

void testSetTouchColor(SDL_Renderer *renderer, SDL_Rect* touches) {
    SDL_Delay(100);
    for (int n = 0; n < nb_touches; n++) {
        if(n>0) {
            resetTouchColor(renderer, touches, n-1);
        }
        setTouchColor(renderer, touches, n, rouge);
        SDL_Delay(100);
    }
    resetTouchColor(renderer, touches, nb_touches-1);
}

SDL_Rect* initKeyboard(SDL_Renderer *renderer, SDL_Color colorTouch, SDL_Rect *touches) {
    
    //Définition de la couleur du fond
    setBackgroundColor(renderer, gris);

    largeur_touche_blanche = WIDTH/(nb_touches_blanches-1);
    largeur_touche_noire = round(largeur_touche_blanche*0.5);
    
    setWindowColor(renderer, blanc);

    SDL_Rect fond = {0, HEIGHT-longueur_touche_blanche,WIDTH,longueur_touche_blanche};
    SDL_RenderFillRect(renderer, &fond);

    
    //Coloriage des touches blanches
    int current_x = 0;
    setWindowColor(renderer, colorTouch);

    for(int i = 0; i<nb_touches; i++) {
        if(touches_type[i]==0) {
            SDL_Rect touche = {current_x,HEIGHT-longueur_touche_blanche,largeur_touche_blanche,longueur_touche_blanche};
            touches[i]=touche;
            SDL_RenderDrawRect(renderer, &touche);
            current_x+=largeur_touche_blanche;
        }
    }

    //Coloriage des touches noires
    current_x = 0;

    for(int i = 0; i<nb_touches; i++) {
        if(touches_type[i]==0) {
            current_x+=largeur_touche_blanche;
        } else {
            current_x-=largeur_touche_noire/2;  
            SDL_Rect touche = {current_x,HEIGHT-longueur_touche_blanche,largeur_touche_noire,longueur_touche_noire};
            touches[i]=touche;
            SDL_RenderFillRect(renderer, &touche);
            
            current_x+=largeur_touche_noire/2;
        }
    }

    SDL_RenderPresent(renderer);
    return touches;
}

int initSDLClavier(SDL_Window **window, SDL_Renderer **renderer) {
    *window = SDL_CreateWindow("Transcription des notes de piano", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
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

