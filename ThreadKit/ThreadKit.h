/*
File ThreadKit.h

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



#ifndef THREADKIT_H
#define THREADKIT_H



// Include
#include <WProgram.h>
#include <stdio.h>



// Constants

// Serial
#define SERIAL_RATE 57600


// threads
#define SCHEDULER_FREQ 100
#define SCHEDULER_PERIOD (1000 / SCHEDULER_FREQ)

#define MAX_N_THREADS 16


// console
#define MAX_LINE 80



// Types

// threads and events
typedef enum
 {
  PTS_ANY,
  PTS_SLEEPING,
  PTS_RUNNING,
  PTS_YIELDING,
  PTS_WAITING
 } PT_State;



typedef enum 
 {
  NEVER = 0,
  STARTUP = 1
 } Built_In_Event;



typedef uint8_t Event;



class PThread
 {
  public:

  void *continue_addr;
  PT_State state;
  int16_t delay_var;

  Event trigger_event;
  boolean is_triggered;

  PThread ();
  void reset ();
 };



typedef PThread *PPT;
typedef void (*THREAD_FUNC) (PPT pt);



// Externs
extern THREAD_FUNC thread_funcs [MAX_N_THREADS];
extern int scheduler_idle_count;
extern int scheduler_used_time;



// Functions
void user_setup ();
void broadcast (Event event);



// Macros

// helpers for making labels based on line number

#define PASTE(a, b)                             \
 a ## b                                         \

#define PT_LABEL_1(n)                           \
 PASTE (__PT, n)                                \

#define PT_LABEL()                              \
 PT_LABEL_1 (__LINE__)                          \



// events

#define EVENTS(event_list...)                   \
 enum                                           \
  {                                             \
   USER_EVENT_0 = 128,                          \
   event_list                                   \
  }                                             \

   

// threads

#define THREAD(name)                            \
 void name (PPT __pt__)                         \



#define BEGIN_THREAD(event)                     \
  if (__pt__ -> continue_addr)                  \
   goto *(__pt__ -> continue_addr);             \
  __pt__ -> trigger_event = event;              \
  WAIT_UNLESS (__pt__ -> is_triggered);         \
  __pt__ -> is_triggered = false                \



#define END_THREAD                              \
  __pt__ -> continue_addr = && PT_LABEL();      \
  PT_LABEL():                                   \
  __pt__ -> state = PTS_SLEEPING;               \
  return                                        \



#define THREADS(thread_list...)                 \
  THREAD_FUNC thread_funcs [MAX_N_THREADS] =    \
    { thread_list, 0 }                          \



// waits and time

#define SET_DELAY(millis)                       \
  __pt__ -> delay_var = millis                  \



#define IS_DELAY_OVER()                         \
  ((__pt__ -> delay_var) <= 0)                  \



#define YIELD()                                 \
  do                                            \
   {                                            \
    __pt__ -> continue_addr = && PT_LABEL();    \
    __pt__ -> state = PTS_YIELDING;             \
    return;                                     \
    PT_LABEL():                                 \
    ;                                           \
   } while (0)                                  \



#define WAIT_TICK()                             \
  do                                            \
   {                                            \
    __pt__ -> continue_addr = && PT_LABEL();    \
    __pt__ -> state = PTS_WAITING;              \
    return;                                     \
    PT_LABEL():                                 \
    ;                                           \
   } while (0)                                  \



#define WAIT(condition)                         \
  do                                            \
   {                                            \
    WAIT_TICK ();                               \
   } while (! (condition))                      \



#define WAIT_UNLESS(condition)                  \
  do                                            \
   {                                            \
    while (! (condition))                       \
     WAIT_TICK ();                              \
   } while (0)                                  \



#define WAIT_DELAY(millis)                      \
  do                                            \
   {                                            \
    SET_DELAY (millis);                         \
    WAIT (IS_DELAY_OVER ());                    \
   } while (0)                                  \



#define WAIT_IMPATIENTLY(condition, timeout)    \
  do                                            \
   {                                            \
    SET_DELAY (timeout);                        \
    WAIT (IS_DELAY_OVER() || (condition));      \
   } while (0)                                  \



// non-EVENT inter-thread signals

#define PULSE(flag)                             \
  do                                            \
   {                                            \
    flag = true;                                \
    WAIT (true);                                \
    flag = false;                               \
   } while (0)                                  \


#endif   // THREADKIT_H



