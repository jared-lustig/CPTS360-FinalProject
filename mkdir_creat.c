#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

extern int dev;
extern MINODE *root;
extern PROC *running;

int my_mkdir(char *pathname)
{
    char *tester;
    MINODE *mip = root;
    // 1. if (pathname is absolute) dev = root->dev;
    if(*pathname != '/') // if pathname not root
    {
        mip = running->cwd;    
    }
    
    dev = mip->dev;

    /*2. divide pathname into dirname and basename;*/
    char dirname[64], base[64];
    int i;
    for(i = strlen(pathname); pathname[i] != '/' && i != 0; i--);
    if(i == 0) // if you are making directory within root directory
    {
        strcpy(base, pathname);
        strcpy(dirname, "/");
    }
    else // if new directory has path included
    {
        strcpy(base, &pathname[i+1]);
        strncpy(dirname, pathname, i+1);
    }
    
    printf("dirname = %s, base = %s\n", dirname, base);

    //3. // dirname must exist and is a DIR:
    int pino = getino(dirname);
    MINODE *pmip = iget(dev, pino);
    // //check pmip ->INODE is a DIR
;
    if(S_ISDIR(pmip->INODE.i_mode)) // check if DIR
    {
        printf("mkdir: Valid Dirname\n");
    }
    else
    {
        printf("mkdir: Invalid Dirname\n");
        return -1;
    }    

    printf("ino = %d, base = %s", pino, base);

    /*4. // basename must not exist in parent DIR:*/
    if(search(&pmip->INODE, base))
    {
        printf("mkdir: BaseName already exists in directory\n");
        return -1;
    }

    /*5. call kmkdir(pmip, basename) to create a DIR;
    kmkdir() consists of 4 major steps:*/    
    kmkdir(pmip, base, pino);

    //6. increment parent INODE's link_count by 1 and mark pmip dirty;
    pmip->dirty = 1;
    pmip->INODE.i_links_count++;
    iput(pmip);
    
   return 0;
}

int kmkdir(MINODE *pmip, char *base, int pino)
{
    char buf[BLKSIZE];
    //5-1. allocate an INODE and a disk block:
    int ino = ialloc(dev), bno = balloc(dev);
    MINODE *mip = iget(dev,ino); // load INODE into an minode;

    //5-2. initialize mip->INODE as a DIR INODE;
    //pmip->INODE.i_block[0] = bno; //other i_block[ ] are 0;
    //pmip->dirty = 1; //marking as dirty
    //iput(pmip); // write INODE back to disk

	/*Set all of the MINODE and INOE properties*/
	//drwxr-xr-x
	mip->INODE.i_mode 	  = 0x41ED; 
	printf("i_mode set to %x\n", mip->INODE.i_mode);
	mip->INODE.i_uid    	  = running->uid;
	mip->INODE.i_gid  	  = running->gid;
	mip->INODE.i_size	  = 1024;
	mip->INODE.i_links_count = 2; 
	mip->INODE.i_atime       = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
	mip->INODE.i_blocks      = 2;
	mip->INODE.i_block[0]    = bno;

    mip->dirty = 1;
    iput(mip);

    //5-3. make data block 0 of INODE to contain . and .. entries;
    //write to disk block blk

    char temp_buf[1024] = {0};
    char *cp = temp_buf;
    DIR *dp = (DIR *)temp_buf;

    dp->inode = ino;
    dp->name_len = 1;
    dp->rec_len = 12;
    strcpy(dp->name, ".");

    cp += dp->rec_len;
    dp = (DIR *)cp;

    dp->inode = pino;
    dp->name_len = 2;
    dp->rec_len = 1024 - 12;
    strcpy(dp->name, "..");

    put_block(dev, bno, temp_buf);

    //5-4. enter_child(pmip, ino, basename); which enters
    //(ino, basename) as a DIR entry to the parent INODE;
    enter_name(pmip, ino, base);

    return 0;
}


int my_creat(char *pathname)
{
    //This is similar to mkdir() except
    //(1). the INODE.i_mode field is set to Reg file type, permission bits set to 0644 for rw-r--r--, and
    //(2). no data block is allocated for it, so the file size is 0.
    //(3). Do not increment parent INODE's links_count

    MINODE *mip = root;
    // 1. if (pathname is absolute) dev = root->dev;
    if(*pathname != '/') // if pathname not root
    {
        mip = running->cwd;    
    }
    
    dev = mip->dev;

    /*2. divide pathname into dirname and basename;*/
    char dirname[64], base[64];
    int i;
    for(i = strlen(pathname); pathname[i] != '/' && i != 0; i--);
    if(i == 0) // if you are making directory within root directory
    {
        strcpy(base, pathname);
        strcpy(dirname, "/");
    }
    else // if new directory has path included
    {
        strcpy(base, &pathname[i+1]);
        strncpy(dirname, pathname, i+1);
    }
    
    printf("dirname = %s, base = %s\n", dirname, base);

    //3. // dirname must exist and is a DIR:
    int pino = getino(dirname);
    MINODE *pmip = iget(dev, pino);
    // //check pmip ->INODE is a DIR
;
    if(S_ISDIR(pmip->INODE.i_mode)) // check if File
    {
        printf("Creat: Valid Dirname\n");
    }
    else
    {
        printf("Creat: Invalid Dirname\n");
        return -1;
    }    

    printf("ino = %d, base = %s", pino, base);

    /*4. // basename must not exist in parent DIR:*/
    if(search(&pmip->INODE, base))
    {
        printf("mkdir: BaseName already exists in directory\n");
        return -1;
    }

    /*5. call kmkdir(pmip, basename) to create a DIR;
    kmkdir() consists of 4 major steps:*/    
    kcreat(pmip, base, pino);

    //6. increment parent INODE's link_count by 1 and mark pmip dirty;
    pmip->dirty = 1;
    pmip->INODE.i_links_count = 0;
    iput(pmip);
    
   return 0;
}

int kcreat(MINODE *pmip, char *base, int pino)
{
    char buf[BLKSIZE];
    //5-1. allocate an INODE and a disk block:
    int ino = ialloc(dev), bno = balloc(dev);
    MINODE *mip = iget(dev,ino); // load INODE into an minode;

	/*Set all of the MINODE and INOE properties*/
	//
	mip->INODE.i_mode 	  = 0x81A4; 
	printf("i_mode set to %x\n", mip->INODE.i_mode);
	mip->INODE.i_uid    	  = running->uid;
	mip->INODE.i_gid  	  = running->gid;
	mip->INODE.i_size	  = 0; 
    mip->INODE.i_links_count = 1;
	mip->INODE.i_atime       = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
	mip->INODE.i_block[0]    = 0;
    mip->INODE.i_block[0]    = bno;

    mip->dirty = 1;
    iput(mip);

    enter_name(pmip, ino, base);

    return 0;
}