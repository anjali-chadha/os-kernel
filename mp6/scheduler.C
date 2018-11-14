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
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  if (Machine::interrupts_enabled()) Machine::disable_interrupts();  
  //If there are no threads in the ready queue, return
  if(head == NULL) return;
  
  ReadyQNode* nxt_thread = head;
  head = head->next;
  Thread::dispatch_to(nxt_thread->tcb);
  if (!Machine::interrupts_enabled()) Machine::enable_interrupts();
}


void Scheduler::resume(Thread * _thread) {
  if (Machine::interrupts_enabled()) Machine::disable_interrupts();      
  ReadyQNode* nw_thrd = new ReadyQNode();
  nw_thrd->tcb = _thread;
  nw_thrd->next = NULL;
  if(head == NULL) {
	head = nw_thrd;
	tail = nw_thrd;
  } else {
	tail->next = nw_thrd;
	tail = nw_thrd;
  }
  if (!Machine::interrupts_enabled()) Machine::enable_interrupts();
}

void Scheduler::add(Thread * _thread) {
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  if (Machine::interrupts_enabled()) Machine::disable_interrupts();  
  if(head == NULL) return;
  ReadyQNode* curr = head;
  ReadyQNode* pre = NULL;

  while(curr->tcb != _thread) {
     pre = curr;
     curr = curr->next;
     if(curr == NULL) return; //When the thread is not present in the ready queue
  }
  pre->next = curr->next;
  delete(curr);
  if (!Machine::interrupts_enabled()) Machine::enable_interrupts();
  return;
}
