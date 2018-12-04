/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "console.H"
#include "assert.H"
#include "file_system.H"

unsigned int FileSystem::freeBlock = 0;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    head = new FileNode();
    tail = head;
    FileSystem::freeBlock = 1;
    FileSystem::freeBlock++;
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

static int totalDirectories;
static int totalMemoryRemaining;
static int totalFiles;
static int size;
bool
FileSystem::Mount(SimpleDisk *_disk) {
    Console::puts("Starting to perform mount operation on the disk\n");
    simpleDisk = _disk;

    memset(diskBuff, 0, BLOCK_SIZE);
    (*simpleDisk).read(0, diskBuff);
    memcpy(&size, diskBuff + POINTER_INFO_SIZE, POINTER_INFO_SIZE);
    totalMemoryRemaining = size;
    totalDirectories = 0;
    memcpy(&totalFiles, diskBuff, POINTER_INFO_SIZE);
    Console::putui(totalFiles);
    Console::puts("Finished mount operation successfully! \n");
    return true;
}

bool
FileSystem::Format(SimpleDisk *_disk, unsigned int _size) {
    Console::puts("Formatting Disk to new size ");
    Console::putui(_size);
    Console::puts("\n");
    unsigned int temp = 0;
    freeBlock = 2;
    unsigned int totalBlocks = calculateBlocksRequired(_size);

    memset(diskBuff, 0, BLOCK_SIZE);
    totalDirectories = 0;
    getString(diskBuff, _disk, totalBlocks);

    resetFSParams(_size, temp, diskBuff);
    (*_disk).write(0, diskBuff);
    return true;
}

static int totalMemoryUsed;
void
FileSystem::resetFSParams(unsigned int _size, unsigned int &temp, unsigned char *buff) {
    memcpy(buff, &temp, POINTER_INFO_SIZE);
    temp = _size;
    memcpy(buff + 2 * POINTER_INFO_SIZE, &freeBlock, POINTER_INFO_SIZE);
    totalMemoryUsed = 0;
    memcpy(buff + POINTER_INFO_SIZE, &temp, POINTER_INFO_SIZE);
    totalMemoryRemaining = _size;
    memset(buff + 3 * POINTER_INFO_SIZE, 0, 500);
    totalFiles = 0;
    totalDirectories = 0;
}

static int currBlock;
unsigned char *
FileSystem::getString(unsigned char *buff, SimpleDisk *_disk, unsigned int totalBlocks) {
    currBlock = 1;
    (*_disk).write(currBlock++, buff);

    nextBlock = -1;
    while (currBlock < (totalBlocks - 1)) {
        //Update the value of the next block
        nextBlock = currBlock + 1;
        totalMemoryUsed--;
        totalMemoryRemaining++;
        memcpy(buff + ACTUAL_FILE_SIZE, &nextBlock, POINTER_INFO_SIZE);
        (*_disk).write(currBlock++, buff);
    }

    nextBlock = -1; //indicates null
    memcpy(buff + ACTUAL_FILE_SIZE, &nextBlock, POINTER_INFO_SIZE);
    (*_disk).write(currBlock, buff);
}

unsigned int
FileSystem::calculateBlocksRequired(unsigned int _size) {
    unsigned int totalBlocks = _size / BLOCK_SIZE;
    if (totalBlocks * BLOCK_SIZE < _size)
        totalBlocks += 1;
    return totalBlocks;
}

File *
FileSystem::LookupFile(int _file_id) {
    Console::puts("Looking up for file ");
    Console::putui(_file_id);
    Console::puts("\n");

    FileNode *curr = head->next;
    if (NULL == curr) return NULL;

    for (int i = 0; i < totalFiles; i++) {
        if (curr->fileId == _file_id) {
            return curr->file;
        }
        curr = curr->next;
    }
    return NULL;
}

