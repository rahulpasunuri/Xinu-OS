#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0 //has filesystem block
#define BM_BLK 1 // has freemem mask..
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int getFreeInodeNum();
int getFreeBlockNum();
void initInodes();

//returns 1 if they are same..
//returns 0 otherwise..
int isSameString(char* s1, char* s2)
{
    int index = 0;
    while(s1[index] == s2[index] && s1[index] != '\0' && s2[index] != '\0')
    {
        index++;
    }
    
    if(s1[index] != '\0' || s2[index] != '\0')
    {
        return 0;
    }
    
    return 1;
}

int getStringLength(char* s)
{
    int len = 0;
    while(s[len] != '\0')
    {
        len++;
    }
    
    return len;
}

void initDirectoryEntry(int index)
{
    fsd.root_dir.entry[index].state = DIRENT_FREE;
    fsd.root_dir.entry[index].currFileSize = 0;
    fsd.root_dir.entry[index].inode_num = -1;
    fsd.root_dir.entry[index].curr_inode = -1;
    fsd.root_dir.entry[index].currFileSize = 0;
    fsd.root_dir.entry[index].fileMutex = semcreate(1);
}

void createNewDirEntry(int index, char* fileName)
{
    initDirectoryEntry(index);
    
    //the directory entry is used now!!
    fsd.root_dir.entry[index].state = DIRENT_USED;
    fsd.root_dir.entry[index].fileMutex = semcreate(1);
    //set the name in the directory entry!!
    int nameLen = getStringLength(fileName);
    //fsd.root_dir.entry[index].name = getmem(nameLen+1); //one extra byte for the null character..
    int i=0;
    for(; i<nameLen; i++)
    {
        fsd.root_dir.entry[index].name[i] = fileName[i];
    }
    fsd.root_dir.entry[index].name[nameLen] = '\0';
    
    //increment the total number of entries
    fsd.root_dir.numentries ++;
}

void initFCB(int index)
{
    oft[index].lfstate = FILE_CB_FREE;
    oft[index].dirEntry = NULL;
    //oft[index].fileMutex = semcreate(1);
    oft[index].filePos = 0;
    oft[index].dirEntry = NULL; 
}

//initialize the directory properties..
void init_dir()
{
    fsd.root_dir.mutex = semcreate(1);
    fsd.root_dir.numentries = 0;
}

//initializes the file system!!
void fs_init()
{
    initInodes();
    init_dir();
    //TODO: call this method from some where..
    
    int pos = 0;
    for(; pos < MAX_OPEN_FILES; pos++)
    {
        //update the state of every file control block..
        initFCB(pos);
    }

    //loop through all the directory entries and mark their state..
    for(pos=0; pos < DIRECTORY_SIZE; pos++)
    {
        initDirectoryEntry(pos);
    }
    
}


