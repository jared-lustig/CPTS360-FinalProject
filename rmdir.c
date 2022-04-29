#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"


/* 1. Gets number of location of directory, sets data to mip
   2. Verifies we are deleteing a directory
   3. Verifies directory is empty before deleting
   4. Gets number of location of parent directory
   5. Gets name of directory
   6. Removes directory by checking first, last, and middle cases
   7. Sets dirty to notify changes, iterate down by 1, put changes, deallocate block number*/
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

   mip->refCount = 1;

   // (3) Dir is empty
   if(mip->INODE.i_links_count > 2)
   {
      printf("DIrectory is not empty, cannot RMDIR.\n");
      iput(mip);
      return -1;
   }

   //(4). /* get parent’s ino and inode */
   int pino = findino(mip, &ino); //get pino from .. entry in INODE.i_block[0]
   MINODE *pmip = iget(mip->dev, pino);   

   char *name;
   //(5). /* get name from parent DIR’s data block
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

/* Remove Child
INPUT: MINODE, name 
DESCRIPTION: get's block, then with a new buffer, sets the dp to traverse through the block
If the currently block name is not 0.
First IF statement checks to see if entry is at the beginning of the block (will have the entry block as rec len)
First IF currently does nothing as this use case is very unlikely as we would need to fill up a block first, as . and .. are usually first entry
Second IF statement checks to see if entry is at the end of the block, will take the rec len (space) and add it to the previous entry
Last IF statment checks for middle case, takes space and adds it to last entry using the while loop to again traverse through the block with lastdp variable name*/
void rm_child(MINODE *pmip, char *name) {
   int i = 0;
   char buf[BLKSIZE], temp[256];
   char *cp;
   DIR *dp;
   DIR *prev;

   printf("searching for ino...\n");
   //int ino = search(pmip, name);
   printf("getting block pmip = %d, pmip->INODE.i_block[i] = %d...\n", pmip->dev, pmip->INODE.i_block[i]);
   get_block(pmip->dev, pmip->INODE.i_block[i], buf);
   dp = (DIR *)buf;
   cp = buf;

   bzero(temp, 256);
   strncpy(temp, dp->name, dp->name_len);
   temp[dp->name_len] = 0;
   while (cp < buf + BLKSIZE){

      if(strncmp(name, temp, dp->name_len) == 0)   
      {

         printf("dp->name = %s\n", dp->name);

         if (dp->rec_len == BLKSIZE) { // The node to be deleted is the only entry
            //printf("2...\n");
            // will always have "." and "..". Only happen if we fill up an entry block and make a new block
            printf("Only Case...\n");
         }
         else if (cp + dp->rec_len >= buf + BLKSIZE){ // The node to be deleted is the last entry
            printf("Last Case...\n");
            prev->rec_len += dp->rec_len; // Adds space from deleted node to the node just before itself.
            put_block(pmip->dev, pmip->INODE.i_block[i], buf);

            return;
         }
         else { // The node to be deleted if it is the middle node of many
            printf("Middle Case...\n");
            int size = buf + BLKSIZE - (cp + dp->rec_len); // size is set to the blksize - middle nodes size
            int space = dp->rec_len;

            char *lastcp = buf;
            DIR *lastdp = (DIR *)buf; // temp directory to traverse to the end

            while (lastcp + lastdp->rec_len < buf + BLKSIZE){  // traverses to the end of the block
               lastcp += lastdp->rec_len;
               lastdp = (DIR *)lastcp;
            }

            lastdp->rec_len += space; // Adds space from deleted node to the node from the middle of the block

            memcpy(cp, (cp + dp->rec_len), size); // sets cp buf to size of the rec_len

            put_block(pmip->dev, pmip->INODE.i_block[i], buf); // saves changes to block
            return;
         }
      }

      prev = (DIR *)cp;
      cp += dp->rec_len;
      dp = (DIR *)cp;

      // resets the temp buffer, and continues to traverse looking for the middle node 
      bzero(temp, 256);
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
   }
}