#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

int read_file(int fd, char *buf, int nbytes)
{
//   Preparations: 
//     ASSUME: file is opened for RD or RW;
//     ask for a fd  and  nbytes to read;
//     verify that fd is indeed opened for RD or RW;
//     return(myread(fd, buf, nbytes));
    
    //If fd is empty, set the buffer to 0 and retun 0
    if (fd == 1 || fd == 3)
    {
        printf("File is not open for RD or RW\n");
        return 0;
    }

    // Else we know its in read
    return (myread(fd, buf, nbytes));
}

int myread(int fd, char *buf, int nbytes)
{
    //1. 
    OFT *oftp = running->fd[fd];

    int count = 0;
    int lbk, blk;
    int startByte, remain;
    //printf("oftp->minodePTR->INODE.i_size = %d\n", oftp->minodePtr->INODE.i_size);
    //int offset = oft->offset;    
    //avil = fileSize - OFT's offset // number of bytes still available in file.
    int avil = oftp->minodePtr->INODE.i_size - oftp->offset; // Where do I get filesize and OFT offset
    

    char readbuf[BLKSIZE];
    char *cq = buf;                // cq points at buf[ ]

    //2. 
    while (nbytes && avil){

      // Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

      lbk = oftp->offset / BLKSIZE;
      startByte = oftp->offset % BLKSIZE;
     
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
       
 
       if (lbk < 12){                     // lbk is a direct block
           blk = oftp->minodePtr->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
       }
       else if (lbk >= 12 && lbk < 256 + 12) { 
            //  indirect blocks
            char ibuf[256];
            
            get_block(oftp->minodePtr->dev, oftp->minodePtr->INODE.i_block[12], (char *)ibuf);
            printf("lbk = %d IDblk = %d blk = %d\n", lbk, oftp->minodePtr->INODE.i_block[12], blk);

            //getchar();
       }
       else{ 
            //  double indirect blocks

            lbk -= (12 + 256);

            blk = (oftp->minodePtr->ino - 1) / BLKSIZE;
            int offset = (oftp->minodePtr->ino - 1) % BLKSIZE;

            printf("double indirect block not finished\n");
        } 

       /* get the data block into readbuf[BLKSIZE] */
       get_block(oftp->minodePtr->dev, blk, readbuf);
       
       /* copy from startByte to buf[ ], at most remain bytes in this block */
       char *cp = readbuf + startByte;   
       remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
        
       while (remain > 0){
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             oftp->offset++;           // advance offset 
             count++;                  // inc count as number of bytes read
             avil--; nbytes--;  remain--;
             if (nbytes <= 0 || avil <= 0) 
                 break;
        

       }
 
       // if one data block is not enough, loop back to OUTER while for more ...

    }
   printf("myread: read %d char from file descriptor %d\n", count, fd);  
   return count;   // count is the actual number of bytes read
}

int mycat(char *pathname)
{
//   cat filename:
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

    char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
    int n;

    //   1. 
    int fd = open_file(base, 0); // currently returns -1, guessing it is reading from current directory rather than mkdisk, may need actual open for it to work.

    printf("fd = %d\n", fd);
    //int fd = myopen(base, "0");
    //   2. 

    while( n = read_file(fd, mybuf, BLKSIZE)){
       mybuf[n] = 0;             // as a null terminated string
       printf("%s", mybuf);   //<=== THIS works but not good
       //spit out chars from mybuf[ ] but handle \n properly;
    } 
//   3. close(fd);
    close(fd);
    return 0;
}