int fs_open(char *filename, int flags)
{

    //get a lock on the directory..
    wait(fsd.root_dir.mutex);

    int pos = 0;
    int ptrPos = -1;
    
    //ensure that the length of the file name is in allowed range!!
    int nameLength = 0;
    while(filename[nameLength] != '\0') { 
        nameLength++; 
    } 
    
    if(nameLength > FILENAMELEN)
    {
        kprintf("Error: File name specified is greater than the allowed limit!!\n");
        signal(fsd.root_dir.mutex); //release the mutex on the file directory..
        return SYSERR;
    }
    
    for(; pos < MAX_OPEN_FILES; pos++)
    {
        if(oft[pos].lfstate == FILE_CB_FREE)
        {
            if(ptrPos == -1)
            {
                ptrPos = pos; //get the position of the first free control block!!
            }    
            continue;
        }
        
        //here the all control blocks are already open...
        //make sure that the corresponding file is not already open!!
        
        if(isSameString(oft[pos].dirEntry->name, filename))
        {
            kprintf("Error: File cannot be opened, as it is already open!!\n");
            signal(fsd.root_dir.mutex); //release the mutex on the file directory..
            return SYSERR;
        }
    }
    
    if(ptrPos == -1)
    {
        kprintf("Error: The limit on max nuber of open file reached!!");
        signal(fsd.root_dir.mutex); //release the mutex on the file directory..
        return SYSERR;
    }
    
    //search through the directory entries to see if the file exists already!!
    struct dirent* dirPtr = fsd.root_dir.entry;
    int dirIndex = 0;
    int freeDirPos = -1;
    int isDirEntryFound = 0;
    

    for(; dirIndex< DIRECTORY_SIZE; dirIndex++)
    {
        if (dirPtr[dirIndex].state == DIRENT_USED)
        {
            //match the name with the current name..
            if(isSameString(filename, dirPtr[dirIndex].name))
            {
                //directory entry found..
                isDirEntryFound = 1;
                break;
            }   
        }
        else
        {
            if(freeDirPos == -1)
            {
                //get the index of the first free directory entry!!
                freeDirPos = dirIndex;
            }
        }
    }
    
    //TODO: update the numentires, wherever...
    if(fsd.root_dir.numentries == DIRECTORY_SIZE && isDirEntryFound == 0)
    {
        kprintf("Error: Max limit on the size of the directory entries!!\n");
        signal(fsd.root_dir.mutex); //release the mutex on the file directory..
        return SYSERR;
    }

    if(isDirEntryFound)
    {
        /*
        if(flags&FILE_NEW)
        {
            kprintf("Error: File Already exists!!\n");
            signal(fsd.root_dir.mutex); //release the mutex on the file directory..
            return SYSERR;
        }
        */
    }
    else
    {
        //file doesnt exist..
        if((flags & O_CREAT) == 0)
        {
            kprintf("Error: File Doesn't exist!!\n");
            signal(fsd.root_dir.mutex); //release the mutex on the file directory..
            return SYSERR;
        }

        //create a new directory entry here..
        createNewDirEntry(freeDirPos, filename);
        dirIndex = freeDirPos;        
    }  
    
    //if(isOpenMode)
    //{
    oft[ptrPos].lfstate = FILE_CB_USED;
    oft[ptrPos].dirEntry = &(dirPtr[dirIndex]);
    oft[ptrPos].dirEntry->mode = flags;
    //}
    signal(fsd.root_dir.mutex); //release the mutex on the file directory..
    //if(isOpenMode)
    //{
    return ptrPos;
    //}
    //return OK;
}

void initInodes()
{
    int inodeNum = 0;
    for( ; inodeNum < fsd.ninodes; inodeNum++)
    {
        struct inode currInode;
        currInode.state = INODE_FREE;
        currInode.id = inodeNum;
        currInode.nextInodeNum = -1;
        int blockIndex = 0;
        for(; blockIndex < INODEBLOCKS; blockIndex++)
        {
            currInode.blocks[blockIndex] = -1;
        }
        
        currInode.offsetInBlock = 0;
        currInode.currBlock = -1;
        
        //TODO: what more properties to add???
        fs_put_inode_by_num(0, inodeNum, &currInode);
    }
}

int fs_close(int fd)
{
    wait(fsd.root_dir.mutex);
    if( fd < 0 || fd >= MAX_OPEN_FILES)
    {
        kprintf("Error: File descriptor is out of allowed bounds!!");
        signal(fsd.root_dir.mutex);
        return SYSERR;
    }

    if(oft[fd].lfstate != FILE_CB_USED)
    {
        kprintf("Error: Cannot close unopened file!!");
        signal(fsd.root_dir.mutex);
        return SYSERR;
    }

    //just close the corresponding control block
    initFCB(fd);
    signal(fsd.root_dir.mutex);
    return OK;
}

int fs_create(char *filename)
{
    return fs_open(filename, O_RDWR | O_CREAT ) ;
}

int readDataBlock(int dataBlock, int offset, char *c)
{
    bs_bread(0, 2+ NUM_INODE_BLOCKS + dataBlock, offset, (void*)c, 1);
    return OK;
}

