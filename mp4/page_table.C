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
   unsigned long page_directory_frame = kernel_mem_pool->get_frames(1);
   unsigned long page_directory_memory_add = page_directory_frame * PAGE_SIZE;
   page_directory = (unsigned long*) (page_directory_memory_add);

   //Allocate 1 frame(4kb) from kernel memory pool to Page Table
   unsigned long * page_table = (unsigned long*) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);

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
   for(i=1; i<1024; i++)
   {  // attribute set to: supervisor level, read/write, not present(010 in binary)
      page_directory[i] = 0 | 2; 
   }
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

  unsigned long page_fault_address = read_cr2();
  unsigned long * page_directory = current_page_table -> page_directory;

  unsigned long page_directory_index = page_fault_address >> 22;
  unsigned long page_table_index = (page_fault_address & 0x3ff000) >> 12;
  unsigned long PDE = page_directory[page_directory_index];
  unsigned long * ptable;
  
  if ((PDE&1) != 1) {
    //Page Fault at PageDirectory level
     
    //Allocate memory to new page table
    ptable = (unsigned long*) ((kernel_mem_pool->get_frames(1) * PAGE_SIZE));

    //Fill the new page table
    for (int i = 0; i < ENTRIES_PER_PAGE; i++) {
      ptable[i] = 0|2;
    }
   
    page_directory[page_directory_index] = (unsigned long) ptable;
    page_directory[page_directory_index] |= 3;   
  } else {
    //Page Table already exists
    ptable = (unsigned long *) (PDE & 0xFFFFF000);

  } 

  assert((ptable[page_table_index] & 1) == 0)
  
  //
  ptable[page_table_index] = (process_mem_pool->get_frames(1) * PAGE_SIZE);
  ptable[page_table_index] |= 3; 
  Console::puts("handled page fault\n");
  }


