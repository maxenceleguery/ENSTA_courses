/* Maxence Leguéry & Sam Gadoin */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <gsl/gsl_fft.h>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_errno.h>
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

#define REAL(z,i) ((z)[2*(i)])
#define IMAG(z,i) ((z)[2*(i)+1])

#include "getFreq.h"

double* getData(char* fichieraudio, unsigned long* sizeData, unsigned int* frequenceData) {

    unsigned long taille=1; //variable qui correspondra par la suite a la longueur du tableau (puissance de 2)
    int nbech=0; //nombre d echantillons extraits du fichier audio
    int puissance=0; //variable qui serta incrementee qui correspond a l indice de la puissance de 2 pour la taille du tableau

    //float tempstotal=0;

    printf("Nom du fichier : %s\n", fichieraudio);

    /*---------------------ouverture du wave----------------------------------------*/
    FILE *wav = fopen(fichieraudio,"rb"); //ouverture du fichier wave
    struct wavfile header; //creation du header

    if ( wav == NULL ) {
        printf("\nProblème lors de l'ouverture du fichier\n");
        exit(0);
    }
    /*---------------------fin de ouverture du wave---------------------------------*/

    /*---------------------lecture de l entete et enregistrement dans header--------*/
    //initialise le header par l'entete du fichier wave
    //verifie que le fichier possede un entete compatible
    if ( fread(&header,sizeof(header),1,wav) < 1 ) {
        printf("\nNe peut pas lire le header\n");
        exit(0);
    }
    if (header.id[0] != 'R'|| header.id[1] != 'I'|| header.id[2] != 'F'|| header.id[3] != 'F') {
        printf("\nErreur le fichier n'est pas un format wave valide\n");
        exit(0);
    }
    if (header.channels!=1) {
        printf("\nErreur : le fichier n'est pas mono\n");
        exit(0);
    }
    /*----------------fin de lecture de l entete et enregistrement dans header-------*/



    /*-------------------------determiner la taille des tableaux---------------------*/
    nbech=(header.bytes_in_data/header.bytes_by_capture);
    //printf("\nLe fichier audio contient %d echantillons\n",nbech);
    while (nbech>taille) {
        taille=taille*2;
        puissance=puissance+1;
    }
    //printf("Nombre de points traites : 2^%d=%ld\n",puissance,taille);
    //tempstotal=(1./header.frequency)*taille;
    //printf("Temps total : %f s\n",tempstotal);
    taille=taille*2;
    /*------------------fin de determiner la taille des tableaux---------------------*/



    /*---------------------creation des tableaux dynamiques--------------------------*/
    double *data=NULL; //tableau de l'onde temporelle
    data=malloc((taille) * sizeof(double));
    if (data == NULL) {
        printf("Problème d'allocation");
        free(data);
        return 0;
    }
    /*---------------------fin de creation des tableaux dynamiques-------------------*/
    /*---------------------initialisation des tableaux dynamiques--------------------*/
    for(int i=0; i<taille; i++) {
        data[i]=0;
    }
    /*---------------------fin de initialisation des tableaux dynamiques-------------*/

    /*---------------------remplissage du tableau tab avec les echantillons----------*/
    int j=0;
    short value=0;
    while( fread(&value,(header.bits_per_sample)/8,1,wav) ) {
        //lecture des echantillons et enregistrement dans le tableau
        //data[2*j]=value*(0.54 - 0.56*cos(M_PI*j/taille));
        data[2*j]=value;
        j++;
    }

    *sizeData=taille;
    *frequenceData=header.frequency;
    return data;
}

// Ancienne fonction déterminant la fréquence de la note
int getFreq2(double* data,unsigned int frequency,unsigned int tailleNote) {

    int sizeFFT = 4096;
    //Fenetrage de Hamming
    for (int j=0;j<tailleNote;j++) {
        data[j]=data[j]*(0.54 - 0.56*cos(M_PI*j/tailleNote));
    }
    //Transformee de Fourier
    gsl_fft_complex_radix2_forward(data, 1, sizeFFT);

    //Recherche de la fréquence maximale
    int indiceMax = 0;
    double amp = pow(REAL(data,0),2) + pow(IMAG(data,0),2);
    for (int i=1; i<sizeFFT/2; i++) {
        if (amp<pow(REAL(data,i),2) + pow(IMAG(data,i),2)) {
            indiceMax=i;
            amp=pow(REAL(data,i),2) + pow(IMAG(data,i),2);
        }
    }
    int freq = indiceMax*frequency/sizeFFT;
    return freq;
}

double moyenne(double* data,int sizeFFT) {
    double moy = pow(REAL(data,0),2) + pow(IMAG(data,0),2);
    for (int i=1; i<sizeFFT/2; i++) {
        moy+=pow(REAL(data,i),2) + pow(IMAG(data,i),2);
    }
    return moy/=(sizeFFT/2);
}

// Nouvelle fonction déterminant les fréquences de l'accord
int* getFreq(double* data,unsigned int frequency,unsigned int tailleNote, unsigned int* nbFreq) {

    int sizeFFT = 4096;
    //Fenetrage de Hamming
    for (int j=0;j<tailleNote;j++) {
        data[j]=data[j]*(0.54 - 0.46*cos(2*M_PI*j/tailleNote));
    }
    //Transformee de Fourier
    gsl_fft_complex_radix2_forward(data, 1, sizeFFT);

    for (int j=0;j<tailleNote/5;j++) {
        data[j]=data[j]*2*exp(-j/100);
    }

    //Recherche des fréquences de l'accord
    int N = 3; // Nombre maximal de note dans l'accord
    int k=0;
    int* tabFreq = malloc(N*sizeof(int));
    if (tabFreq == NULL) {
        printf("Problème allocation mémoire : tableau fréquence");
        free(tabFreq);
        return NULL;
    }
    double moy = moyenne(data,sizeFFT);
    double moyPrev = moyenne(data,sizeFFT);
    double amp = 1;
    double ampOrigin = 1;
    while ((ampOrigin-amp)/ampOrigin<0.995 && moyPrev/moy<15 && k<N) {

        int indiceMax = 1;
        amp = pow(REAL(data,indiceMax),2) + pow(IMAG(data,indiceMax),2);
        for (int i=indiceMax; i<sizeFFT/2; i++) {
            if (amp<pow(REAL(data,i),2) + pow(IMAG(data,i),2)) {
                indiceMax=i;
                amp=pow(REAL(data,i),2) + pow(IMAG(data,i),2);
            }
        }

        if (k==0) { ampOrigin = amp; }

        tabFreq[k] = indiceMax*frequency/sizeFFT;
        //printf("%d %f\n",tabFreq[k], moyPrev/moy);

        int f = tabFreq[k];
        int n = 2;
        while (f<frequency) {
            int fg = f - (f/pow(2,1/12))/7;
            int fd = f*(1+pow(2,1/12)/7);
            int ig = sizeFFT*fg/frequency;
            int id = sizeFFT*fd/frequency;
            for (int j=ig;j<id+1;j++) {
                data[j]=0;
            }
            f=n*tabFreq[k];
            n++;
        }

        amp = pow(REAL(data,0),2) + pow(IMAG(data,0),2);
        for (int i=1; i<sizeFFT/2; i++) {
            if (amp<pow(REAL(data,i),2) + pow(IMAG(data,i),2)) {
                indiceMax=i;
                amp=pow(REAL(data,i),2) + pow(IMAG(data,i),2);
            }
        }
        moyPrev = moy;
        moy=moyenne(data,sizeFFT);
        k++;
    }
    *nbFreq = k;
    return tabFreq;
}