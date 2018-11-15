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
#include "mirrored_disk.H"

extern Scheduler* SYSTEM_SCHEDULER;
MirroredDisk* SECONDARY_DISK;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
        : SimpleDisk(_disk_id, _size) {
  SECONDARY_DISK = new MirroredDisk(SLAVE, _size);
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/


void BlockingDisk::wait_until_ready() {
    //addToBlockedThreadsQueue(Thread::CurrentThread());
    while(!is_ready() || SECONDARY_DISK->is_ready()) {
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
        SYSTEM_SCHEDULER->yield();
    }
    //removeReadyThreadFromBlockedQueue(Thread::CurrentThread());
}
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
    /* Reads 512 Bytes in the given block of the given disk drive and copies them
      to the given buffer. No error check! */

    SECONDARY_DISK->issue_read(_block_no);
    issue_operation(READ, _block_no);

    wait_until_ready();

    /* read data from port */
    if(is_ready()) {
        int i;
        unsigned short tmpw;
        for (i = 0; i < 256; i++) {
            tmpw = Machine::inportw(0x1F0);
            _buf[i * 2] = (unsigned char) tmpw;
            _buf[i * 2 + 1] = (unsigned char) (tmpw >> 8);
        }
    } else {
        SECONDARY_DISK->read_from_buffer(_buf);
    }
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
    /* Writes 512 Bytes from the buffer to the given block on the given disk drive. */

    SECONDARY_DISK->issue_write(_block_no);
    issue_operation(WRITE, _block_no);

    wait_until_ready();

    /* write data to port */
    if(is_ready()) {
        int i;
        unsigned short tmpw;
        for (i = 0; i < 256; i++) {
            tmpw = _buf[2 * i] | (_buf[2 * i + 1] << 8);
            Machine::outportw(0x1F0, tmpw);
        }
    } else {
        SECONDARY_DISK->write_to_buffer(_buf);
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

