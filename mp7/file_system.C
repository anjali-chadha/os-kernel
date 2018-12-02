/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    files = NULL;
    size = 0;

}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system form disk\n");
    disk = _disk;

}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    Console::puts("formatting disk\n");
    FILE_SYSTEM->disk = _disk;
    int blocksCount = _size/BLOCK_SIZE;
    if(blocksCount * BLOCK_SIZE < _size)
        blocksCount += 1;

    //clear superblock data

}

File * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file\n");
    assert(false);
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file\n");

    // File Lookup
    for(int i = 0; i < fileCount; i++) {
        //File already present. Don't create new file
        if(files[i].fileId == _file_id) return false;
    }

    //Create new file
    File* newFile = (File*) new File(this, disk);
    newFile->fileId = _file_id;
    //newFile->Rewrite();
    newFile->inodeBlock = findFreeBlock();

    addFileToList(newFile);
    return true;
}

int FileSystem::findFreeBlock() {

}

void FileSystem::addFileToList(File* newFile) {
    if (fileCount == 0) {
        files = (File*) new File[1];
        files[0] = *newFile;
    } else {
        File* new_files = (File*)new File[fileCount + 1];
        for (int i = 0; i < fileCount; i++) {
            new_files[i] = files[i];
        }
        new_files[fileCount] = *newFile;
        delete[] files;
        files = new_files;
    }
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file\n");

    int fileIndex = -1;
    for(int i = 0; i < fileCount; i++) {
        if(files[i].fileId == _file_id) {
            fileIndex = i;
            break;
        }
    }
    if(fileIndex == fileCount) return false;

    files[fileIndex].Rewrite();
    //TODO set the block to free
    if (fileCount == 1) {
        delete[] files;
        files = NULL;
    } else {
        int newFileCount = fileCount - 1;
        File* newFiles = (File*) new File[newFileCount];
        for (int n = 0, old = 0; n < newFileCount, old < fileCount; old++) {
            if (old != fileIndex) {
                newFiles[n] = files[old];
                n++;
            }
        }
        delete[] files;
        files = newFiles;
    }
    return true;
}
