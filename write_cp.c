#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

int write_file(char* pathname)
{
    //   1. Preprations:
    //      ask for a fd   and   a text string to write;

    // cp src dest:
    char dirname[64], base[64];
    int i;
    for(i = strlen(pathname); pathname[i] != '/' && i != 0; i--);
    if(i == 0) // if you are making directory within root directory
    {
        strcpy(base, pathname);
        strcpy(dirname, "/");
    }
    else // if new directory has path included
    {
        strcpy(base, &pathname[i+1]);
        strncpy(dirname, pathname, i+1);
    }
    
    printf("dirname = %s, base = %s\n", dirname, base);

    int fd;
    char wbuf[BLKSIZE];
    fd = atoi(base);

    //   2. verify fd is indeed opened for WR or RW or APPEND mode
    if (fd != 1 || fd != 3)
    {
        printf("File is not open for RW or WR\n");
        return 0;
    }
    //   3. copy the text string into a buf[] and get its length as nbytes.

    fgets(wbuf, BLKSIZE, stdin);
    wbuf[strlen(wbuf) - 1] = 0;

    return(mywrite(fd, wbuf, strlen(wbuf)));
}

int mywrite(int fd, char buf[ ], int nbytes) 
{
    int lbk, blk, dblk;
    int startByte;
    int offset, remain;


    OFT *oftp = running->fd[fd];

    MINODE *mip = oftp->minodePtr;
    INODE *ip = &mip;

    char ibuf[BLKSIZE], wbuf[BLKSIZE], dbuf[BLKSIZE];

    char *cp = buf, *cq = buf;

    while (nbytes > 0 ){

        //compute LOGICAL BLOCK (lbk) and the startByte in that lbk:

        lbk       = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        // I only show how to write DIRECT data blocks, you figure out how to 
        // write indirect and double-indirect blocks.

        if (lbk < 12){                         // direct block
            if (ip->i_block[lbk] == 0){   // if no data block yet
                mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
            }
            blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
        }
        else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
              // HELP INFO:
              printf("writing to indirect block\n");
              if (ip->i_block[12] == 0){
                  //allocate a block for it;
                  //zero out the block on disk !!!!
                  mip->INODE.i_block[12] = balloc(mip->dev);
                  //memset(ibuf, 0, 256);
              }
              //get i_block[12] into an int ibuf[256];
              blk = ibuf[lbk - 12];
              if (blk==0){
                 //allocate a disk block;
                 //record it in i_block[12];
                 mip->INODE.i_block[lbk] = balloc(mip->dev);
                 ibuf[lbk - 12] = mip->INODE.i_block[lbk];

              }
              //.......
        }
        else{
            // double indirect blocks */
            printf("writing to double indirect block\n");
            //memset(ibuf, 0,  256);
            get_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf);
            lbk -= (12 + 256);
            dblk = dbuf[lbk / 256];
            get_block(mip->dev, dblk, (char *)dbuf);
            blk = dbuf[lbk % 256];
        }

     /* all cases come to here : write to the data block */
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
     char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

     while (remain > 0){               // write as much as remain allows  
           *cp++ = *cq++;              // cq points at buf[ ]
           nbytes--; remain--;         // dec counts
           oftp->offset++;             // advance offset
           if (offset > mip->INODE.i_size)  // especially for RW|APPEND mode
               mip->INODE.i_size++;    // inc file size (if offset > fileSize)
           if (nbytes <= 0) break;     // if already nbytes, break
     }
     put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
     
     // loop back to outer while to write more .... until nbytes are written
  }

  mip->dirty = 1;       // mark mip dirty for iput() 
  printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
  return nbytes;
}

int mycp(char* pathname, char* destination)
{
    // cp src dest:
    char dirname[64], base[64];
    int i;
    for(i = strlen(pathname); pathname[i] != '/' && i != 0; i--);
    if(i == 0) // if you are making directory within root directory
    {
        strcpy(base, pathname);
        strcpy(dirname, "/");
    }
    else // if new directory has path included
    {
        strcpy(base, &pathname[i+1]);
        strncpy(dirname, pathname, i+1);
    }
    
    printf("source = %s, destination = %s\n", base, destination);

    char* buf[BLKSIZE];
    int n;

    // 1. fd = open src for READ;
    int fd = open_file(base, 0);

    // 2. gd = open dst for WR|CREAT; 
    int gd = open_file(destination,2);

    //    NOTE:In the project, you may have to creat the dst file first, then open it 
    //         for WR, OR  if open fails due to no file yet, creat it and then open it
    //         for WR.

    while( n = read(fd, buf, BLKSIZE) ){
        mywrite(gd, buf, n);  // notice the n in write()
    }

    close(fd);
    close(gd);
}