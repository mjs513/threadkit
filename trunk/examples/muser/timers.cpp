// File timers.cpp
// Copyright 2011 Daniel Ozick



// Include
#include <WProgram.h>
#include "ThreadKit.h"
#include "timers.h"



// Constants




// Variables




// Functions

/*
  ATmega328P timer outputs pin assignments:

  OC1A --> PB1 --> Arduino 9
  OC1B --> PB2 --> Arduino 10

  OC2A --> PB3 --> Arduino 11
  OC2B --> PD3 --> Arduino 3
*/



/*
  Timer 1 (16 bits)

  With 16 MHz clock, lowest output frequency is 0.24 Hz (4.2 s period).
*/

void timer_1_tone (int freq)
{
 static const uint16_t divisors [] = { 0, 1, 8, 64, 256, 1024 };
 static const uint8_t n_divisors = sizeof (divisors) / sizeof (divisors [0]);
 uint8_t prescale = 0;
 uint32_t count = 0;

 // Find minimum prescale for freq
 if (freq > 0)
  for (prescale = 1; prescale < n_divisors; prescale++)
   {
    count = (F_CPU / divisors [prescale]) / (freq * 2);
    if (count <= ((uint32_t) 1 << 16))
     break;
   }

 // OC1A on PB1 (Arduino 9)
 pinMode (9, OUTPUT);

 // Toggle OC1A on compare match
 TCCR1A = _BV (COM1A0);

 // prescale select
 // CTC mode (clear timer on compare match)
 TCCR1B = prescale | _BV (WGM12);

 // No force output
 TCCR1C = 0;

 // Output compare value (top)
 OCR1A = count - 1;

 // No interrupts
 TIMSK1 = 0;
}



/*
  Timer 2 (8-bit counter)

  With 16 MHz clock, lowest output frequency is 30.5 Hz (MIDI 23, B0).
*/

void timer_2_tone (int freq)
{
 static const uint16_t divisors [] = { 0, 1, 8, 32, 64, 128, 256, 1024 };
 static const uint8_t n_divisors = sizeof (divisors) / sizeof (divisors [0]);
 uint8_t prescale = 0;
 uint32_t count = 0;

 // Find minimum prescale for freq
 if (freq > 0)
  for (prescale = 1; prescale < n_divisors; prescale++)
   {
    count = (F_CPU / divisors [prescale]) / (freq * 2);
    if (count <= (1 << 8))
     break;
   }

 // OC2A on PB3 (Arduino 11)
 pinMode (11, OUTPUT);

 // Toggle OC2A on compare match
 // CTC mode (clear timer on compare match)
 TCCR2A = _BV (COM2A0) | _BV (WGM21);

 // prescale select
 TCCR2B = prescale;

 // Output compare value (top)
 OCR2A = count - 1;

 // No interrupts
 TIMSK1 = 0;
}



