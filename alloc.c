#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

extern int imap;
extern int ninodes;
extern int bmap;

extern MINODE *iget();

int tst_bit(char *buf, int bit) // in Chapter 11.3.1
{
    return buf[bit/8] & (1<<(bit%8));
}

int set_bit(char *buf, int bit) // in Chapter 11.3.1
{
    return buf[bit/8] |= (1<<(bit%8));
}

int clr_bit(char *buf, int bit)
{
    return buf[bit/8] &= ~(1<<(bit%8));
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){ // use ninodes from SUPER block
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
	put_block(dev, imap, buf);

	decFreeInodes(dev);

	printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

// WRITE YOUR OWN balloc(dev) function, which allocates a FREE disk block number

int balloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){ // use ninodes from SUPER block
    if (tst_bit(buf, i)==0){

        set_bit(buf, i);
	    put_block(dev, bmap, buf);
	    decFreeInodes(dev);

	    printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}