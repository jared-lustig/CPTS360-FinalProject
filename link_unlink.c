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

extern char* name[];
extern int n;
extern char third[128], pathname[128];

int my_link() {
    int i;
    int oino = 0;
    int pino = 0;
    MINODE* omip;
    MINODE* pmip;
    pmip = running->cwd;
    if (pathname == '/') {
        pmip = 2;
    }
    tokenize(pathname);
    if (n > 1) {
        for (i = 0; i <= n-2; i++) { // get parent from name - 1(base)
            pmip = iget(dev, search(pmip, name[i]));
        }
    }
    pino = pmip->ino;
    printf("Got Parent INO, pino = %d\n", pino);
    printf("name = %s, n = %d\n", name[n-1], n);

    if (search(pmip, name[n-1]) != 0) {
        oino = getino(name[n-1]);
        omip = iget(dev, oino);
        if(S_ISDIR(omip->INODE.i_mode)) {
            return;
        }

        tokenize(third);
        printf("name = %s, n = %d\n", name[n-1], n);
        pmip = running->cwd;
        if (pathname == '/') {
            pmip = 2;
        }
        if (n > 1) {
            for (i = 0; i <= n-2; i++) { // get parent from name - 1(base)
                pmip = iget(dev, search(pmip, name[i]));
            }
        }
        pino = pmip->ino;
        printf("Got Parent INO, pino = %d\n", pmip->ino);
        // creat entry in new parent DIR with same inode number of old_file
        enter_name(pmip, oino, name[n-1]);
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
    if(S_ISDIR(mip->INODE.i_mode)) 
    {
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
    if (mip->INODE.i_links_count > 0) 
    {
            mip->dirty = 1; // for write INODE back to disk
    }
    else
    { // if links_count = 0: remove filename
        mip->refCount++;
        mip->dirty = 1;
        idalloc(dev, mip->ino);
    }
    iput(mip); // release mip
}