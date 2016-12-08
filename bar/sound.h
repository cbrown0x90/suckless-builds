#ifndef SOUND_H
#define SOUND_H

typedef struct sound {
    long volume;
    int status;
    char percent[4];
} sound;

void soundInit(void);
void soundDestroy(void);
sound getMasterStatus(void);
char* volIcon(void);

#endif
