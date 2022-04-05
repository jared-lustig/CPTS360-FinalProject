int mkdir(char *pathname)
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
    /*2. divide pathname into dirname and basename;*/
    char *dirname;

    //3. // dirname must exist and is a DIR:
    int pino = getino(&dev, dirname);
    MINODE *pmip = iget(dev, pino);
    //check pmip ->INODE is a DIR
    if(pmip)
    {
        
    }    

    /*4. // basename must not exist in parent DIR:*/
    search(pmip, basename); //must return 0;

    /*5. call kmkdir(pmip, basename) to create a DIR;
    kmkdir() consists of 4 major steps:

        5-1. allocate an INODE and a disk block:
        ino = ialloc(dev); blk = balloc(dev);
        mip = iget(dev,ino); // load INODE into an minode

        5-2. initialize mip->INODE as a DIR INODE;
        mip->INODE.i_block[0] = blk; other i_block[ ] are 0;
        mark minode modified (dirty);
        iput(mip); // write INODE back to disk

        5-3. make data block 0 of INODE to contain . and .. entries;
        write to disk block blk.

        5-4. enter_child(pmip, ino, basename); which enters
        (ino, basename) as a DIR entry to the parent INODE;

    6. increment p
    */
}

int creat(char *pathname)
{
    //This is similar to mkdir() except
    //(1). the INODE.i_mode field is set to Reg file type, permission bits set to 0644 for rw-r--r--, and
    //(2). no data block is allocated for it, so the file size is 0.
    //(3). Do not increment parent INODE's links_count
}