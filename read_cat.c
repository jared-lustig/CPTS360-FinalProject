#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

int read_file()
{
//   Preparations: 
//     ASSUME: file is opened for RD or RW;
//     ask for a fd  and  nbytes to read;
//     verify that fd is indeed opened for RD or RW;
//     return(myread(fd, buf, nbytes));
}

int myread(int fd, char *buf, int nbytes)
{
    //1. 
    int count = 0;
    int lbk, blk;
    int startByte, remain;
    //int offset = OFT.offset;
    //avil = fileSize - OFT's offset // number of bytes still available in file.
    int avil; // Where do I get filesize and OFT offset

    char readbuf[BLKSIZE];
    char *cq = buf;                // cq points at buf[ ]

    MINODE *mip;

    //2. 
    while (nbytes && avil){

      // Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

      //lbk = oftp->offset / BLKSIZE;
      //startByte = oftp->offset % BLKSIZE;
     
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
 
       if (lbk < 12){                     // lbk is a direct block
           blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
       }
       else if (lbk >= 12 && lbk < 256 + 12) { 
            //  indirect blocks 
       }
       else{ 
            //  double indirect blocks
       } 

       /* get the data block into readbuf[BLKSIZE] */
       get_block(mip->dev, blk, readbuf);

       /* copy from startByte to buf[ ], at most remain bytes in this block */
       char *cp = readbuf + startByte;   
       remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

       while (remain > 0){
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             //oftp->offset++;           // advance offset 
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

int mycat()
{
//   cat filename:

//   char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
//   int n;

//   1. int fd = open filename for READ;
//   2. while( n = read(fd, mybuf[1024], 1024)){
//        mybuf[n] = 0;             // as a null terminated string
//        // printf("%s", mybuf);   <=== THIS works but not good
//        spit out chars from mybuf[ ] but handle \n properly;
//    } 
//   3. close(fd);
}