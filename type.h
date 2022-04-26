#ifndef TYPE
#define TYPE

/*************** type.h file for LEVEL-1 ****************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   

#define FREE        0
#define READY       1

#define BLKSIZE  1024
#define NMINODE   128
#define NPROC       2
#define NFD        10
#define NOFT       64

typedef struct minode{
  INODE INODE;           // INODE structure on disk
  int dev, ino;          // (dev, ino) of INODE
  int refCount;          // in use count
  int dirty;             // 0 for clean, 1 for modified

  int mounted;           // for level-3
  struct mntable *mptr;  // for level-3
}MINODE;

typedef struct oft{ //OpenFileTable
  int mode; //R | W | RW | APP
  int refCount;
  MINODE *minodePtr;
  int offset;
}OFT;

OFT oft[64];

typedef struct proc{
  struct proc *next;
  int          pid;      // process ID  
  int          uid;      // user ID
  int          gid;
  MINODE      *cwd;      // CWD directory pointer  

  OFT *fd[NFD];
}PROC;

typedef struct mtable{
  int dev; // device number; 0 for FREE
  int ninodes; // from superblock
  int nblocks;
  int free_blocks; // from superblock and GD
  int free_inodes;
  int bmap; // from group descriptor
  int imap;
  int iblock; // inodes start block
  MINODE *mntDirPtr; // mount point DIR pointer
  char devName[64]; //device name
  char mntName[64]; // mount point DIR name
}MTABLE;

// ADD this to type.h: Mount Table structure
typedef struct Mount{
  int    dev;       // dev (opened vdisk fd number) 0 means FREE 
  int    ninodes;   // from superblock
  int    nblocks;
  int    bmap;      // from GD block  
  int    imap;
  int    iblk;
  MINODE *mounted_inode;
  char   name[64];  // device name, e.g. mydisk
  char   mount_name[64]; // mounted DIR pathname
} MOUNT;

#endif