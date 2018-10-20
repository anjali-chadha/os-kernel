/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  head = NULL;
  tail = NULL;
  ready_queue_size = 0;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  
}

void Scheduler::resume(Thread * _thread) {
  ReadyQNode* nw_thrd = new ReadyQNode();
  nw_thrd->tcb = _thread;
  nw_thrd->next = NULL;
  if(ready_queue_size == 0) {
	head = nw_thrd;
	tail = nw_thrd;
  } else {
	tail->next = nw_thrd;
	tail = nw_thrd;
  }
  ready_queue_size++;
}

void Scheduler::add(Thread * _thread) {
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  assert(false);
}
