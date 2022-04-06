int my_mkdir(char *pathname)
{
    // 1. if (pathname is absolute) dev = root->dev;
    if(pathname) // if pathname is absolute
    {
        dev = root;      
    }
    else
    {
        dev = running->cwd;
    }
    printf("is it getting to this statement?\n");
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
    MINODE *pmip = iget(&dev, pino);
    // //check pmip ->INODE is a DIR
    if(!S_ISDIR(pmip->INODE.i_mode)) // check if DIR
    {
        printf("mkdir: Valid Dirname\n");
    }
    else
    {
        printf("mkdir: Invalid Dirname\n");
        return -1;;
    }    

    /*4. // basename must not exist in parent DIR:*/
    int exist = search(pmip, base); //must return 0;
    if(exist != 0)
    {
        printf("mkdir: basename already exist, cannot make another directory the same name \n");
        return -1;
    }
    /*5. call kmkdir(pmip, basename) to create a DIR;
    kmkdir() consists of 4 major steps:*/
    kmkdir(pmip, base);

    //6. increment parent INODE's link_count by 1 and mark pmip dirty;
    pmip->dirty = 1;
    pmip->INODE.i_links_count++;
    
   return 0;
}

int kmkdir(MINODE *pmip, char *base)
{
    char buf[BLKSIZE];
    //5-1. allocate an INODE and a disk block:
    int ino = ialloc(dev), bno = balloc(dev);
    int mip = iget(dev,ino); // load INODE into an minode;

    //5-2. initialize mip->INODE as a DIR INODE;
    pmip->INODE.i_block[0] = bno; //other i_block[ ] are 0;
    pmip->dirty = 1; //marking as dirty
    iput(pmip); // write INODE back to disk


    //5-3. make data block 0 of INODE to contain . and .. entries;
    //write to disk block blk
    new_directory(pmip, ino, bno, buf);

    //5-4. enter_child(pmip, ino, basename); which enters
    //(ino, basename) as a DIR entry to the parent INODE;
    enter_child(pmip, ino, base);

    return 1;
}

// int creat(char *pathname)
// {
//     //This is similar to mkdir() except
//     //(1). the INODE.i_mode field is set to Reg file type, permission bits set to 0644 for rw-r--r--, and
//     //(2). no data block is allocated for it, so the file size is 0.
//     //(3). Do not increment parent INODE's links_count
//     return 0;
// }