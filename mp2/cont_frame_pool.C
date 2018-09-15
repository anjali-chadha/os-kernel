/*
 File: ContFramePool.C
 
 Author: Anjali Chadha
 Date  : 09/11/2018
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool* ContFramePool::frame_pools_head;

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames)
{
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    nInfoFrames = _n_info_frames;

    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

    // Number of frames must "fill" the bitmap!
    assert ((nframes % 4 ) == 0);
   
    //Creating a singly linked list of all the contiguous frame pools.
    //A new frame pool is always added to the front of the linked list.
    if(ContFramePool::frame_pools_head != NULL) {
	nextPool = ContFramePool::frame_pools_head;
     }
     ContFramePool::frame_pools_head = this;	
    

    // Everything ok. Proceed to mark all bits in the bitmap
    for(int i=0; i*4 < _n_frames; i++) {
        bitmap[i] = 0xFF;
    }

    // Mark the first frame as being used if it is being used
    if(_info_frame_no == 0) {
	//TODO: Set number of frames as allocated
        nFreeFrames -= nInfoFrames;
    }
    Console::puts("Contiguous Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // Any frames left to allocate?
    assert(nFreeFrames >= _n_frames);

    unsigned int frame_no = base_frame_no;
    unsigned int total_frames = 0;
    unsigned long first_free_frame = 0x00;
    unsigned long current_frame = base_frame_no;
    
    while(total_frames > 0) {
        
        //Get the status of current frame
	char status = get_frame_status(current_frame);
	
        //If the frame is HEAD_OF_SEQUENCE, look further 
        //for consecutive (n-1) free frames 
        if(status == HEAD_OF_SEQUENCE) {
            first_free_frame = current_frame;
            current_frame++; 
            total_frames--;
            for(int i = 1; i < _n_frames; i++) {
		total_frames--;
            	if(get_frame_status(current_frame++) != FREE) {
		   break;
		}
	    }    
            return allocate_frames(first_free_frame, _n_frames);
         } else {
	   current_frame++;
           total_frames--;
         }  
    }
    
    //If the program reaches till this point, that means
    // number of contiguous frames required were unavailable
    Console::puts("This operation cannot be fulfilled! Inadequate number of free frames available!");
    return 0;
}

unsigned long ContFramePool::allocate_frames(unsigned long head_of_sequence_frame, unsigned int no_of_frames) {

     assert(head_of_sequence_frame >= base_frame_no);
     assert(head_of_sequence_frame + no_of_frames < base_frame_no + nframes);

     unsigned long current_frame = head_of_sequence_frame;
     set_frame_status(current_frame++, HEAD_OF_SEQUENCE);
     while(no_of_frames-- > 1) {
	 set_frame_status(current_frame++, ALLOCATED);
	 nFreeFrames--;     
     } 
     return head_of_sequence_frame;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
     allocate_frames(_base_frame_no, _n_frames);
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    //look for frame pool where "frame_no" resides
    ContFramePool* pool = find_owner_frame_pool(_first_frame_no);
    assert(pool != NULL);
    pool->deallocate_frames(_first_frame_no);
}

ContFramePool* ContFramePool::find_owner_frame_pool(unsigned long frame_no)
{
     assert(frame_pools_head != NULL);

     ContFramePool* currentPool = frame_pools_head;

     while(currentPool != NULL) {
	unsigned current_pool_base_frame_no = currentPool->base_frame_no;
	unsigned current_pool_last_frame_no = current_pool_base_frame_no + currentPool->nframes;
	if(frame_no >= current_pool_base_frame_no && frame_no < current_pool_last_frame_no) {
	    return currentPool;	
	}	
        currentPool = currentPool->nextPool;
     }
     //This will be an error scenario when we are not able to find any frame pool containing the given frame_no.
     return NULL;
}

void ContFramePool::deallocate_frames(unsigned long first_frame) 
{
     assert(first_frame >= base_frame_no);
     assert(first_frame < base_frame_no + nframes);

     unsigned long current_frame = first_frame;
     
     //Go through all the frames until we find a frame with status FREE
     while(get_frame_status(current_frame) != FREE 
	     && current_frame < (base_frame_no + nframes)) {
	 set_frame_status(current_frame++, FREE);
   	 nFreeFrames++;
     } 
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    unsigned long infoFramesCount = _n_frames / (FRAMES_PER_BYTE * FRAME_SIZE);
    
    //Rounding up the number of info frames
    if(_n_frames % (FRAMES_PER_BYTE * FRAME_SIZE) != 0) {
	infoFramesCount += 1;
     }
    return infoFramesCount;
}

unsigned char ContFramePool::get_frame_status(unsigned long frame_number)
{
    //Check if the frame_number lies within a valid range
    assert(frame_number >= base_frame_no && frame_number < base_frame_no + nframes);
    //TODO
    
}

void ContFramePool::set_frame_status(unsigned long frame_number, char status) 
{
    //Check if the frame_number lies within a valid range
    assert(frame_number >= base_frame_no && frame_number < base_frame_no + nframes);
    //Check if the input status is valid
    assert(status == ALLOCATED || status == FREE || status == HEAD_OF_SEQUENCE)
    //TODO:
}

