/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
#define BLOCK_SIZE 512
#define POINTER_INFO_SIZE 4
#define ACTUAL_FILE_SIZE 508

struct FileNode{
    unsigned int fileId;
    File* file;
    bool isFile;
    unsigned long lastUpdateTime;
    FileNode* next;
};

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

class FileSystem {

    friend class File; /* -- not sure if we need this; feel free to delete */

private:
    /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
    static FileNode * head;
    static FileNode * tail;
    static unsigned int size;
    static unsigned int totalFiles;
    static unsigned int totalDirectories;
    static unsigned int totalMemoryUsed;
    static unsigned int totalMemoryRemaining;
    static unsigned int freeBlock;

public:
    SimpleDisk * simpleDisk;

    void freeFileMemory(const FileNode *fileNode);

    void addToFileNodeList(File *newFile) const;

    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */

    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */

    static bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */

    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */

    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */

    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */
    static unsigned int calculateBlocksRequired(unsigned int _size);

    static unsigned char *getString(unsigned char* buff, SimpleDisk *_disk, unsigned int totalBlocks);

    unsigned int getFirstBlockForNewFile(unsigned int &dataBlock, unsigned char *diskBuff) const;

    static void resetFSParams(unsigned int _size, unsigned int &temp, unsigned char *buff);

    void freeMemoryHelper(const FileNode *fileNode, unsigned int currBlock, unsigned char *buff1,
            unsigned char *buff2) const;
};
#endif
