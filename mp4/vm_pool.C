/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}


//TODO: Perfrom roundUp operation of the input size
unsigned long VMPool::allocate(unsigned long _size) {

    assert(_size > 0);
    if(regionsCount >= VMPool::MAX_REGIONS) {
         Console::puts("Cannot allocate more regions for the current memory pool!\n");
	 for(;;);
    }
    unsigned long start_address;
    if(regionsCount <= 0) {
        start_address = base_address;	
    } else {
	unsigned long last_start_address = regions[regionsCount-1].start_address;
	unsigned long size = regions[regionsCount - 1].size;
        start_address = last_start_address + size;
}
    regions[regionsCount].start_address = start_address;
    regions[regionsCount].size = size;
    regionsCount++;
    Console::puts("Allocated region of memory.\n");
    return start_address;
}

void VMPool::release(unsigned long _start_address) {

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    
    for(int i = 0; i < regionsCount; i++) {
	unsigned long rgn_addr = regions[i].start_address;
	unsigned int rgn_size = regions[i].size * Machine::PAGE_SIZE;
	//Range Check
	if(_address >= rgn_addr && _address < (rgn_addr+rgn_size)) return true;
    }
    Console::puts("Checked whether address is part of an allocated region.\n");
    return false;
}

