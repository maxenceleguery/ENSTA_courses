#ifndef __GETFREQ_H__
#define __GETFREQ_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <gsl/gsl_fft.h>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_errno.h>

struct wavfile //definit la structure de l entete d un wave
{
    char        id[4];          // doit contenir "RIFF"
    int         totallength;        // taille totale du fichier moins 8 bytes
    char        wavefmt[8];     // doit etre "WAVEfmt "
    int         format;             // 16 pour le format PCM
    short       pcm;              // 1 for PCM format
    short       channels;         // nombre de channels
    int         frequency;          // frequence d echantillonage
    int         bytes_per_second;   // nombre de bytes par secondes
    short       bytes_by_capture;
    short       bits_per_sample;  // nombre de bytes par echantillon
    char        data[4];        // doit contenir "data"
    int         bytes_in_data;      // nombre de bytes de la partie data
};

double* getData(char* fichieraudio, unsigned long* sizeData, unsigned int* frequenceData);
int getFreq2(double* data,unsigned int frequency,unsigned int tailleNote);
double moyenne(double* data,int sizeFFT);
int* getFreq(double* data,unsigned int frequency,unsigned int tailleNote, unsigned int* nbFreq);


#endif