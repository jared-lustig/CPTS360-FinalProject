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

/*Description: 
1. Check if old file exists and new file does not
2. Creat new_file; change new_file to LNK type;
3. Store old_file name in newfile’s INODE.i_block[ ] area.
4. change parent directory to dirty
5. iput
*/
int my_symlink() {
    // Check if old file exists and new file does not
    MINODE* omip; 
    int oino = getino(pathname); // get old file
    printf("old ino = %d\n pathname = %s\n", oino, pathname);
    omip = iget(dev, oino); // get old mip
    if(S_ISDIR(omip->INODE.i_mode)) { // make sure old file is not directory
        printf("Error: Cannot symlink a Dir.\n");
        return;
    }
    // Creat new_file; change new_file to LNK type;
    my_creat(third); // create new file
    int ino = getino(third); //get ino of new file
    printf("ino = %d\npathname = %s\n", ino, third);
    MINODE* mip; 
    mip = iget(dev, ino); // get mip of new file
    mip->INODE.i_mode = 0xA000; // set new file to LNK TYPE
    // Store old_file name in newfile’s INODE.i_block[ ] area.
    DIR *dp;
    char buf[BLKSIZE];
    get_block(dev, mip->INODE.i_block[0], buf); // get the iblock of new file
    dp = (DIR *)buf; // get the directory entry of new file
    
    DIR *odp;
    char obuf[BLKSIZE];
    get_block(dev, omip->INODE.i_block[0], obuf);
    odp = (DIR *)obuf; // get directory entry of old file
    strcpy(dp->name, odp->name); // Set file size to length of old_file name
    mip->dirty = 1; // Mark new_file’s minode dirty;
    iput(mip); // Mark new_file parent minode dirty;
    char* parent = dirname(pathname);  // get parent pathanme
    int pino = getino(parent); // get parent ino
    MINODE* pmip = iget(dev, pino); // get parent mip
    pmip->dirty = 1; // get parent to dirty
    iput(pmip); // iput parent
}

/*Description: 
1. Get mip of file link
2. Make sure file is a link
3. Get the directory entry of file link
4. Print out file size
*/
int my_readlink() {
    MINODE* mip;
    int ino = getino(pathname); // get ino of file link
    printf("old ino = %d\npathname = %s\n", ino, pathname);
    mip = iget(dev, ino); // get mip of file link
    if(!S_ISLNK(mip->INODE.i_mode)) { // make sure file is a link
        printf("Error: Not a link.\n");
        return;
    }
    DIR *dp;
    char buf[BLKSIZE];
    get_block(dev, mip->INODE.i_block[0], buf); // get the iblock into buffer
    dp = (DIR *)buf; // get the directory entry of file link
    printf("\nFile Size = %d\n", dp->name_len); // print out file size
}