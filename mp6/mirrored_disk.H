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

class MirroredDisk : public SimpleDisk {

private:
     /* -- FUNCTIONALITY OF THE IDE LBA28 CONTROLLER */

     DISK_ID      disk_id;            /* This disk is either MASTER or SLAVE */

     unsigned int disk_size;          /* In Byte */
  void issue_operation(DISK_OPERATION _op, unsigned long _block_no);
    /* Send a sequence of commands to the controller to initialize the READ/WRITE
       operation. This operation is called by read() and write(). */


    /* -- HERE WE CAN DEFINE THE BEHAVIOR OF DERIVED DISKS */

    

public:
     bool is_ready();
     /* Return true if disk is ready to transfer data from/to disk, false otherwise. */

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
