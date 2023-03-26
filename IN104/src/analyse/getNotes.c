/* Maxence Leguéry & Sam Gadoin */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "getNotes.h"
#include "getFreq.h"

double sigmoidPerso(double x, double c1, double c2, double c3) {
    if (x<c3) {
        return 1/(1+exp(-c1*(x-c2)));
    }
    else {
        return 1/2;
    }
}

struct listeNotes *addNotes(double *elem, unsigned int tailleNote, struct listeNotes *lst) {
    struct listeNotes* tmp = malloc(sizeof(struct listeNotes));
    if (tmp == NULL) {
        printf("Problème allocation mémoire");
        free(tmp);
        return NULL;
    }
    tmp->note = elem;
    tmp->tailleNote = tailleNote;
    tmp->first = lst->first;
    tmp->next = NULL;
    lst->next = tmp;
    //printf("Note ajoutée\n");
    return tmp;
}

struct listeNotes* getNotes(char* fichieraudio, unsigned int* frequenceData) {
    
    unsigned long taille;
    unsigned int frequency;
    double* data = getData(fichieraudio, &taille, &frequency);

    double* dataCorrel = malloc(taille*sizeof(double));
    if (dataCorrel == NULL) {
        printf("Problème allocation mémoire : dataCorrel");
        free(dataCorrel);
        return NULL;
    }

    for (int i=2;i<taille;i+=2) {
        dataCorrel[i]=abs(abs(data[i])-abs(data[i-2]));
    }

    //Test filtrage
    /*float CUTOFF = 10;
    float RC = 1.0/(CUTOFF*2*M_PI);
    float dt = 1.0/taille;
    float alpha = RC/(RC + dt);
    double* dataCorrelFiltered = malloc(taille*sizeof(double));
    dataCorrelFiltered[0] = dataCorrel[0];
    for (int i = 1; i<taille; i++){
        dataCorrelFiltered[i] = alpha * (dataCorrelFiltered[i-1] + dataCorrel[i] - dataCorrel[i-1]);
    }
    free(dataCorrel);
    dataCorrel = dataCorrelFiltered;*/

    double dataCorrelMax = 0;
    for (int i=0;i<taille;i++) {
        if (dataCorrelMax<dataCorrel[i]) {
            dataCorrelMax=dataCorrel[i];
        }
    }
    //printf("%f\n",dataCorrelMax);

    double c1 = 0.01;
    double c2 = dataCorrelMax/3;
    for (int i = 1; i<taille; i++){
        dataCorrel[i] = dataCorrel[i]*sigmoidPerso(dataCorrel[i],c1,c2,dataCorrelMax/1.5);
    }

    //Ecriture des données dans un fichier pour visualisation avec Python (plot.py)
    FILE *dat=fopen("dataCorrel.dat","w");
    if ( dat == NULL ) {
        printf("\nProblème lors de l'ouverture du fichier\n");
        fclose(dat);
        exit(0);
    }
    for (int i=2; i<taille; i+=2) {
        fprintf(dat,"%lf\n", dataCorrel[i]);
    }
    fclose(dat);

    dataCorrelMax = 0;
    for (int i=0;i<taille;i++) {
        if (dataCorrelMax<dataCorrel[i]) {
            dataCorrelMax=dataCorrel[i];
        }
    }

    double* note = malloc(taille*sizeof(double));
    if (note == NULL) {
        printf("Problème allocation mémoire : note");
        free(note);
        return NULL;
    }
    for (int j=0;j<taille;j++) { note[j]=0; }
    struct listeNotes* liste = malloc(sizeof(struct listeNotes));
    int dataProcessed=0;

    //Décomposition du signal en accords individuels

    for(int i=0;i<taille;i++) {
        note[i-dataProcessed]=data[i];
        if (data[i]!=0 && i-dataProcessed > 12000) {
            if (dataCorrel[i]>dataCorrelMax*0.100) {
                //printf("Nouvelle note %d\n", i);
                liste=addNotes(note,i-dataProcessed,liste);
                if (dataProcessed==0) {
                    liste->first=liste;
                }
                note = malloc(taille*sizeof(double));
                if (note == NULL) {
                    printf("Problème allocation mémoire : note");
                    free(note);
                    return NULL;
                }
                for (int j=0;j<taille;j++) { note[j]=0; }
                dataProcessed=i;
            }
        }
    }
    liste=addNotes(note,taille-dataProcessed,liste);
    
    if (dataProcessed==0) {
        liste->first=liste;
    }

    free(data);
    free(dataCorrel);
    //printf("All good\n");
    *frequenceData=frequency;
    return liste->first;
}

void freeListe(struct listeNotes* listeFinal) {
    //printf("Début libération mémoire\n");
    listeFinal=listeFinal->first;
    while (listeFinal != NULL) {
        free(listeFinal->note);
        struct listeNotes* tmp = listeFinal->next;
        free(listeFinal);
        listeFinal=tmp;
    }
    //printf("Fin libération mémoire\n");
}

void writeNotesInFile(char* src,char* dst) {
    unsigned int frequencyData;
    unsigned int nbFreq;
    FILE *dat=fopen(dst,"w");
    if ( dat == NULL ) {
        printf("\nProblème lors de l'ouverture du fichier\n");
        fclose(dat);
        exit(0);
    }
    struct listeNotes* listeFinal = getNotes(src,&frequencyData);
    struct listeNotes* listeFirst = listeFinal->first;
    int N=0;
    while (listeFinal != NULL) {
        N++;
        listeFinal=listeFinal->next;
    }
    //fprintf(dat,"%d\n",N);
    listeFinal=listeFirst;
    //printf("BEGIN ANALYSIS\n---\n");
    int c = 1;
    while (listeFinal != NULL) {
        //printf("Analyse note %d\n",c);
        int* freq = getFreq(listeFinal->note,frequencyData,listeFinal->tailleNote,&nbFreq);
        int time = 1000*listeFinal->tailleNote/(2.0*frequencyData);
        fprintf(dat,"%d ",time);
        for (int k=0;k<nbFreq;k++) {
            //int n = getPianoKeyNumber(freq[k]);
            //printf("Fréquence : %d Hz, %d, %d ms\n",freq[k], n, time);
            fprintf(dat,"%d ",freq[k]);
        }
        fprintf(dat,"S\n");
        listeFinal = listeFinal->next;
        //printf("---\n");
        c++;
    }
    freeListe(listeFirst);
    fclose(dat);
}