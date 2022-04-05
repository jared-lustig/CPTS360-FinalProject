/************* cd_ls_pwd.c file **************/
int cd()
{
  //printf("cd: under construction READ textbook!!!!\n");

  // READ Chapter 11.7.3 HOW TO chdir

  int ino = getino(pathname);
  MINODE *mip = iget(dev, ino);
  if(mip) // if a DIR
  {
    iput(running->cwd);
    running->cwd = mip;
    printf("cd: changed directories\n");
  }
  else{
    printf("cd: mip was not of type DIR\n");
  }
}

int ls_file(MINODE *mip, char *name)
{
  //printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
}

int ls_dir(MINODE *mip)
{
  //printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;

  while (cp < buf + BLKSIZE){
     //int ino = getino(pathname);
     mip = iget(dev, dp->inode);
     
      char *t1 = "xwrxwrxwr-------";
      char *t2 = "----------------";

      if ((mip->INODE.i_mode & 0xF000) == 0x8000) // if (S_ISREG())
      printf("%c",'-');
      if ((mip->INODE.i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
      printf("%c",'d');
      if ((mip->INODE.i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
      printf("%c",'l');
      for (int i=8; i >= 0; i--){
        if (mip->INODE.i_mode & (1 << i)) // print r|w|x
          printf("%c", t1[i]);
        else
          printf("%c", t2[i]); // or print -
      }

     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
    //  ls_file(mip, temp);
    //  iput(mip);

     int gid, uid;
     gid = getgid(); uid = geteuid();
     char mtime[64];
     ctime_r((time_t *)&mip->INODE.i_mtime, mtime);
     mtime[strlen(mtime) - 1] = 0;

     printf("%8u %8u %8u %s %8u %s \t[%d %2d]\n", mip->INODE.i_links_count, 
     mip->INODE.i_gid, mip->INODE.i_uid, mtime, mip->INODE.i_size, temp, mip->dev, mip->ino);

     cp += dp->rec_len;
     dp = (DIR *)cp;

     
  }
  printf("\n");
}

int ls(char *pathname)
{
  // printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
  // ls_dir(running->cwd);  
  if(pathname[0] == '\0')
    ls_dir(running->cwd);

  int ino = getino(pathname);
  MINODE* mip = iget(dev, ino);

  int mode = mip->INODE.i_mode;

  if(S_ISDIR(mode))
    ls_dir(mip);
  else 
    ls_file(mip, running->cwd);

  iput(mip);
}

char *pwd(MINODE *wd)
{
  //printf("pwd: READ HOW TO pwd in textbook!!!!\n");
  if (wd == root){
    printf("pwd = /\n");
    return;
  }
  else{
    rpwd(wd);
    printf("\n");
  }
}

int rpwd(MINODE *wd)
{
  if(wd == root) return;
  int *ino;
  char sbuf[BLKSIZE], thename[20];

  get_block(dev, wd->INODE.i_block[0], sbuf);
  
  int parent_ino = findino(wd, &ino); // getting parent_ino
  MINODE* pip = iget(dev, parent_ino); // getting pip MINODE

  //printf("parent ino = %d, my_ino = %d\n", parent_ino, ino);    

  findmyname(pip, ino, &thename);

  rpwd(pip); // recursive call with parent minode

  printf("/%s", thename);
  
  iput(pip); // no longer need pip
}

