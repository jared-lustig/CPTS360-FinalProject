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
 
 /* write INODE back to disk */
 /**************** NOTE ******************************
  For mountroot, we never MODIFY any loaded INODE
                 so no need to write it back
  FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

  Write YOUR code here to write INODE back to disk
 *****************************************************/
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

void new_directory(MINODE *pmip, int ino, int bnum, char *buf) // step 5.3 in kmkdir
{
   char *cp;
   DIR *dp = (DIR *)buf;
   dp->inode = ino;
   dp->name_len = 1;
   dp->rec_len = 12;
   strncpy(dp->name, ".", 1);
   cp = buf + 12;
   dp = (DIR *)cp;
   dp->inode = pmip->ino;
   dp->name_len = 2;
   dp->rec_len = BLKSIZE - 12;
   strncpy(dp->name, "..", 2);
   put_block(pmip->dev, bnum, buf);
}

enter_name(MINODE *pmip, int oino, char* child) {
   printf("Currently inside enter_name...\n");
   printf("oino = %d\n", oino);
   char *cp;
    char buf[1024];
    int i = 0;
    printf("Variables Defined!\n");
   //(1). Get parent’s data block into a buf[ ];
   printf("Creating Inode Pointer...\n");
   INODE *ip;
   ip = &(pmip->INODE);
   get_block(dev, ip->i_block[0], buf);
    if (ip->i_block[i]==0) {
        printf("Error, No Memory in Data Block\n");
        //Allocate a new data block; increment parent size by BLKSIZE;
        //Enter new entry as the first entry in the new data block with rec_len¼BLKSIZE.
    }
    else{
        //(2). In a data block of the parent directory, each dir_entry has an ideal length
        printf("Reading Parent Inode...\n");
        get_block(pmip->dev, pmip->INODE.i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;
        printf("Traversing Block Until Last Entry...\n");
        while (cp + dp->rec_len < buf + BLKSIZE){
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        // dp NOW points at last entry in block

        int ideal_length = 4*( (11 + dp->name_len)/4 );
        int remain = dp->rec_len - ideal_length;
        int need_length = 4*( (11 + strlen(child))/4 );
        printf("ideal length = %d\nremain = %d\nneed length = %d\ndp->rec_len = %d\nstrlen(child) = %d\noino = %d\n",ideal_length, remain, need_length, dp->rec_len, strlen(child), oino);
        if (remain >= need_length){
            printf("Creating New Entry...\n");
            printf("Trimming Last Entry...\n");
            dp->rec_len = ideal_length;
            printf("Moving to New Last Entry...\n");
            cp += dp->rec_len;
            dp = (DIR *)cp;
            printf("Writing New Entry...\n");
            printf("rec_len = %d, needlen = %d\n", dp->rec_len, need_length);
            dp->inode = oino;
            dp->rec_len = remain;
            dp->name_len = strlen(child);
            strncpy(dp->name, child, dp->name_len);
            printf("rec_len = %d, needlen = %d\n", dp->rec_len, need_length);
            //enter the new entry as the LAST entry and 
            //trim the previous entry rec_len to its ideal_length;
        }
    }
    //Write data block to disk;
    put_block(pmip->dev, pmip->INODE.i_block[i], buf);
}

int ideal_len(int n)
{
   return 4 * ((8 + n + 3) / 4);
}