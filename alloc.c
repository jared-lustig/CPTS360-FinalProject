int tst_bit(char *buf, int bit) // in Chapter 11.3.1
{
    int i = bit / 8, j = bit % 8; // 8 = number of bits in a byte

    if(buf[i] & (1<<j)) // if bit 1
    {
        return 1;
    }
    else // if bit 0
    {
        return 0;
    }
}

int set_bit(char *buf, int bit) // in Chapter 11.3.1
{
    int i = bit / 8, j = bit % 8; // 8 = number of bits in a byte

    buf[i] |= (1<<j);
}

int clr_bit(char *buf, int bit)
{
    int i = bit / 8, j = bit % 8; // 8 = number of bits in a byte

    buf[i] &= -(i<<j);
}

int decFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // dec free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){ // use ninodes from SUPER block
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
	put_block(dev, imap, buf);

	decFreeInodes(dev);

	printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}

// WRITE YOUR OWN balloc(dev) function, which allocates a FREE disk block number
