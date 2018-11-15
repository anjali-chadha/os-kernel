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

MirroredDisk::MirroredDisk(DISK_ID _disk_id, unsigned int _size)
        : SimpleDisk(_disk_id, _size) {
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool MirroredDisk::is_ready() {
    return ((Machine::inportb(0x177) & 0x08) != 0);
}

void MirroredDisk::issue_operation(DISK_OPERATION _op, unsigned long _block_no) {
    Machine::outportb(0x171, 0x00); /* send NULL to port 0x1F1         */
    Machine::outportb(0x172, 0x01); /* send sector count to port 0X1F2 */
    Machine::outportb(0x173, (unsigned char)_block_no);
    /* send low 8 bits of block number */
    Machine::outportb(0x174, (unsigned char)(_block_no >> 8));
    /* send next 8 bits of block number */
    Machine::outportb(0x175, (unsigned char)(_block_no >> 16));
    /* send next 8 bits of block number */
    Machine::outportb(0x176, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | (disk_id << 4));
    /* send drive indicator, some bits,
       highest 4 bits of block no */

    Machine::outportb(0x177, (_op == READ) ? 0x20 : 0x30);

}

void MirroredDisk::wait_until_ready() {
    //addToBlockedThreadsQueue(Thread::CurrentThread());
    while(!is_ready()) {
        SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
        SYSTEM_SCHEDULER->yield();
    }

    //removeReadyThreadFromBlockedQueue(Thread::CurrentThread());
}
void MirroredDisk::issue_read(unsigned long _block_no) {
    /* Reads 512 Bytes in the given block of the given disk drive and copies them
      to the given buffer. No error check! */
    issue_operation(READ, _block_no);

}

void MirroredDisk::read_from_buffer(unsigned char* _buf) {
    /* read data from port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x170);
        _buf[i*2]   = (unsigned char)tmpw;
        _buf[i*2+1] = (unsigned char)(tmpw >> 8);
    }
}


void MirroredDisk::issue_write(unsigned long _block_no) {
    /* Writes 512 Bytes from the buffer to the given block on the given disk drive. */

    issue_operation(WRITE, _block_no);
}

void MirroredDisk::write_to_buffer(unsigned char *_buf) {
    /* write data to port */
    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
        Machine::outportw(0x170, tmpw);
    }
}

void MirroredDisk::addToBlockedThreadsQueue(Thread* thread) {
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

void MirroredDisk::removeReadyThreadFromBlockedQueue(Thread* thread) {
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