void checkBlocks(int fd)
{
    struct inode currInode;
    //printf("Start Inode: %d\n", oft[fd].dirEntry->inode_num);    
    fs_get_inode_by_num(0, oft[fd].dirEntry->inode_num, &currInode);  
    //printf("Start Block: %d\n", currInode.blocks[0]);
    //char temp[1024];  
    //bs_bread(0, 2+ NUM_INODE_BLOCKS + currInode.blocks[0], 0, (void*)temp, fsd.blocksz);
    //printf("Block is: %s", temp);
}

int fs_getc(struct dirent* dirEntry)
{
    
    if ((dirEntry->mode & O_WRONLY) == 1)
    {
        return SYSERR;
    }
    
    if ( ! ((dirEntry->mode & O_RDONLY) | (dirEntry->mode & O_RDWR) ))
    {
        return SYSERR;
    }
    wait(dirEntry->fileMutex);
    struct inode currInode;
    fs_get_inode_by_num(0, dirEntry->curr_inode, &currInode);
    char currChar;
    readDataBlock(currInode.blocks[currInode.currBlock], currInode.offsetInBlock, &currChar);
    //char temp[2014];
    //printf("Block: " );
    if( currInode.offsetInBlock == (fsd.blocksz -1 ))
    {
        if(currInode.currBlock == (INODEBLOCKS - 1))
        {
            //move to next inode..
            currInode.offsetInBlock = 0;
            currInode.currBlock = 0;
            if(currInode.nextInodeNum < 0)
            {
                signal(dirEntry->fileMutex);
                return SYSERR;
            }
            dirEntry->curr_inode = currInode.nextInodeNum;
        }
        else
        {
            //move to next block..
            currInode.offsetInBlock = 0;
            currInode.currBlock++;
        }
    }
    else
    {
        //move to next position..
        currInode.offsetInBlock++;
    }
    fs_put_inode_by_num(0, currInode.id, &currInode);
    signal(dirEntry->fileMutex);
    
    if(currChar == '\0')
    {
        //return EOF;
    }
    return currChar;
}

int createNewInode(struct inode *currInode)
{
    //a new inode is created here..
    int freeInodeNum = getFreeInodeNum();
    if(freeInodeNum < 0)
    {
        kprintf("Error: No Free inodes are available!!\n");
        return SYSERR;
    }

    //get the inode..
    fs_get_inode_by_num(0, freeInodeNum, currInode);

    //mark the inode as used!!
    currInode->state = INODE_USED;
    currInode->currBlock = 0;

    //update the status in the database,,
    fs_put_inode_by_num(0, freeInodeNum, currInode);

    //fsd.ninodes++; 
    return OK;
}

int updateDataBlock(int dataBlock, int offset, char c)
{
    bs_bwrite(0, 2+ NUM_INODE_BLOCKS + dataBlock, offset, (void*)&c, 1);
    return OK;
}

