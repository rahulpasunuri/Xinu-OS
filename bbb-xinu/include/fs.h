#define FILENAMELEN 32
#define INODEBLOCKS 12
#define INODEDIRECTBLOCKS (INODEBLOCKS - 2)
#define DIRECTORY_SIZE 16

#define MDEV_BLOCK_SIZE 512
#define MDEV_NUM_BLOCKS 512
#define DEFAULT_NUM_INODES (MDEV_NUM_BLOCKS / 4)

#define INODE_TYPE_FILE 1
#define INODE_TYPE_DIR 2

//FILE OPEN MODES..
#define O_CREAT 1
#define O_WRONLY  2
#define O_RDWR 4
#define O_RDONLY 8

#define MAX_OPEN_FILES 15

#define FILE_CB_USED 11
#define FILE_CB_FREE 12

#define DIRENT_USED 17
#define DIRENT_FREE 18

#define INODE_FREE 30
#define INODE_USED 31

#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct inode {
  int id;
  short int state;
  int nextInodeNum;
  int currBlock;
  int offsetInBlock;
  int blocks[INODEBLOCKS];
};

#define FSTATE_CLOSED 0
#define FSTATE_OPEN 1

struct filetable {
  int lfstate; //is entry free or used.. 
  uint32 filePos; //current file Position..
  struct dirent *dirEntry;
  int fileptr;
};

struct dirent {
  sid32 fileMutex;
  int mode; //mode for the file..
  int inode_num;
  int curr_inode;
  int state;
  uint32 currFileSize;
  char name[FILENAMELEN];
};

struct directory {
  int numentries;
  sid32 mutex;
  struct dirent entry[DIRECTORY_SIZE];
};

struct fsystem {
  int nblocks;
  int blocksz;
  int ninodes;
  int inodes_used;
  int freemaskbytes;
  char *freemask;
  struct directory root_dir;
};

/* file and directory functions */
int fs_open(char *filename, int flags);
int fs_close(int fd);
int fs_create(char *filename);
int fs_seek(int fd, int offset);
int fs_read(int fd, void *buf, int nbytes);
int fs_write(int fd, void *buf, int nbytes);


/* filesystem functions */
int fs_mkfs(int dev, int num_inodes);
int fs_mount(int dev);

/* filesystem internal functions */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in);
int fs_put_inode_by_num(int dev, int inode_number, struct inode *in);
int fs_setmaskbit(int b);
int fs_clearmaskbit(int b);
int fs_getmaskbit(int b);

void checkBlocks(int fd);

/*
  Block Store functions
  bread, bwrite,
  bput, bget write entire blocks (macro with offset=0, len=blocksize)
 */
int bs_mkdev(int dev, int blocksize, int numblocks);
int bs_bread(int bsdev, int block, int offset, void *buf, int len);
int bs_bwrite(int bsdev, int block, int offset, void * buf, int len);

/* debugging functions */
void fs_printfreemask(void);
void fs_print_fsd(void);

void fs_init();

