#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "functions.h"

MOUNT mountTablep[8]; // set all dev = 0 in init()

// Modify mount_root(): Use mountTable[0] to record
//   dev, ninodes, nblocks, bmap, imap, iblk of root device

// write a MOUNT *getmptr(int dev) function, which returns a
//	  pointer to dev's mountTable[] entry

int mount()    /*  Usage: mount filesys mount_point OR mount */
{

    // 1. Ask for filesys (a virtual disk) and mount_point (a DIR pathname).
    //    If no parameters: display current mounted filesystems.

    // 2. Check whether filesys is already mounted: 
    //    (you may store mounted filesys name in the MOUNT table entry). 
    //    If already mounted, reject;
    //    else: allocate a free MOUNT table entry (dev=0 means FREE).

    // 3. LINUX open filesys for RW; use its fd number as the new DEV;
    //    Check whether it's an EXT2 file system: if not, reject.

    // 4. For mount_point: find its ino, then get its minode:
    //     ino  = getino(pathname);  // get ino:
    //     mip  = iget(dev, ino);    // get minode in memory;    

    // 5. Verify mount_point is a DIR.  // can only mount on a DIR, not a file  
    //    Check mount_point is NOT busy (e.g. can't be someone's CWD)

    // 6. Allocate a FREE (dev=0) mountTable[] for newdev;

    //    Record new DEV, ninodes, nblocks, bmap, imap, iblk in mountTable[] 


    // 7. Mark mount_point's minode as being mounted on and let it point at the
    //    MOUNT table entry, which points back to the mount_point minode.

    //    return 0 for SUCCESS;
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