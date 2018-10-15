#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{  
   //Allocate 1 frame(4kb) from kernel memory pool to Page Directory 
   unsigned long page_directory_frame = process_mem_pool->get_frames(1);
   unsigned long page_directory_memory_add = page_directory_frame * PAGE_SIZE;
   page_directory = (unsigned long*) (page_directory_memory_add);

   //Allocate 1 frame(4kb) from process memory pool to Page Table
   unsigned long * page_table = (unsigned long*) (process_mem_pool->get_frames(1) * PAGE_SIZE);

   // map the first 4MB of memory
   unsigned long address=0;
   unsigned int i;
   for(i=0; i<1024; i++)
   {  // attribute set to: supervisor level, read/write, present(011 in binary)
      page_table[i] = address | 3; 
      address = address + PAGE_SIZE; // PAGE_SIZE = 4096 = 4kb
   };
   
   // Fill the first entry of the page directory
   page_directory[0] = (unsigned long)page_table; 
   page_directory[0] = page_directory[0] | 3;

   // Fill the remaining 1023 entries of page directory and set the bits to indicate that their corresponding page table
   // are not present 
   for(i=1; i<1023; i++)
   {  // attribute set to: supervisor level, read/write, not present(010 in binary)
      page_directory[i] = 0 | 2; 
   }

   for(i=0; i < MAX_POOL_SIZE; i++) {pool_list[i] = NULL;}
 
   //Recrusive Page Table Lookup
   page_directory[1023] = ((unsigned long)page_directory) | 3;
   paging_enabled = 0;
   Console::puts("Constructed Page Table object\n");
}

// Put the address of the page directory into CR3.
void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

/*
 * The paging bit of CR0(bit 31) when set to 1 enables paging
*/
void PageTable::enable_paging()
{
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   unsigned int err_code = _r->err_code;
   //If the last bit is not equal to 0, then we can ignore this exception, as it didn't occur due
   //to the missing page
   if((err_code & 1) == 1) {
      Console::puts("Page Fault Exception due to Protection Fault\n");
      for(; ;); //For this program implementation, this is unexpected scenario, stop the program here
      return;
   }
  VMPool** vm_item = current_page_table->pool_list;
  unsigned long page_fault_address = read_cr2();

  //Check whether the address is legitimate or not
  bool isLegitimate = false;
  for(int i = 0;i < MAX_POOL_SIZE; i++)
     if(vm_item[i] != NULL && vm_item[i] -> is_legitimate(page_fault_address )) {
        isLegitimate = true;
        break;
     }

  if(!isLegitimate) {
      Console::puts("Page Fault Address is invalid\n");
      for(;;);
  }

  unsigned long *page_directory = (unsigned long *) 0xFFFFF000;
  unsigned long page_directory_index = page_fault_address >> 22;
  unsigned long *page_table = (unsigned long *) (0xFFC00000 | (page_directory_index << 12));
  unsigned long table_index = ((page_fault_address >> 12) & 0x03FF);
  unsigned long PDE = page_directory[page_directory_index];
  unsigned long * ptable;
  
  if ((PDE&1) != 1) {
    //Page Fault at PageDirectory level
     //Allocate memory to new page table
    page_directory[page_directory_index] = (unsigned long) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);
    page_directory[page_directory_index] |= 3;
    for(int i = 0; i < 1024; i++) {
	page_table[i] = 0 | 2;
    }
  } 
  page_table[table_index] = (unsigned long)(process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
  Console::puts("handled page fault\n");
 }


void PageTable::register_pool(VMPool * _vm_pool)
{
   if (pool_size >= MAX_POOL_SIZE) {
     Console::puts("VM Pool registration failed! PageTable capacity exceeded.\n");
     for(;;);
   }
   pool_list[pool_size] = _vm_pool;
   pool_size++;
   Console::puts("registered VM pool\n");
}


void PageTable::free_page(unsigned long _page_no) {
 
    unsigned long PDE = _page_no >> 22;
    unsigned long* page_table = (unsigned long*)((0xFFC00 | PDE) << 12);
    unsigned long PTE = (_page_no >> 12) & 0x03FF; 
    if((page_table[PTE] & 1) != 0) {
	  ContFramePool::release_frames((page_table[PTE] & 0xFFFFF000)>>12);
          page_table[PTE] &= (0xFFFFFFFE); //Set the entry to indicate it an invalid entry  
         Console::puts("freed page\n");
    }
}
