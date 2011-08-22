// File threads.cpp
// Copyright 2011 Daniel Ozick



// Include
#include <WProgram.h>
#include <avr/pgmspace.h>
#include "ThreadKit.h"

#include "iodefs.h"
#include "timers.h"
#include "songs.h"



// Constants

// declare user events
EVENTS (
        VOICE_1_START,
        VOICE_2_START
        );



/*
  midi_frequencies

  Frequencies of MIDI note numbers from 0 to 108.
  Created programmatically using equal temperament formula.

  A0 ==> 21 ==> lowest note on piano
  C4 ==> 60 ==> middle C
  A4 ==> 69 ==> concert A
  C8 ==> 108 ==> highest note on piano

  Note: The extern declaration of a PROGMEM table is required to
  defeat C++ warnings.  See "C++ warning with PROGMEM" at
  http://www.avrfreaks.net.
*/

extern const uint16_t midi_frequencies [] PROGMEM;
const uint16_t midi_frequencies [] =
 {
  8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 19, 21, 22, 23, 24,
  26, 28, 29, 31, 33, 35, 37, 39, 41, 44, 46, 49, 52, 55, 58, 62, 65, 69, 73,
  78, 82, 87, 92, 98, 104, 110, 117, 123, 131, 139, 147, 156, 165, 175, 185,
  196, 208, 220, 233, 247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440,
  466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988, 1047,
  1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976, 2093, 2217,
  2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951, 4186
 };



// Variables

boolean voice_2_done;

/*
  The following are intended to be U.I. variables controllable through
  a console.
*/
boolean print_scheduler_info = false;
int song_delay = 5000;
int tempo_factor = 240;
int note_gap = 50;



// Functions

// user_setup -- called before any threads start
void user_setup ()
{
 // setup I/O
 pinMode (LED_0, OUTPUT);
 pinMode (LED_1, OUTPUT);

 Serial.println ("user_setup: finished");
}



// midi_frequency -- retrieve frequency of midi note from PROGMEM table
uint16_t midi_frequency (uint8_t midi_number)
{
 return (pgm_read_word_near (midi_frequencies + midi_number));
}



// Threads

THREAD (blink)
{
 BEGIN_THREAD (STARTUP);

 while (true)
  {
   digitalWrite (LED_0, HIGH);
   WAIT_DELAY (100);

   digitalWrite (LED_0, LOW);
   WAIT_DELAY (100);
  }

 END_THREAD;
}



THREAD (scheduler_monitor)
{
 BEGIN_THREAD (STARTUP);

 while (true)
  {
   WAIT (print_scheduler_info);
   print_scheduler_info = false;

   printf ("scheduler_idle_count: %d \n", scheduler_idle_count);
   printf ("scheduler_used_time: %d \n", scheduler_used_time);
  }  

 END_THREAD;
}



THREAD (song_player_1)
{
 BEGIN_THREAD (VOICE_1_START);

 static int i;
 
 for (i = 0; true; i++)
  {
   int pitch = song_joan [i] . pitch;
   int duration = song_joan [i] . duration;

   // if end of song
   if (pitch == 0)
    break;

   // start second part of round
   if (i == 5)
    broadcast (VOICE_2_START);

   // play note and gap
   timer_1_tone (midi_frequency (pitch));
   WAIT_DELAY ((tempo_factor * duration) - note_gap);
   timer_1_tone (0);
   WAIT_DELAY (note_gap);
  }

 END_THREAD;
}



THREAD (song_player_2)
{
 BEGIN_THREAD (VOICE_2_START);

 static int i;
 
 for (i = 0; true; i++)
  {
   int pitch = song_joan [i] . pitch;
   int duration = song_joan [i] . duration;

   // if end of song
   if (pitch == 0)
    break;

   // play note and gap
   timer_2_tone (midi_frequency (pitch));
   WAIT_DELAY ((tempo_factor * duration) - note_gap);
   timer_2_tone (0);
   WAIT_DELAY (note_gap);
  }

 PULSE (voice_2_done);

 END_THREAD;
}



THREAD (tone_tester)
{
 BEGIN_THREAD (NEVER);

 static int i;

 // From A0 (lowest piano key) to C8 (highest piano key)
 for (i = 21; i <= 108; i++)
  {
   Serial.println (midi_frequency (i));
   timer_1_tone (midi_frequency (i));
   timer_2_tone (midi_frequency (i));
   WAIT_DELAY (200);
   timer_1_tone (0);
   timer_2_tone (0);
   WAIT_DELAY (50);
  }

 END_THREAD;
}



THREAD (song_starter)
{
 BEGIN_THREAD (STARTUP);

 while (true)
  {
   WAIT_DELAY (song_delay);
   broadcast (VOICE_1_START);
   WAIT (voice_2_done);
  }

 END_THREAD;
}



THREAD (yield_tester)
{
 BEGIN_THREAD (STARTUP);

 static int i;

 while (true)
  {
   i = 1000;
   while (i--)
    {
     YIELD ();
     // spin wait for 1 ms to show what happens to scheduler idle time
     delay (1);
    }
   WAIT_DELAY (1000);
  }

 END_THREAD;
}



THREADS (
         blink,

         scheduler_monitor,

         song_player_1,
         song_player_2,

         tone_tester,
         yield_tester,

         song_starter
         );



