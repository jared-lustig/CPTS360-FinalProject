/*********** util.c file ****************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
  name[n] = 0;
  
  for (i= 0; i<n; i++)
    printf("%s  ", name[i]);
  printf("\n");
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, offset;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount && mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino to buf    
       blk    = (ino-1)/8 + iblk;
       offset = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + offset;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
}

void iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 if (mip==0) 
     return;

 mip->refCount--;
 
 if (mip->refCount > 0) return;
 if (!mip->dirty)       return;
 
 block = (mip->ino - 1) / 8 + iblk;
 offset = (mip->ino -1) % 8;

 get_block(mip->dev, block, buf);
 ip = (INODE *)buf + offset;
 *ip = mip->INODE;
 put_block(mip->dev, block, buf);
 mip->refCount = 0;
} 

int search(MINODE *mip, char *name)
{
   int i; 
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name,mip->dev,mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   printf("  ino   rlen  nlen  name\n");

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     printf("%4d  %4d  %4d    %s\n", 
           dp->inode, dp->rec_len, dp->name_len, dp->name);
     if (strcmp(temp, name)==0){
        printf("found %s : ino = %d\n", temp, dp->inode);
        return dp->inode;
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;
}

int getino(char *pathname)
{
  int i, ino, blk, offset;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;
  
  // starting mip = root OR CWD
  if (pathname[0]=='/')
     mip = root;
  else
     mip = running->cwd;

  mip->refCount++;         // because we iput(mip) later
  
  tokenize(pathname);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
 
      if (mip->mounted == 1)
      {
         dev = 4;
         iget(dev, mip->mptr);
         printf("Does this shit fuck up\n");
      }

      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);
      mip = iget(dev, ino);
   }

   iput(mip);
   return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[ ]) 
{
  // WRITE YOUR code here
  // search parent's data block for myino; SAME as search() but by myino
  // copy its name STRING to myname[ ]

   int i;
   char *cp, temp[256], sbuf[BLKSIZE];
   for (i=0; i<12; i++){ // search DIR direct blocks only
      if (parent->INODE.i_block[i] == 0){
         return 0;
      }
      get_block(dev, parent->INODE.i_block[i], sbuf);
      //printf("Search = %s\n", name);
      dp = (DIR *)sbuf;
      cp = sbuf;
      
      while (cp < sbuf + BLKSIZE){
         strncpy(temp, dp->name, dp->name_len);
         temp[dp->name_len] = 0;

         //printf("name = %s, temp = %s\n", dp->name, temp);

         if(dp->inode == myino)
         {
            strcpy(myname, temp);
            return dp->inode;
         }
            
         cp += dp->rec_len;
         dp = (DIR *)cp;
      }
   }
   return 0;
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
  // mip points at a DIR minode
  // WRITE your code here: myino = ino of .  return ino of ..
  // all in i_block[0] of this DIR INODE.

   char *cp, temp[256], sbuf[BLKSIZE];

   get_block(dev, mip->INODE.i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;

   *myino = dp->inode;

   cp += dp->rec_len;
   dp = (DIR *)cp;

   return dp->inode;

}

void new_directory(int ino, int bnum, int dev) // step 5.3 in kmkdir
{
   char temp_buf[1024] = {0};
   char *cp = temp_buf;
   DIR *dp = (DIR *)temp_buf;

   dp->inode = ino;
   dp->name_len = 1;
   dp->rec_len = 12;
   strncpy(dp->name, ".", 1);

   cp += dp->rec_len;
   dp = (DIR *)cp;

   dp->inode = dev;
   dp->name_len = 2;
   dp->rec_len = 1024 - 12;
   strncpy(dp->name, "..", 2);

   put_block(dev, bnum, temp_buf);
}

int ideal_len(int n)
{
   return 4 * ((8 + n + 3) / 4);
}

int truncate(MINODE *mip)
{
   int i;
   char buf[BLKSIZE];
   // Direct Release
   for (i = 0; i < 12; i++)
   {
      get_block(mip->dev, mip->INODE.i_block[0], buf);
      memset(buf, 0, BLKSIZE);
      put_block(mip->dev, mip->INODE.i_block[i], buf);
      memset(buf, 0, BLKSIZE);
   }
   mip->dirty = 1;
   mip->INODE.i_size = 0;
   //iput(mip);

   // Sucess return
   return 0;
}

int pfd() {
   int i = 0;
   printf("open fd's:\n");
   while (running->fd[i] && i < 10) {
      printf("fd = %d\n", i);
      i++;
   }
}