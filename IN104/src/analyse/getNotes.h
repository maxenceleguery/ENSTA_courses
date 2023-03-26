#ifndef __GETNOTES_H__
#define __GETNOTES_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

struct listeNotes {
    double *note;
    unsigned int tailleNote;
    struct listeNotes *first;
    struct listeNotes *next;
};

double sigmoidPerso(double x, double c1, double c2, double c3);
struct listeNotes *addNotes(double *elem, unsigned int tailleNote, struct listeNotes *lst);
struct listeNotes* getNotes(char* fichieraudio, unsigned int* frequenceData);
void freeListe(struct listeNotes* listeFinal);
void writeNotesInFile(char* src,char* dst);

#endif