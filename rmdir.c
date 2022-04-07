#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

int my_rmdir(char *pathname)
{
   //(1). get in-memory INODE of pathname:
   int ino = getino(pathname);
   MINODE *mip = iget(dev, ino);

   //(2). verify INODE is a DIR (by INODE.i_mode field);
   //minode is not BUSY (refCount = 1);
   //verify DIR is empty (traverse data blocks for number of entries = 2);
   if(S_ISDIR(mip->INODE.i_mode)) // check if DIR
   {
      printf("rmdir: Valid Directory\n");
   }
   else
   {
      printf("mkdir: Invalid Directory\n");
      return -1;
   } 

   //If not busy?
   // if(mip->refCount > 1)
   // {
   //    printf("refCount is busy\n");
   //    iput(mip);
   //    return -1;
   // }

   mip->refCount = 1;

   //Dir is empty
   if(mip->INODE.i_links_count > 2)
   {
      printf("DIrectory is not empty, cannot RMDIR.\n");
      iput(mip);
      return -1;
   }

   //(3). /* get parent’s ino and inode */
   int pino = findino(mip, &ino); //get pino from .. entry in INODE.i_block[0]
   MINODE *pmip = iget(mip->dev, pino);   

   char *name;
   //(4). /* get name from parent DIR’s data block
   findmyname(pmip, ino, name); //find name from parent DIR338
   
   printf("name - %s\n", name);

   //(5).
   //EXT2 File System
   //remove name from parent directory */
   rm_child(pmip, name);

   //(6). dec parent links_count by 1; mark parent pimp dirty;
   pmip->dirty = 1;
   pmip->INODE.i_links_count--;
   iput(pmip);

   //(7). /* deallocate its data blocks and inode */
   bdalloc(mip->dev, mip->INODE.i_block[0]);
   idalloc(mip->dev, mip->ino);
   iput(mip);
}