int fs_putc(struct dirent* dirEntry, char c)
{
    if ((dirEntry->mode & O_RDONLY) == 1)
    {
        return SYSERR;
    }

    if ( ! ((dirEntry->mode & O_WRONLY) | (dirEntry->mode & O_RDWR) ))
    {
        return SYSERR;
    }
    //printf("Inside putc1\n");
    wait(dirEntry->fileMutex);
     //printf("Inside putc2\n");
    int inodeNum = dirEntry->inode_num;
    struct inode currInode;
    if( inodeNum < 0)
    {
        // printf("Inside putc3\n");
        if(createNewInode(&currInode) == SYSERR)
        {
          //   printf("Inside putc4\n");
            signal(dirEntry->fileMutex);
            return SYSERR;
        }
         //printf("Inside putc5\n");
         //sleep(1);
        dirEntry->inode_num = currInode.id;
        //printf("Inside putc55\n");sleep(1);
        dirEntry->curr_inode = currInode.id;
         //printf("Inside putc6\n");sleep(1);
        currInode.currBlock = 0;
        //create the block..
        int freeBlockNum = getFreeBlockNum();
           // printf("Inside putc7\n");
        if(freeBlockNum<0)
        {
             //printf("Inside putc8\n");
            signal(dirEntry->fileMutex);
            return SYSERR;            
        }
            //printf("Inside putc9\n");
        fs_setmaskbit(freeBlockNum);
        currInode.blocks[currInode.currBlock] = freeBlockNum;
         //printf("Inside putc10\n");
        updateDataBlock(freeBlockNum, 0, c);
        currInode.offsetInBlock++;
         //printf("Inside putc11\n");
        fs_put_inode_by_num(0, currInode.id, &currInode);
    }
    
    else
    {
         //printf("Inside putc12\n");
        inodeNum = dirEntry->curr_inode;
        fs_get_inode_by_num(0, inodeNum, &currInode);
        updateDataBlock(currInode.blocks[currInode.currBlock], currInode.offsetInBlock, c);
         //printf("Inside putc13\n");
        //update the pointer..
        if(currInode.offsetInBlock == (fsd.blocksz -1 ) )
        {
           //  printf("Inside putc14\n");
            if(currInode.currBlock == (INODEBLOCKS - 1))
            {
             //    printf("Inside putc15\n");
                //have to create a new inode..
                struct inode newInode;
                if(createNewInode(&newInode) == SYSERR)
                {
               //      printf("Inside putc16\n");
                    signal(dirEntry->fileMutex);
                    return SYSERR;
                }
                 //printf("Inside putc17\n");
                currInode.nextInodeNum = newInode.id;
                currInode.currBlock = 0;
                currInode.offsetInBlock = 0;
                fs_put_inode_by_num(0, currInode.id, &currInode);
                dirEntry->curr_inode = currInode.id;
                 //printf("Inside putc18\n");
            }
            else
            {
                 //printf("Inside putc19\n");
                //create a new block..
                currInode.currBlock++;
                currInode.offsetInBlock = 0;
                int freeBlockNum = getFreeBlockNum();
                 //printf("Inside putc20\n");
                if(freeBlockNum<0)
                {
                   //  printf("Inside putc21\n");
                    signal(dirEntry->fileMutex);
                    return SYSERR;            
                }
                 //printf("Inside putc22\n");
                fs_setmaskbit(freeBlockNum);
                currInode.blocks[currInode.currBlock] = freeBlockNum;
                 //printf("Inside putc23\n");
            }
        }
        else
        {
            //just update the offset parameter..
            currInode.offsetInBlock++;
             //printf("Inside putc24\n");
        }
        fs_put_inode_by_num(0, currInode.id, &currInode);
         //printf("Inside putc25\n");
    }
     //printf("Inside putc26\n");
    signal(dirEntry->fileMutex);      
    return OK; 
}

int fs_read(int fd, void *buf, int nbytes)
{
    if(nbytes <= 0)
    {
        kprintf("Error: numbytes must be greater than 0!!\n");
        return SYSERR;
    }
    
    if(oft[fd].lfstate == FILE_CB_FREE)
    {
        kprintf("Error: File cannot be read, because it is not open!!\n");
        return SYSERR;
    }

    int byteCount = 0;
    char* result = (char *)buf;
    for(;byteCount < nbytes; byteCount++)
    {
        //printf("Inside read1\n");
        int32 c = fs_getc(oft[fd].dirEntry);
        if( c == SYSERR)
        {   
            //printf("Inside read2\n");
            return SYSERR;
        }
        if( c == EOF)
        {
            //printf("Inside read3\n");
            if(byteCount == 0)
            {
                //printf("Inside read4\n");
                return EOF;
            }
               //printf("Inside read5\n");
            return byteCount;
        }
        //printf("Inside read6\n");
        result[byteCount] = (char) (0xff & c);
    }
    //printf("Inside read7\n");
    return byteCount;
}

int fs_write(int fd, void *buf, int nbytes)
{
    if(nbytes <= 0)
    {
        kprintf("Error: numbytes must be greater than 0!!\n");
        return SYSERR;
    }
    
    if(oft[fd].lfstate == FILE_CB_FREE)
    {
        kprintf("Error: File cannot be written, because it is not open!!\n");
        return SYSERR;
    }
    
    int byteCount = 0;
    char* data = (char *)buf;
    for(;byteCount < nbytes; byteCount++)
    {
        if(fs_putc(oft[fd].dirEntry, data[byteCount]) == SYSERR)
        {
            return SYSERR;
        }
            oft[fd].filePos++;
            //increase the file size..
        (oft[fd].dirEntry->currFileSize)++;
    }
    return nbytes; 
}

