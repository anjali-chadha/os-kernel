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

File::File(int _fileId, unsigned int* _startBlock, unsigned int* _dataBlock) {
    Console::puts("In file constructor.\n");

    countDataBlocks = 1;
    startBlockData = *_dataBlock;
    currBlock = startBlockData;
    fileSize = BLOCK_SIZE;
    fileId = _fileId;
    startBlockInfo = *_startBlock;
    Console::puts("File object created with fileId: ");
    Console::puti(_fileId);
    Console::puts("\n");
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("Starting to Read from file\n");
    unsigned char *disk_buff = new unsigned char[BLOCK_SIZE];

    int i = 0;
    unsigned int remainingChars = 0;
    unsigned int remainingInCurrentBlock = 0;

    for (i = 0; i < _n;) {
        if(EoF()) {
            Console::puts("EOF reached! \n");
            Console::puts("Number of chars read = ");
            Console::putui(i);
            Console::puts("\n");
            return i;
        }
        remainingChars = _n - i;
        remainingInCurrentBlock = ACTUAL_FILE_SIZE - currPosition;
        FILE_SYSTEM->simpleDisk->read(currBlock, disk_buff);
        if (remainingChars > remainingInCurrentBlock) {
            memcpy(_buf + i, disk_buff + currPosition, remainingInCurrentBlock);
            i += remainingInCurrentBlock;
            //update the current block
            memcpy(&currBlock, disk_buff + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
            currPosition = 0;
        } else {
            break;
        }
    }

    memcpy(_buf + i, disk_buff + currPosition, remainingChars);
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
    unsigned int remainingChars = 0;
    unsigned int remainingInCurrentBlock = 0;
    unsigned char* disk_buff = new unsigned char[BLOCK_SIZE];

    for(pos = 0; pos < _n;) {
        if(EoF()) {
            //Perform resizing
            resizeFile();
        }
        remainingChars = _n - pos;
        remainingInCurrentBlock = ACTUAL_FILE_SIZE - currPosition;

        (*FILE_SYSTEM).simpleDisk->read(currBlock, disk_buff);

        if(remainingChars > remainingInCurrentBlock){
            memcpy(disk_buff + currPosition, _buf + pos, remainingInCurrentBlock);
            FILE_SYSTEM->simpleDisk->write(currBlock, disk_buff);
            pos += remainingInCurrentBlock;
            countDataBlocks++;
            //Allocate free block
            currBlock = FileSystem::freeBlock;
            (*FILE_SYSTEM).simpleDisk->read(FileSystem::freeBlock, disk_buff);
            memcpy(&(FileSystem::freeBlock), disk_buff + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
            currPosition = 0;
        } else {
            break;
        }
    }

    memcpy(disk_buff + currPosition, _buf + pos, remainingChars);

    (*FILE_SYSTEM).simpleDisk->write(currBlock, disk_buff);
    currPosition   += remainingChars;
    Console::puts("Write Operation finished!");
}

void File::resizeFile() {
    fileSize += BLOCK_SIZE;
}

void File::Reset() {
    Console::puts("reset current position in file\n");
    currBlock = startBlockData;	//first block is used to store management information
    currPosition = 0;
}

void File::Rewrite() {
    Console::puts("erase content of file\n");
    unsigned int currBlock = startBlockData;
    unsigned int nextBlock = 0;

    unsigned char* buff1 = new unsigned char[BLOCK_SIZE];
    unsigned char* buff2 = new unsigned char[BLOCK_SIZE];

    for(int i = 0; i < countDataBlocks; i++) {
        if((int)nextBlock == -1) break;

        (*FILE_SYSTEM).simpleDisk->read(currBlock, buff2);

        memcpy(&nextBlock, buff2 + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);

        memcpy(buff1 + ACTUAL_FILE_SIZE, &(FileSystem::freeBlock), POINTER_INFO_SIZE);

        (*FILE_SYSTEM).simpleDisk->write(currBlock, buff1);

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


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    int loc = countDataBlocks * BLOCK_SIZE + currPosition;
    return loc > fileSize;
}
