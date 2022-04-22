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


int open_file(char* pathname, int mode)
{
    OFT *ftp;
  //1. ask for a pathname and mode to open:
         //You may use mode = 0|1|2|3 for R|W|RW|APPEND
    

  //2. get pathname's inumber, minode pointer:
    int ino = getino(pathname); 
    if (ino == 0){ // if file does not exist
        my_creat(pathname); // creat it first, then
        ino = getino(pathname); // get its ino
    }
    MINODE *mip = iget(dev, ino); 

  //3. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
    if(!S_ISREG(mip->INODE.i_mode)) {
        printf("ERROR: File not regular.\n");
        return(-1);
    }
    
      
     //Check whether the file is ALREADY opened with INCOMPATIBLE mode:
           //If it's already opened for W, RW, APPEND : reject.
           //(that is, only multiple R are OK)
    int i = 0;
    //ftp = running->fd;
    while (running->fd[i] != 0 && i < 10) { 
        if(running->fd[i]->minodePtr == mip) {
            if (running->fd[i]->mode == 0 || running->fd[i]->mode == 1 || running->fd[i]->mode == 2 || running->fd[i]->mode == 3) {
                printf("ERROR: File already open.\n");
                return(-1);
            }
        }
        i++;
    }

  //4. allocate a FREE OpenFileTable (OFT) and fill in values:
    for (i = 0; i < NOFT; i++) {
        if (running->fd[i] == NULL) {
            init_oft[i];
            init_oft[i].mode = 1;      // mode = 0|1|2|3 for R|W|RW|APPEND 
            init_oft[i].refCount = 1;
            init_oft[i].minodePtr = mip;  // point at the file's minode[]
            break;
        }
    }


  //5. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

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

   //7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
      //Let running->fd[i] point at the OFT entry
    if (i == 10) {
        printf("ERROR: FDs are full.\n");
        return(-1);
    }
    running->fd[i] = &init_oft[i];
    

   //8. update INODE's time field
     //for R: touch atime. 
    if (mode == 0) {
        mip->INODE.i_atime = time(0L);
    }
     //for W|RW|APPEND mode : touch atime and mtime
    if (mode == 1 || mode == 2) {
        mip->INODE.i_atime = time(0L);
        mip->INODE.i_mtime = time(0L);
    }
    //mark Minode[ ] dirty
    mip->dirty = 1;

    //9. return i as the file descriptor
    return i;
}   


int close(int fd) {
    //(1). check fd is a valid opened file descriptor;
    //(2). 
    //OFT *oftp;
    //oftp = &running->fd[fd];
    printf("oldrefCount = %d\n", running->fd[fd]->refCount);
    
    if (running->fd[fd] != 0){ // points to an OFT
        running->fd[fd]->refCount--; // dec OFT’s refCount by 1
        if (running->fd[fd]->refCount == 0) // if last process using this OFT
            iput(running->fd[fd]->minodePtr); // release minode
    }
    //(4). 
    printf("new refCount = %d\n", running->fd[fd]->refCount);
    running->fd[fd] = 0; // clear PROC’s fd[fd] to 0
}

int my_lseek(int fd, int position)
{
    //From fd, find the OFT entry. 

    //change OFT entry's offset to position but make sure NOT to over run either end
    //of the file.
    printf("in seeek....\n");
    if (position > running->fd[fd]->minodePtr->INODE.i_size) {
        return -1;
    }
    int originalPosition = running->fd[fd]->offset;
    running->fd[fd]->offset = position;

    //return originalPosition
    printf("op = %d", originalPosition);
    return originalPosition;
}