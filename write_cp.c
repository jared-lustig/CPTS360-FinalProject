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
    fd = getino(pathname);

    //   2. verify fd is indeed opened for WR or RW or APPEND mode
    if (fd != 1 || fd != 3)
    {
        printf("File is not open for RW or WR\n");
        return 0;
    }
    //   3. copy the text string into a buf[] and get its length as nbytes.

    wbuf[strlen(wbuf) - 1] = 0;

    return(mywrite(fd, wbuf, BLKSIZE));
}

int mywrite(int fd, char buf[ ], int nbytes) 
{
    int lbk, blk, dblk;
    int startByte;
    int offset, remain;
   // int count = 0;


    OFT *oftp = running->fd[fd];

    MINODE *mip = oftp->minodePtr;
    INODE *ip = &mip;

    int ib[BLKSIZE / sizeof(int)], id[BLKSIZE / sizeof(int)];

    int buf13[256]; // buf13[] = |D0|D1|D2|...|

    char ibuf[BLKSIZE], wbuf[BLKSIZE], dbuf[BLKSIZE];

    char *cp = buf, *cq = buf;

    while (nbytes > 0 ){
        bzero(wbuf, BLKSIZE);
        //compute LOGICAL BLOCK (lbk) and the startByte in that lbk:

        lbk       = oftp->offset / BLKSIZE;
        printf("\n\nlbk = %d\n\n", lbk);
        startByte = oftp->offset % BLKSIZE;

        // I only show how to write DIRECT data blocks, you figure out how to 
        // write indirect and double-indirect blocks.

        if (lbk < 12){                         // direct block
            if (ip->i_block[lbk] == 0){   // if no data block yet
                mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
                get_block(mip->dev, mip->INODE.i_block[lbk], wbuf);
                memset(wbuf, 0, BLKSIZE);
                put_block(mip->dev, mip->INODE.i_block[lbk], wbuf);
            }
            blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
            printf("\nblk inside direct block = %d\n", blk);
        }
        else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
              // HELP INFO:
              printf("writing to indirect block\n");
              if (ip->i_block[12] == 0){
                  //allocate a block for it;
                  //zero out the block on disk !!!!
                  mip->INODE.i_block[12] = balloc(mip->dev);
                  get_block(mip->dev, mip->INODE.i_block[12], ibuf);
                  memset(ibuf, 0, BLKSIZE);
                  put_block(mip->dev, mip->INODE.i_block[12], ibuf);
              }
              memset(ib, 0, BLKSIZE / sizeof(int));
              get_block(mip->dev, mip->INODE.i_block[12], (char *)ib);
              //get i_block[12] into an int ibuf[BLKSIZE];
              blk = ib[lbk - 12];
              if (blk==0){
                 //allocate a disk block;
                 //record it in i_block[12];
                 blk = balloc(mip->dev);
                 get_block(mip->dev, blk, wbuf);
                 bzero(wbuf, BLKSIZE);
                 put_block(mip->dev, blk, wbuf);
                 ib[lbk - 12] = blk;
                 put_block(mip->dev, mip->INODE.i_block[12], (char *)ib);
              }
              printf("\nblk inside indirect direct block = %d\n", blk);
              
              //.......
        }
        else{
            // double indirect blocks */
            printf("writing to double indirect block\n");
            memset(ibuf, 0,  256);
            get_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf);
            lbk -= (12 + 256);
            dblk = dbuf[lbk / 256]; 
            get_block(mip->dev, dblk, (char *)dbuf);
            blk = dbuf[lbk % 256];
        }
     memset(wbuf, 0, BLKSIZE);
     /* all cases come to here : write to the data block */
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
     char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

    //make sure remain / startbyte is set correctly.

     while (remain > 0){               // write as much as remain allows  
           *cp++ = *cq++;              // cq points at buf[ ]
           //count ++;
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
    int gd = open_file(destination,1);

    //    NOTE:In the project, you may have to creat the dst file first, then open it 
    //         for WR, OR  if open fails due to no file yet, creat it and then open it
    //         for WR.
    memset(buf, 0, BLKSIZE);
    while( n = read_file(fd, buf, BLKSIZE) ){;     
        printf("my write buf = %s", buf); // buffer might be messed up 
        mywrite(gd, buf, n);  // notice the n in write()
        memset(buf, 0, BLKSIZE); // setting all index of array to a certain value
    }

    close(fd);
    close(gd);
}

int my_mv(char* src, char* dest) {
    //mv src dest:
    printf("running mv...\n");
    //1. verify src exists; get its INODE in ==> you already know its dev
    int ino_src = getino(src); 
    if (ino_src == 0){ // if file does not exist
        printf("ERROR: Src does not exist.\n");
        return -1;
    }
    MINODE *mip_src = iget(dev, ino_src);
    //2. check wheter src is on the same dev as src
    int ino_dest = getino(dest); 
    if (ino_dest == 0){ // if file does not exist
        printf("ERROR: Dest does not exist.\n");
        return -1;
    }
    MINODE *mip_dest = iget(dev, ino_dest);

    if (mip_src->dev == mip_dest->dev) {
        //CASE 1: same dev:
        //3. Hard link dst with src (i.e. same INODE number)
        printf("Same device, hard linking...\n");
        my_link(src, dest);
        //4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
               //link count by 1).
        printf("Unlinking src...\n");
        my_unlink();
        return 0;
    }
            //CASE 2: not the same dev:
    //3. cp src to dst
//my_cp(mip_src, mip_dest);
    //4. unlink src
    printf("Unlinking src...\n");
    my_unlink();
}