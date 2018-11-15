/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "simple_disk.H"
#include "scheduler.H"
extern Scheduler* SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
        : SimpleDisk(_disk_id, _size) {
    head = NULL;
    tail = NULL;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {

    issue_operation(READ, _block_no);
    Thread* curr_thrd = Thread::CurrentThread();
    addToBlockedThreadsQueue(curr_thrd);
    while (!is_ready()) {
        SYSTEM_SCHEDULER->resume(curr_thrd);
        SYSTEM_SCHEDULER->yield();
    }
    removeReadyThreadFromBlockedQueue(curr_thrd);
    /* read data from port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i*2]   = (unsigned char)tmpw;
        _buf[i*2+1] = (unsigned char)(tmpw >> 8);
    }
}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
    issue_operation(READ, _block_no);
    Thread* curr_thrd = Thread::CurrentThread();
    addToBlockedThreadsQueue(curr_thrd);
    while (!is_ready()) {
        SYSTEM_SCHEDULER->resume(curr_thrd);
        SYSTEM_SCHEDULER->yield();
    }
    removeReadyThreadFromBlockedQueue(curr_thrd);
    /* write data to port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
        Machine::outportw(0x1F0, tmpw);
    }

}

void BlockingDisk::addToBlockedThreadsQueue(Thread* thread) {
    BlockedQNode* nw_thrd = new BlockedQNode();
    nw_thrd->tcb = thread;
    nw_thrd->next = NULL;
    if(head == NULL) {
        head = nw_thrd;
        tail = nw_thrd;
    } else {
        tail->next = nw_thrd;
        tail = nw_thrd;
    }
}

void BlockingDisk::removeReadyThreadFromBlockedQueue(Thread* thread) {
    if(head == NULL) return;
    BlockedQNode* prev = NULL;
    BlockedQNode* curr = head;
    while(curr != NULL) {
        //Remove this thread from the queue
        if(curr->tcb == thread) {
            if(prev == NULL) {
                head = head->next;
                return;
            } else {
                prev->next = curr->next;
                return;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    return;
}