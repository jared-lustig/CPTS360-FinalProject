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

// int chmod(char *filename)
// {
//     return 0;
// }

int my_utime(char *pathname)
{
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
