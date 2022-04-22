#ifndef FUNCTIONS_H
#define FUCNTIONS_H
// THis will be my header files for my function definitions
#include "type.h"

extern int dev;

extern char pathname[128];

extern MINODE *root;
extern PROC proc[NPROC], *running; 

//open_close_lseek.c
int open_file(char* pathname, int mode);
int close(int fd);
int my_lseek(int fd, int position);

//Link_Unlink.c

int my_link();
int my_unlink();
int symlink();
int readlink();     

//Mkdir_creat.c

int my_mkdir(char *pathname);
int kmkdir(MINODE *pmip, char *base, int pino);
int my_creat(char *pathname);
int kcreat(MINODE *pmip, char *base, int pino);
int enter_name(MINODE *pmip, int ino, char *name);

//Rmdir.c

int my_rmdir(char *pathname);
void rm_child(MINODE *pmip, char *name);

//cd_ls_pwd.c

int cd(char *pathname);
int ls(char *pathname);
int ls_file(MINODE *mip, char *name);
int ls_dir(MINODE *mip);
char *pwd(MINODE *wd);
int rpwd(MINODE *wd);

//alloc.c

int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
void decFreeInodes(int dev);
int incFreeInodes(int dev);
int ialloc(int dev);
int balloc(int dev);
int bdalloc(int dev, int blk);
int idalloc(int dev, int ino);

//util.c

int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char myname[ ]);
int findino(MINODE *mip, u32 *myino);
void new_directory(int ino, int bnum, int dev);
int ideal_len(int n);

//levelOneMinor.c

// utime filename: change file's access time to current time
int my_utime(char *pathname);
// chmod oct filename: Change filename's permission bits to octal value
int my_chmod(char *pathname, char *mode);
// stat ...
int my_stat();
// DecToOctal
int DecToOctal(char num[]);
int power(int base, int exp);

//Read_cat.c

//Reads a file
int read_file(int fd, char *buf, int nbytes);
//myread - helper function for read_file
int myread(int fd, char buf[ ], int nbytes);
//mycat - displays content of file to terminal
int mycat(char *pathname);

//Write_cp.c

//Writes to a file
int write_file(char* pathname);
//mywrite - helper function for write_file
int mywrite(int fd, char buf[ ], int nbytes);
//mycp - ...
int mycp(char* pathname, char* destination);

int my_mv(char* src, char* dest);

#endif