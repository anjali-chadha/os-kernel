/*
     File        : blocking_disk.H

     Author      :

     Date        :
     Description :

*/

#ifndef _MIRRORED_DISK_H_
#define _MIRRORED_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "blocking_disk.H"
#include "thread.H"
#include "scheduler.H"
extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/

class MirroredDisk : public BlockingDisk {


public:
    MirroredDisk(DISK_ID _disk_id, unsigned int _size);
    /* Creates a BlockingDisk device with the given size connected to the
       MASTER or SLAVE slot of the primary ATA controller.
       NOTE: We are passing the _size argument out of laziness.
       In a real system, we would infer this information from the
       disk controller. */

    /* DISK OPERATIONS */
    virtual void wait_until_ready();
    void issue_read(unsigned long _block_no);
    void read_from_buffer(unsigned char* _buf);
    /* Reads 512 Bytes from the given block of the disk and copies them
       to the given buffer. No error check! */

    void issue_write(unsigned long _block_no);
    void write_to_buffer(unsigned char* _buf);
    /* Writes 512 Bytes from the buffer to the given block on the disk. */

};

#endif
