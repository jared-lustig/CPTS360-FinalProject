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

extern char third[128], pathname[128];

int my_symlink() {
    // Check if old file exists and new file does not
    MINODE* omip;
    int oino = getino(pathname);
    printf("old ino = %d\n pathname = %s\n", oino, pathname);
    omip = iget(dev, oino);
    if(S_ISDIR(omip->INODE.i_mode)) {
        printf("Error: Cannot symlink a Dir.\n");
        return;
    }
    // Creat new_file; change new_file to LNK type;
    my_creat(third);
    int ino = getino(third);
    printf("ino = %d\npathname = %s\n", ino, third);
    MINODE* mip;
    mip = iget(dev, ino);
    mip->INODE.i_mode = 0xA000; //LNK TYPE
    // Store old_file name in newfile’s INODE.i_block[ ] area.
    DIR *dp;
    char buf[BLKSIZE];
    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    
    DIR *odp;
    char obuf[BLKSIZE];
    get_block(dev, omip->INODE.i_block[0], obuf);
    odp = (DIR *)obuf;
    // Set file size to length of old_file name
    strcpy(dp->name, odp->name);
    // Mark new_file’s minode dirty;
    mip->dirty = 1;
    iput(mip);
    // Mark new_file parent minode dirty;
    char* parent = dirname(pathname); 
    int pino = getino(parent);
    MINODE* pmip = iget(dev, pino);
    pmip->dirty = 1;
    iput(pmip);
}

int my_readlink() {
    MINODE* mip;
    int ino = getino(pathname);
    printf("old ino = %d\npathname = %s\n", ino, pathname);
    mip = iget(dev, ino);
    if(!S_ISLNK(mip->INODE.i_mode)) {
        printf("Error: Not a link.\n");
        return;
    }
    DIR *dp;
    char buf[BLKSIZE];
    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    printf("\nFile Size = %d\n", dp->name_len);
}