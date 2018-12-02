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

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    blocks = NULL;
    Console::puts("In file constructor.\n");
}

File::File(unsigned int _file_id, FileSystem * _file_system) {
    fileSystem = _file_system;
    fileId = _file_id;

    Console::puts("In file constructor.\n");
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file\n");

}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file\n");

}

void File::Reset() {
    Console::puts("reset current position in file\n");
    currBlock = 0;
    currPos = 0;
}

void File::Rewrite() {
    Console::puts("erase content of file\n");

}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    if(fileSize == 0) return true;

    return false;
}
