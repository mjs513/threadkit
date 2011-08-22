/*
File ThreadKit.cpp

Copyright 2011 Daniel Ozick / Computing Explorations

MIT License

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



// Include
#include <WProgram.h>
#include "ThreadKit.h"



// Constants



// Variables

// stdio for printf
static FILE uartout = { 0 };

// threads
PThread threads [MAX_N_THREADS];

// internal state
int scheduler_idle_count;
int scheduler_used_time;



// Class Methods

PThread::PThread ()
{
 continue_addr = 0;
 state = PTS_SLEEPING;
 delay_var = 0;
 trigger_event = NEVER;
 is_triggered = false;
}



void PThread::reset ()
{
 continue_addr = 0;
 state = PTS_SLEEPING;
 delay_var = 0;
 is_triggered = false;
}



// Functions

int uart_putchar (char c, FILE *stream)
{
 Serial.write (c);
 return 0 ;
}



void scheduler (PT_State pt_state_filter)
{
 int i;

 for (i = 0; i < MAX_N_THREADS; i++)
  {
   THREAD_FUNC tf = thread_funcs [i];
   PPT pt = threads + i;

   if (tf == NULL)
    break;

   if ((pt_state_filter == PTS_ANY) ||
        (pt->state == pt_state_filter) )
    {
     // resume thread
     pt->state = PTS_RUNNING;
     (*tf) (pt);
    }
  }
}



void update_thread_delay_vars ()
{
 int i;

 for (i = 0; i < MAX_N_THREADS; i++)
  {
   THREAD_FUNC tf = thread_funcs [i];
   PPT pt = threads + i;

   if (tf == NULL)
    break;

   if (pt->delay_var > 0)
    pt->delay_var -= SCHEDULER_PERIOD;
  }
}



void broadcast (Event event)
{
 int i;

 for (i = 0; i < MAX_N_THREADS; i++)
  {
   THREAD_FUNC tf = thread_funcs [i];
   PPT pt = threads + i;

   if (tf == NULL)
    break;

   if (pt->trigger_event == event)
    {
     pt->is_triggered = true;
     pt->continue_addr = 0;
    }
  }
}



// Arduino init

void setup ()
{
  Serial.begin (SERIAL_RATE);

  // specify write function for UART file descriptor and assign stdout
  fdev_setup_stream (& uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = & uartout;

  printf ("setup: compiled on %s at %s \n", __DATE__, __TIME__);

  user_setup ();

  printf ("setup: finished \n");
}



// Arduino main loop

void loop ()
{
 static long time = 0;
 static boolean startup = true;

 update_thread_delay_vars ();
 scheduler (PTS_ANY);

 if (startup)
  {
   broadcast (STARTUP);
   startup = false;
  }
 
 scheduler_used_time = millis () - time;

 scheduler_idle_count = 0;
 while ((millis () - time) < SCHEDULER_PERIOD)
  {
   scheduler (PTS_YIELDING);
   scheduler_idle_count++;
  }

 time += SCHEDULER_PERIOD;
}



