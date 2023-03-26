/* Maxence Leguéry & Sam Gadoin */

#include <math.h>
#include <stdio.h>

#define f0 27.5 //Définition de la fréquence de la plus basse touche (en Hz)

int getPianoKeyNumber(double freq) {
    int n = round(12.0 * (log2(freq*1.0/f0)));
    return n; 
}

//Fonction pour tester que toutes les touches de piano donnees par getPianoKeyNumber sont associées a la bonne frequence
void testGetPianoKeyNumber(char* file_name) {
    FILE *f = fopen(file_name, "rb");

    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier test %s",file_name);
        fclose(f);
        return;
    }

    float freq;
    fscanf(f, "%f", &freq);
    printf("%f\n", freq);
    int i = 0;

    while (!feof(f) && i < 250) {   
        fscanf(f, "%f", &freq);
        printf("f = %f",freq);
        int n=getPianoKeyNumber(freq);
        printf("; n = %d\n",n);
        i++;
    }
    fclose(f);
}
