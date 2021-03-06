/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"
#include "functions.h" 

extern int cd();
extern char *pwd(MINODE *wd);
extern int ls(char *pathname);
extern int my_mkdir(char *pathname);
extern int my_rmdir(char *pathname);

extern MINODE *iget();

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[128], third[128];
OFT init_oft[10];

MTABLE mountTablep[8]; // set all dev = 0 in init()

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    for (j = 0; j < NFD; j++)
    {
      proc[i].fd[j] = 0;
      proc[i].next = &proc[i+1];
    }
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
  }
  for (i = 0; i < NOFT; i++)
  {
    oft[i].refCount = 0;
  }

  for (i = 0; i < 8; i++)
  {
    mountTablep[i].dev = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

char *disk = "mydisk";
int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  dev = fd;    // global dev same as this fd   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  printf("number of blocks = %d\n", nblocks);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  // WRTIE code here to create P1 as a USER process
  
  while(1){
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink\n|chmod|utime|stat|read|cat|mount|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;
    third[0] = 0;

    sscanf(line, "%s %s %s", cmd, pathname, third);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0)
       ls(pathname);
    else if (strcmp(cmd, "cd")==0)
       cd(pathname);
    else if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    else if (strcmp(cmd, "mkdir") == 0)
      my_mkdir(pathname);
    else if (strcmp(cmd, "creat") == 0)
      my_creat(pathname); //pathname must have a / in front of file to make it
    else if (strcmp(cmd, "rmdir") == 0)
       my_rmdir(pathname);
    else if (strcmp(cmd, "link")==0)
       my_link();
    else if (strcmp(cmd, "unlink")==0)
       my_unlink();
    else if (strcmp(cmd, "symlink")==0)
       my_symlink();
    else if (strcmp(cmd, "readlink")==0)
       my_readlink();
    else if (strcmp(cmd, "utime") == 0)
      my_utime(pathname);
    else if (strcmp(cmd, "chmod") == 0)
      my_chmod(pathname, third);
    else if (strcmp(cmd, "stat") == 0)
      my_stat();
    else if (strcmp(cmd, "open")==0) {
      int open_int = atoi(third); 
      open_file(pathname, open_int);
    }
    else if (strcmp(cmd, "close")==0) {
      int close_int = atoi(pathname); 
      close(close_int);
    }
    else if (strcmp(cmd, "read") == 0)
      myread(fd, buf, n);
    else if(strcmp(cmd, "cat") == 0)
      mycat(pathname);
    else if(strcmp(cmd, "cp") == 0)
      mycp(pathname, third);
    else if(strcmp(cmd, "mv") == 0)
      my_mv(pathname, third);
    else if(strcmp(cmd, "seek") == 0) {
      int mv_src = atoi(pathname);
      int mv_dest = atoi(third);
      my_lseek(mv_src, mv_dest);
    }
    else if(strcmp(cmd, "mount") == 0)
      my_mount(pathname, third);
    else if(strcmp(cmd, "pfd") == 0)
      pfd(pathname, third);
    else if (strcmp(cmd, "quit")==0)
       quit();
  }
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