//TODO
int fs_seek(int fd, int offset)
{        
    if(oft[fd].lfstate == FILE_CB_FREE)
    {
        kprintf("Error: File cannot be read, because it is not open!!\n");
        return SYSERR;
    }
    int newPosition = oft[fd].filePos + offset;
    if( newPosition < 0 || newPosition >= oft[fd].dirEntry->currFileSize)
    {
        kprintf("Error: Seek cannot be out of bounds of the file\n");
        return SYSERR;
    }
    
    //update the current INODE..
    struct inode currInode;    
    fs_get_inode_by_num(0, oft[fd].dirEntry->curr_inode, &currInode);
    currInode.currBlock = 0; currInode.offsetInBlock = 0;
    fs_put_inode_by_num(0, currInode.id, &currInode);
    
    //update the position..
    oft[fd].filePos = newPosition;
    
    //update the block..
    int inode_total_size = INODEBLOCKS * fsd.blocksz;
    fs_get_inode_by_num(0, oft[fd].dirEntry->inode_num, &currInode);
    while( newPosition >= inode_total_size)
    {
        currInode.currBlock = 0; currInode.offsetInBlock = 0;
        fs_put_inode_by_num(0, currInode.id, &currInode);
        
        if(currInode.nextInodeNum < 0)
        {
            kprintf("Error in seeking!!\n");
            return SYSERR;
        }
        
        //go to the next inode..
        fs_get_inode_by_num(0, currInode.nextInodeNum, &currInode);
    }
    
    //update the last inode..
    currInode.currBlock = (int)(newPosition/fsd.blocksz);
    currInode.offsetInBlock = newPosition%fsd.blocksz;
    fs_put_inode_by_num(0, currInode.id, &currInode);
//    printf("CurrBlock: %d\n", currInode.currBlock);
  //  printf("offset in block: %d\n", currInode.offsetInBlock);
    
    //printf("Blocks in this inodes!!");
    int i=0; 
    /*for(;i <3; i++)
    {
      //  printf("--%d\n", currInode.blocks[i]);
    }*/
    
    return OK;
}

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEBLOCKS - 2) {
    printf("No indirect block support\n");
    return SYSERR;
  }

//  diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

  return diskblock;
}

/* read in an inode and fill in the pointer */
int
fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;
  int inode_off;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(struct inode);

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(in, &block_cache[inode_off], sizeof(struct inode));

  return OK;

}

int
fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
  int bl, inn;

  if (dev != 0) {
    printf("Unsupported device\n");
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}
     
int fs_mkfs(int dev, int num_inodes) {
  int i;
  
  if (dev == 0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  }
  else {
    printf("Unsupported device\n");
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  }
  else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) {i++;}
  fsd.freemaskbytes = i / 8; 
  
  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
    printf("fs_mkfs memget failed.\n");
    return SYSERR;
  }
  
  /* zero the free mask */
  for(i=0;i<fsd.freemaskbytes;i++) {
    fsd.freemask[i] = '\0';
  }
  
  fsd.inodes_used = 0;
  
  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
  
  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  return 1;
}

void
fs_print_fsd(void) {

  printf("fsd.ninodes: %d\n", fsd.ninodes);
  printf("sizeof(struct inode): %d\n", sizeof(struct inode));
  printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
  return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void) {
  int i,j;

  for (i=0; i < fsd.freemaskbytes; i++) {
    for (j=0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}

int getFreeInodeNum()
{
    int inodeNum = 0;
    for( ; inodeNum < fsd.ninodes; inodeNum++)
    {
        struct inode currInode;
        fs_get_inode_by_num(0, inodeNum, &currInode);
        if ( currInode.state == INODE_FREE )
        {
            return inodeNum;
        }
    }
    return -1;
}

int getFreeBlockNum()
{
    int blockNum = 0;
    for(; blockNum< fsd.nblocks; blockNum++)
    {
        if(fs_getmaskbit(blockNum) == 0)
        {
            return blockNum;
        }
    }
    return -1;
}

