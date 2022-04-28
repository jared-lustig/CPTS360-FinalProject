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

/*Description: 
1. Get the parent inode of file to be linked from.
2. Get the child inode of file to be linked from.
3. Get the parent inode of file to be linked to.
4. Get the child inode of file to be linked to.
5. Run enter name to create a file to be linked to with inputs: parent mip, ino of file to be linked from, base name of file to be linked to.
6. increase link count and dirty on set file to be linked from.
7. iput both.
*/
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
    tokenize(pathname); // tokenize file to be linked from.
    if (n > 1) {
        for (i = 0; i <= n-2; i++) { // get parent from name - 1(base)
            pmip = iget(dev, search(pmip, name[i])); // Iterates through "name" while searching for each name in each directory until the directory name is found.
        }
    }
    pino = pmip->ino;
    printf("Got Parent INO, pino = %d\n", pino);
    printf("name = %s, n = %d\n", name[n-1], n);

    if (search(pmip, name[n-1]) != 0) { // If the basename exists.
        oino = getino(name[n-1]);
        omip = iget(dev, oino);
        if(S_ISDIR(omip->INODE.i_mode)) { // if its a directory. Fail.
            return;
        }

        tokenize(third); // tokenize the file to be linked to.
        printf("name = %s, n = %d\n", name[n-1], n);
        pmip = running->cwd;
        if (pathname == '/') {
            pmip = 2;
        }
        if (n > 1) {
            for (i = 0; i <= n-2; i++) { // get parent from name - 1(base)
                pmip = iget(dev, search(pmip, name[i])); // // Iterates through "name" while searching for each name in each directory until the directory name is found.
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

/*Description: 
1. Get mip of file to be unlinked
2. Get parent of file to be unlinked
3. Get name of file to be unlinked
4. rm name of child from parent
5. Decrease link counts of file to be unlinked from
6. When a file’s links_count reaches 0, the file is truly removed by deallocating its data blocks and inode.
7. iput
*/
int my_unlink() {
    MINODE* pmip;
    int ino = getino(pathname); // Get ino of file to be unlinked from
    printf("ino = %d\n pathname = %s\n", ino, pathname);
    MINODE* mip = iget(dev, ino); // get mip of file to be unlinked from
    if(S_ISDIR(mip->INODE.i_mode))  // make sure file to be unlinked from is not directory
    {
        printf("Error: Cannon Unlink a Dir.\n");
        return;
    }
    char* parent = dirname(pathname); // get name of parent of file to be unlinked
    char* child = basename(pathname); // get name of child to be unlinked
    printf("getting parent pino\n");
    int pino = getino(parent); // get parent ino of file to be unlinked
    printf("getting parent mino\n");
    pmip = iget(dev, pino); // get parent mip of file to be unlinked
    printf("got parent mino!\n");
    rm_child(pmip, child); // rm the file to be unlinked by inputing name of child and parent mip
    pmip->dirty = 1; // set parent mip to dirty
    iput(pmip); // iput parent
    mip->INODE.i_links_count--; // Decrease link counts of file to be unlinked from
    if (mip->INODE.i_links_count > 0) 
    {
            mip->dirty = 1; // for write INODE back to disk
    }
    else
    { // When a file’s links_count reaches 0, the file is truly removed by deallocating its data blocks and inode.
        mip->refCount++;
        mip->dirty = 1;
        idalloc(dev, mip->ino); //deallocated ino of file to be unlinked from
    }
    iput(mip); // release mip
}