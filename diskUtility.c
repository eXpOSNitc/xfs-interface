#include "diskUtility.h"
#include "exception.h"

/*
 This function empties a block as specified by the first arguement in the memory copy of the disk file.
*/
void emptyBlock(int blockNo) 
{
	int i;
	for(i = 0 ; i < BLOCK_SIZE ; i++)
	{
		strcpy(disk[blockNo].word[i],"") ;
	}
}


/*
  This function frees the blocks specified by the block number present in the first arguement. The second arguement is the size
  of the first argument.
  The memory copy is not committed.
*/
void freeUnusedBlock(int *freeBlock, int size){
	int i=0;
	for( i = 0 ; i < size && freeBlock[i] != -1 && freeBlock[i] != 0; i++){
		//printf("Block Num = %d\nLocation = %d", freeBlock[i],freeBlock[i] % BLOCK_SIZE );
		storeValueAt(DISK_FREE_LIST * BLOCK_SIZE + freeBlock[i] , 0 );
		emptyBlock(TEMP_BLOCK);
		writeToDisk(TEMP_BLOCK,freeBlock[i]);
	}
}

/*
 This function reads an entire BLOCK from the address specified from fileBlockNumber on the disk file to virtBlockNumber on the memory copy of the disk.
*/
int readFromDisk(int virtBlockNumber, int fileBlockNumber) {
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0)
	{
		printf("Unable to Open Disk File\n");
		return -1;
	}
	lseek(fd,sizeof (BLOCK)*fileBlockNumber,SEEK_SET);
	read(fd,&disk[virtBlockNumber],sizeof (BLOCK));
	close(fd);
	return 0;
}


/*
  This function writes an entire block to fileBlocknumber on the disk file from virtBlockNumber on the memory copy of the disk.
*/
int writeToDisk(int virtBlockNumber, int fileBlockNumber) {
	int fd;
	fd = open(DISK_NAME, O_WRONLY, 0666);
	if(fd < 0)
	{
		printf("Unable to Open Disk File\n");
		return -1;
	}
	lseek(fd,0,SEEK_SET);
	lseek(fd,sizeof (BLOCK)*fileBlockNumber,SEEK_CUR);
	write(fd,&disk[virtBlockNumber],sizeof (BLOCK));
	close(fd);	
	return 0;
}


/*
  This function initialises the memory copy of the disk with the contents from the actual disk file.
*/
int loadFileToVirtualDisk()
{
	int i;
	for(i=DISK_FREE_LIST; i<DISK_FREE_LIST + NO_OF_FREE_LIST_BLOCKS; i++)
		readFromDisk(i,i);
	for(i=INODE; i<INODE + NO_OF_INODE_BLOCKS; i++)
		readFromDisk(i,i);
}

/*
  This function wipes out the entire contents of the memory copy of the disk.
*/
void clearVirtDisk()
{
	bzero(disk, sizeof(disk));
}

int openDiskFile()
{
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0){
	  exception_throwException(EXCEPTION_CANT_OPEN_DISK);
	}
	return fd;
}

/*
	Tries to open the disk file, throws an exception if it fails.
	An exception returns the control to the jmppoint set in interface.c
*/
void diskFileExists()
{
	close(openDiskFile());
}


/*
  char* to int conversion
  get integer value at address
*/
int getValueAt(int address)
{
	getValue( disk[(address / BLOCK_SIZE)].word[(address % BLOCK_SIZE)]);
}

/*
  char* to int conversion
 */

int getValue(char* str ) 
{
	return atoi(str);
}

/*
  int to char* conversion
*/
void storeValueAt(int address, int num) 
{
	storeValue( disk[(address / BLOCK_SIZE)].word[(address % BLOCK_SIZE)] , num );
}

void storeValue(char *str, int num) 
{
	sprintf(str,"%d",num);
}

void storeStringValueAt(int address, char *value) 
{
	strcpy( disk[(address / BLOCK_SIZE)].word[(address % BLOCK_SIZE)] , value );
}