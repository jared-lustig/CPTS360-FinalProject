#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

extern MTABLE mountTablep[8];

// Modify mount_root(): Use mountTable[0] to record
//   dev, ninodes, nblocks, bmap, imap, iblk of root device

// write a MOUNT *getmptr(int dev) function, which returns a
//	  pointer to dev's mountTable[] entry

int my_mount(char* pathname, char *third)    /*  Usage: mount filesys mount_point OR mount */
{
    SUPER *supa;
    GD *ged;

    int i = 0;
    int devinitlytemp;
    int nblocks, ninodes, bmap, imap, iblk;

    int ino, fd;
    char mbuf[BLKSIZE];

    printf("disk name = %s\n", pathname);

    printf("checking EXT2 FS ....");
    if ((fd = open(pathname, O_RDWR)) < 0){
        printf("open %s failed\n", pathname);
        exit(1);
    }

    devinitlytemp = fd;    // global dev same as this fd   

    printf("dev = %d, fd = %d\n", dev, fd);

    /********** read super block  ****************/
    bzero(mbuf, BLKSIZE);
    get_block(devinitlytemp, 1, mbuf);
    supa = (SUPER *)mbuf;

    if (supa->s_magic != 0xEF53){
        printf("magic = %x is not an ext2 filesystem\n", supa->s_magic);
        exit(1);
    }     
    printf("EXT2 FS OK\n");

    ninodes = supa->s_inodes_count;
    nblocks = supa->s_blocks_count;

    get_block(dev, 2, mbuf); 
    ged = (GD *)mbuf;

    bmap = ged->bg_block_bitmap;
    imap = ged->bg_inode_bitmap;
    iblk = ged->bg_inode_table;
    printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

    // 1. Ask for filesys (a virtual disk) and mount_point (a DIR pathname).
    //    If no parameters: display current mounted filesystems.

    // 2. Check whether filesys is already mounted: 
    //    (you may store mounted filesys name in the MOUNT table entry). 
    //    If already mounted, reject;
    //    else: allocate a free MOUNT table entry (dev=0 means FREE).

    // 3. LINUX open filesys for RW; use its fd number as the new DEV;
    //    Check whether it's an EXT2 file system: if not, reject.


    /* verify it's an ext2 file system ***********/



    // 4. For mount_point: find its ino, then get its minode:
    ino  = getino(third);  // get ino:
    MINODE *mip  = iget(dev, ino); // get minode in memory;

    printf("mip->ino = %d, running->cwd->ino = %d\n", mip->ino, running->cwd->ino);

    if (mip->ino == running->cwd->ino)
    {
        printf("Currently in use, error\n");
        return -1;
    }    



    // 5. Verify mount_point is a DIR.  // can only mount on a DIR, not a file  
    //    Check mount_point is NOT busy (e.g. can't be someone's CWD)
    //mountTablep[0].mounted_inode
    if(S_ISDIR(mip->INODE.i_mode)) // CHECK IF DIR
        {
        printf("mount: Valid Dirname\n");
    }
    else
    {
        printf("mount: Invalid Dirname\n");
        return -1;
    }    

    // 6. Allocate a FREE (dev=0) mountTable[] for newdev;

    while(mountTablep[i].dev != 0)
    {
        i++;
    }

    //    Record new DEV, ninodes, nblocks, bmap, imap, iblk in mountTable[] 
    mountTablep[i].dev = devinitlytemp;
    mountTablep[i].ninodes = ninodes;
    mountTablep[i].nblocks = nblocks;
    mountTablep[i].bmap = bmap;
    mountTablep[i].imap = imap;
    //mountTablep[i].iblk = iblk;

    // 7. Mark mount_point's minode as being mounted on and let it point at the
    //    MOUNT table entry, which points back to the mount_point minode.
    mip->dirty = 1;
    mip->mounted = 1;
    mip->mptr = mountTablep;
    mountTablep[i].mntDirPtr = mip;

    iput(mip);

    //return 0 for SUCCESS;
    return 0;
}
  

int umount(char *filesys)
{

    // 1. Search the MOUNT table to check filesys is indeed mounted.

    // 2. Check whether any file is still active in the mounted filesys;
    //       e.g. someone's CWD or opened files are still there,
    //    if so, the mounted filesys is BUSY ==> cannot be umounted yet.
    //    HOW to check?      ANS: by checking all minode[].dev with refCount>0

    // 3. Find the mount_point's inode (which should be in memory while it's mounted on).
    //    Reset it to "not mounted"; then 
    //    iput() the minode.  (because it was iget()ed during mounting)

    // 4. return 0 for SUCCESS;
    return 0;

} 