bool
FileSystem::CreateFile(int _file_id) {
    Console::puts("Creating file\n");
    Console::putui(_file_id);

    unsigned int dataBlock;

    // First check if the file exists with same fileId
    if (LookupFile(_file_id)) return false;

    totalFiles += 1;

    memset(diskBuff, 0, BLOCK_SIZE);
    dataBlock = getFirstBlockForNewFile(dataBlock, diskBuff);
    File *newFile = new File(_file_id, &dataBlock);
    addToFileNodeList(newFile);
    totalDirectories = 0;
    return true;
}

unsigned int
FileSystem::getFirstBlockForNewFile(unsigned int &dataBlock, unsigned char *diskBuff) const {
    simpleDisk->read(freeBlock, diskBuff);
    memcpy(&dataBlock, diskBuff + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
    totalMemoryRemaining--;
    simpleDisk->read(dataBlock, diskBuff);
    totalMemoryUsed++;
    memcpy(&freeBlock, diskBuff + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
    return dataBlock;
}

void
FileSystem::addToFileNodeList(File *newFile) const {
    FileNode *newNode = new FileNode();
    newNode->fileId = newFile->fileId;
    newNode->file = newFile;
    newNode->isFile = true;
    newNode->lastUpdateTime = 100;  // TODO Replace that with cpp time directory
    newNode->next = NULL;

    if (totalFiles == 0) {
        head->next = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
}

bool
FileSystem::DeleteFile(int _file_id) {
    FileNode *pre = head;
    FileNode *curr = head->next;

    if (!curr) return false;

    //File already exists
    if (!LookupFile(_file_id)) return false;

    totalFiles--;
    int i = 0;
    for (; i < totalFiles; i++) {
        if (curr->fileId == _file_id) {
            freeFileMemory(curr);
            break;
        }
        pre = pre->next;
        curr = curr->next;
    }
    totalMemoryUsed -= curr->file->countDataBlocks;
    totalMemoryRemaining += curr->file->countDataBlocks;
    if (NULL == curr->next) {
        pre->next = NULL;
        tail = pre;
    } else {
        pre->next = curr->next;
    }
    return true;
}

void
FileSystem::freeFileMemory(FileNode *fileNode) {
    if (!fileNode->isFile)
        return;
    currBlock = 0;
    memset(diskBuff, 0, BLOCK_SIZE);
    totalMemoryUsed -= fileNode->file->countDataBlocks;
    unsigned int info_block = fileNode->file->startBlockInfo;
    memset(diskBuff2, 0, BLOCK_SIZE);
    totalMemoryRemaining += fileNode->file->countDataBlocks;
    simpleDisk->read(info_block, diskBuff2);
    memcpy(&currBlock, diskBuff2 + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
    memcpy(diskBuff + ACTUAL_FILE_SIZE, &freeBlock, POINTER_INFO_SIZE);
    simpleDisk->write(info_block, diskBuff);
    fileNode->lastUpdateTime = -1;
    freeBlock = info_block;
    freeMemoryHelper(fileNode, currBlock, diskBuff, diskBuff2);
}

void
FileSystem::freeMemoryHelper(const FileNode *fileNode, unsigned int currBlock, unsigned char *buff1,
                             unsigned char *buff2) const {
    if (!fileNode->isFile) return;
    nextBlock = 0;
    int dataBlocks2Delete = fileNode->file->countDataBlocks;
    for (int j = 0; j < dataBlocks2Delete; j++) {
        simpleDisk->read(currBlock, buff2);
        if (fileNode->lastUpdateTime == -1) return;
        memcpy(&nextBlock, buff2 + ACTUAL_FILE_SIZE, POINTER_INFO_SIZE);
        totalMemoryRemaining += fileNode->file->countDataBlocks;
        memcpy(buff1 + ACTUAL_FILE_SIZE, &freeBlock, POINTER_INFO_SIZE);
        totalMemoryUsed -= fileNode->file->countDataBlocks;

        simpleDisk->write(currBlock, buff1);

        freeBlock = currBlock;
        currBlock = nextBlock;
    }
}