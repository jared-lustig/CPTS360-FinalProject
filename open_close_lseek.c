#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

extern MINODE *iget();
extern PROC *running; 
extern OFT init_oft[64];

/*Description: 
1. Get mip of file to be opened
2. Check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
3. Check whether the file is ALREADY opened with INCOMPATIBLE mode:
4. If it's already opened for W, RW, APPEND : reject.
5. Allocate a FREE OpenFileTable (OFT) and fill in values:
6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:
7. Find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
      //Let running->fd[i] point at the OFT entry
8. update INODE's time field
9. Mark dirty
10. Return i as the file descriptor
*/
int open_file(char* pathname, int mode)
{
    OFT *ftp;
    // get pathname's inumber, minode pointer:
    int ino = getino(pathname); 
    if (ino == 0){ // if file does not exist
        my_creat(pathname); // creat it first, then
        ino = getino(pathname); // get its ino
    }
    MINODE *mip = iget(dev, ino); // get mip of file to be opened

    // check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
    if(!S_ISREG(mip->INODE.i_mode)) {
        printf("ERROR: File not regular.\n");
        return(-1);
    }
    
      
    //Check whether the file is ALREADY opened with INCOMPATIBLE mode:
    int i = 0;
    while (running->fd[i] != 0 && i < 10) { 
        if(running->fd[i]->minodePtr == mip) {
            if (running->fd[i]->mode == 0 || running->fd[i]->mode == 1 || running->fd[i]->mode == 2 || running->fd[i]->mode == 3) {  
                printf("ERROR: File already open.\n"); //If it's already opened for W, RW, APPEND : reject.
                return(-1);
            }
        }
        i++;
    }

  // allocate a FREE OpenFileTable (OFT) and fill in values:
    for (i = 0; i < NOFT; i++) {
        if (running->fd[i] == NULL) {
            init_oft[i];
            init_oft[i].mode = 1;      // mode = 0|1|2|3 for R|W|RW|APPEND 
            init_oft[i].refCount = 1;
            init_oft[i].minodePtr = mip;  // point at the file's minode[]
            break;
        }
    }


  // Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

      switch(mode){
         case 0 : init_oft[i].offset = 0;     // R: offset = 0
                  break;
         case 1 : truncate(mip);        // W: truncate file to 0 size
                  init_oft[i].offset = 0;
                  break;
         case 2 : init_oft[i].offset = 0;     // RW: do NOT truncate file
                  break;
         case 3 : init_oft[i].offset =  mip->INODE.i_size;  // APPEND mode
                  break;
         default: printf("invalid mode\n");
                  return(-1);
      }

   // find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
      //Let running->fd[i] point at the OFT entry
    if (i == 10) {
        printf("ERROR: FDs are full.\n");
        return(-1);
    }
    running->fd[i] = &init_oft[i];
    

   // update INODE's time field
     //for R: touch atime. 
    if (mode == 0) {
        mip->INODE.i_atime = time(0L);
    }
     // for W|RW|APPEND mode : touch atime and mtime
    if (mode == 1 || mode == 2) {
        mip->INODE.i_atime = time(0L);
        mip->INODE.i_mtime = time(0L);
    }
    //mark Minode[ ] dirty
    mip->dirty = 1;

    //return i as the file descriptor
    return i;
}   

/*Description: 
1. INPUT: file descriptor of file wanting to close
2. check fd is a valid opened file descriptor
3. clear PROC’s fd[fd] to 0
*/
int close(int fd) {
    printf("oldrefCount = %d\n", running->fd[fd]->refCount);
    // check fd is a valid opened file descriptor;
    if (running->fd[fd] != 0){ // points to an OFT
        running->fd[fd]->refCount--; // dec OFT’s refCount by 1
        if (running->fd[fd]->refCount == 0) // if last process using this OFT
            iput(running->fd[fd]->minodePtr); // release minode
    }
    //(4). 
    printf("new refCount = %d\n", running->fd[fd]->refCount);
    running->fd[fd] = 0; // clear PROC’s fd[fd] to 0
}


/*Description: 
1. INPUT: file descriptor of file wanting to seek
2. make sure position is not greater than the file size
3. change OFT entry's offset to position but make sure NOT to over run either end of the file.
4. return original position
*/
int my_lseek(int fd, int position)
{
    printf("in seeek....\n");
    if (position > running->fd[fd]->minodePtr->INODE.i_size) {  // make sure position is not greater than the file size, else fail.
        return -1;
    }
    int originalPosition = running->fd[fd]->offset;  // saving origional position
    running->fd[fd]->offset = position; // change OFT entry's offset to position but make sure NOT to over run either end of the file.

    // return original Position
    printf("op = %d", originalPosition);
    return originalPosition;
}