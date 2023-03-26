
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getFilePathWithName(char* audioname) {
    char* audiopath = malloc(sizeof(char*));
    audiopath="audios/";
    char * final_audio_name = (char *) malloc(1 + strlen(audioname)+ strlen(audiopath) );
    strcpy(final_audio_name, audiopath);
    strcat(final_audio_name, audioname);
    return final_audio_name;
}