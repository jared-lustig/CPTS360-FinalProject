#ifndef FUNCTIONS_H
#define FUCNTIONS_H
// THis will be my header files for my function definitions
#include "type.h"

extern int dev;

extern char pathname[128];

extern MINODE *root;
extern PROC proc[NPROC], *running; 

//Mkdir_creat.c
int my_mkdir(char *pathname);
int kmkdir(MINODE *pmip, char *base);
//int creat(char *pathname);

//Rmdir.c
int my_rmdir(char *pathname);

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
int decFreeInodes(int dev);
int ialloc(int dev);
int balloc(int dev);

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
void new_directory(MINODE *pmip, int ino, int bnum, char *buf);
int enter_child(MINODE *pmip, int ino, char *name);
int ideal_len(int n);


#endif