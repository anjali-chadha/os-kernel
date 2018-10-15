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
    base_address &= 0xfffff000; //Removing lowest three bits of the input address
    size = _size;
    pagesCount = size/(PageTable::PAGE_SIZE);
    frame_pool = _frame_pool;
    page_table = _page_table;
    regionsCount = 0;
    region_list = (PoolRegion*)(_base_address);
    page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {

    assert(_size > 0);
    if(regionsCount >= VMPool::MAX_REGIONS) {
         Console::puts("Cannot allocate more regions for the current memory pool!\n");
	 for(;;);
    }
    unsigned long start_address;
    if(regionsCount <= 0) {
        start_address = base_address + (PageTable::PAGE_SIZE);	
    } else {
	unsigned long last_start_address = region_list[regionsCount-1].start_address;
	unsigned long size = region_list[regionsCount - 1].size;
        start_address = (last_start_address + size) * (PageTable::PAGE_SIZE);
}
    region_list[regionsCount].start_address = start_address;
    region_list[regionsCount].size = _size/(PageTable::PAGE_SIZE);
    regionsCount++;
    Console::puts("Allocated region of memory.\n");
    return start_address;
}

void VMPool::release(unsigned long _start_address) {
    assert(regionsCount > 0);
    unsigned int rgn_index = -1;
    // Find the region with this start_address
    for(int i = 0; i < regionsCount; i++) {
	if(region_list[i].start_address == _start_address) {
	    rgn_index = i;
	    break;
	}
    }
    assert(rgn_index != -1);
    unsigned int rgn_size = region_list[rgn_index].size;

   //Free up the related pages of the region 
   for (int j = 0; j < rgn_size; j++) {
	page_table->free_page(_start_address);
	_start_address = _start_address + PageTable::PAGE_SIZE;
	page_table->load(); //This step is to re-read from CR2 directory to flush the TLB
     }

    //Fill up the hole in array
    for(int p = rgn_index; p < regionsCount-1; p++) { region_list[p] = region_list[p+1];}
    regionsCount--;
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    //Range Check
    if(_address >= base_address && _address < (base_address + pagesCount)) return true;
    Console::puts("Checked whether address is part of an allocated region.\n");
    return false;
}

void VMPool::print_vmpool_info() {
   Console::puts("Base Address = ");
   Console::puti((int)base_address);
   Console::puts("\n");
   Console::puts("Size=");Console::puti((int)size);Console::puts("\n");
}
