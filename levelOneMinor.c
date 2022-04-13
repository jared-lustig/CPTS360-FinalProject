#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

// int stat()
// {
//     return 0;
// }

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
        printf("utime: could not find file to edit utime\n");
        iput(pmip);
        return -1;
    }

    pmip = iget(dev, located_ino);

    printf("current mode = %d\n", pmip->INODE.i_mode);
    printf("0644 mode = %x\n", DecToOctal(0644));

    pmip->INODE.i_mode = DecToOctal(atoi(mode));

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

int DecToOctal(int num)
{
    int octalNumber[100], i = 1;
    while (num != 0)
    {
        octalNumber[i++] = num % 8;
        num = num / 8;
    }
    return octalNumber;
}