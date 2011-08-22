// File songs.h
// Copyright 2011 Daniel Ozick



#ifndef SONGS_H
#define SONGS_H



// Include
#include <WProgram.h>



// Constants



// Types

typedef struct
{
 uint8_t pitch;
 uint8_t duration;
} Note;



// Externs
extern Note song_joan [];



#endif   // SONGS_H



