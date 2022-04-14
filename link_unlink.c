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

int my_link() {
    char dirname[64], base[64];
    int i;
    
    
    printf("dirname = %s, base = %s\n", dirname, base);
    int oino = 0;
    int pino = 0;
    MINODE* omip;
    pino = getino(".");
    printf("Got Parent INO, pino = %d\n", pino);
    MINODE* pmip;
    for(i = strlen(pathname); pathname[i] != '/' && i != 0; i--) 
    {
        if(i == 0) // if you are making directory within root directory
        {
            strcpy(base, pathname);
            strcpy(dirname, "/");
        }
        else // if new directory has path included
        {
            strcpy(base, &pathname[i+1]);
            strncpy(dirname, pathname, i+1);
            int pino = getino(dirname);
        }
    }




    pmip = iget(dev, pino);
    printf("iget ran successfully!\n");
    int new = pmip->ino;
    printf("pmip ino = %d\n", new);
    printf("Checking if File exists...\n");
    if (search(pmip, pathname) != 0) {
        if(S_ISDIR(omip->INODE.i_mode)) {
            return;
        }
        oino = getino(pathname);
        omip = iget(dev, oino);

        char* parent = dirname(third); 
        char* child = basename(third);
        pino = getino(parent);
        pmip = iget(dev, pino);
        // creat entry in new parent DIR with same inode number of old_file
        enter_name(pmip, oino, child);
        omip->INODE.i_links_count++; // inc INODE’s links_count by 1
        omip->dirty = 1; // for write back by iput(omip)
        iput(omip);
        iput(pmip);
    }
}

int my_unlink() {
    MINODE* pmip;
    int ino = getino(pathname);
    printf("ino = %d\n pathname = %s\n", ino, pathname);
    MINODE* mip = iget(dev, ino);
    if(S_ISDIR(mip->INODE.i_mode)) {
        printf("Error: Cannon Unlink a Dir.\n");
        return;
    }
    char* parent = dirname(pathname); 
    char* child = basename(pathname);
    printf("getting parent pino\n");
    int pino = getino(parent);
    printf("getting parent mino\n");
    pmip = iget(dev, pino);
    printf("got parent mino!\n");
    rm_child(pmip, child);
    pmip->dirty = 1;
    iput(pmip);
    mip->INODE.i_links_count--;
    if (mip->INODE.i_links_count > 0) {
            mip->dirty = 1; // for write INODE back to disk
    }
    else{ // if links_count = 0: remove filename
        mip->refCount++;
		mip->dirty = 1;
		idalloc(dev, mip->ino);
    }
    iput(mip); // release mip
}