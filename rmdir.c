
int rmdir(char *pathname)
{
    /*1. get in - memory INODE of pathname:*/
    int ino = getino(pathname);
    MINODE *mip = iget(dev, ino);

    /*2. verify INODE is a DIR (by INODE.i_mode field);
    minode is not BUSY (refCount = 1);
    DIR is empty (traverse data blocks for number of entries = 2);*/
    if(mip->INODE.i_mode)
    {
        //not busy
        mip->refCount = 1;
    }
    else
    {
        //traverse data blocks for number entries = 2
    }

    /*3. get parent's ino and inode*/
    int pino = findino(); //get pino from .. entry in INODE.i_block[0]
    MINODE *pmip = iget(mip->dev, pino);
    
    /*4. remove name from parent directory */
    findname(pmip, ino, name); //find name from parent DIR
    rm_child(pmip, name);

    /*5. deallocate its data blocks and inode */
    truncat(mip); // deallocate INODE's data blocks

    //6. deallocate INODE
    idalloc(mip->dev, mip->ino); iput(mip);

    //7. dec parent links_count by 1;
    //mark parent dirty; iput(pmip);*/
    pmip->INODE.i_links_count -= 1;
    pmip->dirty = 1;
    iput(pmip);
    
    //8. return 0 for SUCCESS
    return 0;
    
}