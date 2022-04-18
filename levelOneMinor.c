#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>

#include "functions.h"

struct my_st {
    int st_dev;         
    int st_ino;         
    int st_mode;        
    int st_nlink;  
    int st_size;   
    int st_uid;         
    int st_gid;                   
    int st_blocks;      
    struct timespec st_atim;   
    struct timespec st_ctim;  
#define st_atime st_atim.tv_sec
#define st_ctime st_ctim.tv_sec
};


int my_stat() {
    struct my_st myst = {};
    MINODE* mip;
    int ino = getino(pathname);
    printf("old ino = %d\npathname = %s\n", ino, pathname);
    mip = iget(dev, ino);
    //copy dev, ino to myst.st_dev, myst.st_ino;
    myst.st_ino = mip->ino;
    myst.st_dev = mip->dev;
    //copy mip->INODE fields to myst fields;
    myst.st_mode= mip->INODE.i_mode;
    myst.st_uid= mip->INODE.i_uid;
    myst.st_size= mip->INODE.i_size;
    myst.st_gid= mip->INODE.i_gid;
    myst.st_nlink= mip->INODE.i_links_count;
    myst.st_blocks= mip->INODE.i_blocks;

    iput(mip);

    printf("Dev = %d\n", myst.st_dev);
    printf("Ino = %d\n", myst.st_ino);
    printf("Mode = %d\n", myst.st_mode);
    printf("Hard links = %d\n", myst.st_nlink);
    printf("Uid = %d\n", myst.st_uid);
    printf("Gid = %d\n", myst.st_gid);
    printf("Size = %d\n", myst.st_size);
    printf("Block Count = %d\n", myst.st_blocks);
}

int my_chmod(char *pathname, char *mode)
{
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

    int located_ino = search(&pmip->INODE, base);

    if(located_ino ==0 )
    {
        printf("chmod: could not find file to edit permissions\n");
        iput(pmip);
        return -1;
    }

    pmip = iget(dev, located_ino);

    if (!S_ISREG(pmip->INODE.i_mode))
    {
        printf("Cannot edit permissions to a directory\n");
        iput(pmip);
        return -1;
    }

    char newmode[64];
    if(mode[0] == '0')
    {
        strcpy(newmode, "10");strcat(newmode, mode);
    }
    else{
        strcpy(newmode, "100");strcat(newmode, mode);
    }

    // printf("0644 mode = %d\n", DecToOctal(newmode));
    // printf("pmip->mode = %d\n", pmip->INODE.i_mode);

    pmip->INODE.i_mode = DecToOctal(newmode);

    pmip->dirty = 1;
    iput(pmip);
}

int my_utime(char *pathname)
{
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

    int located_ino = search(&pmip->INODE, base);

    if(located_ino ==0 )
    {
        printf("utime: could not find file to edit utime\n");
        iput(pmip);
        return -1;
    }

    pmip = iget(dev, located_ino);

    pmip->INODE.i_atime = pmip->INODE.i_ctime = pmip->INODE.i_mtime = time(0L);

    pmip->dirty = 1;
    iput(pmip);
}

int DecToOctal(char num[])
{
    int length = strlen(num) - 1;
    int dec;
    for(int i = 0; i <= length; i++)
    {
        dec += ((num[i] - '0') * power(8, length - i));
        //printf("num[i] = %c\n", num[i]);
        //printf("length = %d\n", length - i);
        //printf("%d * (%d ^ %d) = %d\n", (num[i] - '0'), 8, length - i, (num[i] - '0')  * power(8, length - i));
    }

    return dec;
}

int power(int base, int exp)
{
    int final = base;
    if (exp == 0)
    {
        return 1;
    }
    if (exp == 1)
    {
        return base;
    }
    for(;exp > 1; exp--)
    {
        final = final * base;
    }
    return final;
}
