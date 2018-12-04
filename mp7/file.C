/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
extern FileSystem* FILE_SYSTEM;

File::File(int _fileId, unsigned int* _dataBlock) {
    Console::puts("In file constructor.\n");

    countDataBlocks = 1;
    startBlockData = *_dataBlock;
    currBlock = startBlockData;
    fileSize = BLOCK_SIZE;
    fileId = _fileId;
    Console::puts("File object created with fileId: ");
    Console::puti(_fileId);
    Console::puts("\n");
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("Starting to Read from file\n");
    memset(diskBuff, 0, BLOCK_SIZE);

    int i = 0;
    int remainingChars = 0;
    int remainingInCurrentBlock = 0;

    for (i = 0; i < _n;) {
        remainingChars = _n - i;
        if(EoF()) {
            Console::puts("EOF reached! \n");
            Console::puts("Number of chars read = ");
            Console::putui(i);
            Console::puts("\n");
            return i;
        }
        remainingInCurrentBlock = ACTUAL_FILE_SIZE - currPosition;
        FILE_SYSTEM->simpleDisk->read(currBlock, diskBuff);
        if (remainingChars > remainingInCurrentBlock) {
            memcpy(_buf + i, diskBuff + currPosition, remainingInCurrentBlock);
            i += remainingInCurrentBlock;
            //update the current block
            memcpy(&currBlock, diskBuff + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
            currPosition = 0;
        } else {
            break;
        }
    }

    memcpy(_buf + i, diskBuff + currPosition, remainingChars);
    currPosition += remainingChars;
    Console::puts("Finished Read ");
    Console::putui(_n);
    Console::puts(" characters from file. \n");
    return _n;
}



void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("Starting to write to file ");
    Console::putui(_n);
    Console::puts(" chars. \n");

    Console::puts("File size is: ");
    Console::putui(fileSize);
    Console::puts("\n");

    int pos = 0;
    int remainingChars = 0;
    memset(diskBuff, 0, BLOCK_SIZE);
    int remainingInCurrentBlock = 0;
    for(pos = 0; pos < _n;) {
        remainingChars = _n - pos;
        if(EoF()) {
            //Perform resizing
            resizeFile();
        }
        remainingInCurrentBlock = ACTUAL_FILE_SIZE - currPosition;

        (*FILE_SYSTEM).simpleDisk->read(currBlock, diskBuff);

        if(remainingChars > remainingInCurrentBlock){
            memcpy(diskBuff + currPosition, _buf + pos, remainingInCurrentBlock);
            FILE_SYSTEM->simpleDisk->write(currBlock, diskBuff);
            pos += remainingInCurrentBlock;
            countDataBlocks++;
            //Allocate free block
            currBlock = FileSystem::freeBlock;
            (*FILE_SYSTEM).simpleDisk->read(FileSystem::freeBlock, diskBuff);
            memcpy(&(FileSystem::freeBlock), diskBuff + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
            currPosition = 0;
        } else {
            break;
        }
    }

    memcpy(diskBuff + currPosition, _buf + pos, remainingChars);

    (*FILE_SYSTEM).simpleDisk->write(currBlock, diskBuff);
    currPosition += remainingChars;
    Console::puts("Write Operation finished!");
}

void File::resizeFile() {
    fileSize += BLOCK_SIZE;
}

void File::Reset() {
    Console::puts("reset current position in file\n");
    currBlock = startBlockData;
    currPosition = 0;
}

void File::Rewrite() {
    Console::puts("erase content of file\n");
    unsigned int currBlock = startBlockData;

    memset(diskBuff, 0, BLOCK_SIZE);
    nextBlock = 0;
    memset(diskBuff2, 0, BLOCK_SIZE);

    for(int i = 0; i < countDataBlocks; i++) {
        if(nextBlock == -1) break;

        (*FILE_SYSTEM).simpleDisk->read(currBlock, diskBuff2);

        memcpy(&nextBlock, diskBuff2 + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);

        memcpy(diskBuff + ACTUAL_FILE_SIZE, &(FileSystem::freeBlock), POINTER_INFO_SIZE);

        (*FILE_SYSTEM).simpleDisk->write(currBlock, diskBuff);

        if(startBlockData == currBlock) {
            //we need to keep atleast one block for the file
        }
        else FileSystem::freeBlock = currBlock;

        currBlock = nextBlock;
    }
    resetFileSysParams(currBlock);
}

void File::resetFileSysParams(unsigned int currBlock) {
    fileSize = BLOCK_SIZE;
    currPosition = 0;
    currBlock = startBlockData;
    countDataBlocks = 1;
}


inline bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    int loc = countDataBlocks * BLOCK_SIZE + currPosition;
    return loc > fileSize;
}
