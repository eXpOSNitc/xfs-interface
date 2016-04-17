#ifndef FILESYS_H
#define FILESYS_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "disk.h"

/*
Disk Organization
*/

#define BLOCK_SIZE 512
#define WORD_SIZE 16
#define OS_STARTUP_CODE 0
#define DISK_FREE_LIST 2
#define INODE 3
#define ROOT_FILE 5
#define INIT_BASIC_BLOCK 7
#define SHELL_BASIC_BLOCK 9
#define LIBRARY 13
#define EX_HANDLER 15
#define TIMERINT 17
#define DISKCONTROLLER_INT 19
#define CONSOLE_INT 21
#define INT0 EX_HANDLER
#define INT1 TIMERINT
#define INT2 DISKCONTROLLER_INT
#define INT3 CONSOLE_INT
#define INT4 23
#define INT5 25
#define INT6 27
#define INT7 29
#define INT8 31
#define INT9 33
#define INT10 35
#define INT11 37
#define INT12 39
#define INT13 41
#define INT14 43
#define INT15 45
#define INT16 47
#define INT17 49
#define INT18 51
#define MOD0 53
#define MOD1 55
#define MOD2 57
#define MOD3 59
#define MOD4 61
#define MOD5 63
#define MOD6 65
#define MOD7 67


#define OS_STARTUP_CODE_SIZE 2
#define NO_OF_FREE_LIST_BLOCKS 1
#define ROOT_FILE_SIZE 1
#define NO_OF_INIT_BLOCKS 2
#define NO_OF_SHELL_BLOCKS 2
#define NO_OF_LIBRARY_BLOCKS 13
#define EX_HANDLER_SIZE 2
#define TIMERINT_SIZE 2
#define DISKCONTROLLER_INT_SIZE 2
#define CONSOLE_INT_SIZE 2
#define INT0_SIZE EX_HANDLER_SIZE
#define INT1_SIZE TIMERINT_SIZE
#define INT2_SIZE DISKCONTROLLER_INT_SIZE
#define INT3_SIZE CONSOLE_INT_SIZE
#define INT4_SIZE 2
#define INT5_SIZE 2
#define INT6_SIZE 2
#define INT7_SIZE 2
#define INT8_SIZE 2
#define INT9_SIZE 2
#define INT10_SIZE 2
#define INT11_SIZE 2
#define INT12_SIZE 2
#define INT13_SIZE 2
#define INT14_SIZE 2
#define INT15_SIZE 2
#define INT16_SIZE 2
#define INT17_SIZE 2
#define INT18_SIZE 2
#define MOD0_SIZE 2
#define MOD1_SIZE 2
#define MOD2_SIZE 2
#define MOD3_SIZE 2
#define MOD4_SIZE 2
#define MOD5_SIZE 2
#define MOD6_SIZE 2
#define MOD7_SIZE 2

#define INODE 3
#define NO_OF_INODE_BLOCKS 2


#define INIT_NAME "init.xsm"

#define NO_OF_INTERRUPTS 18

#define DATA_START_BLOCK 69
#define NO_OF_DATA_BLOCKS 187

#define SWAP_START_BLOCK 256
#define NO_OF_SWAP_BLOCKS 256

#define NO_OF_DISK_BLOCKS 512

#define DISK_SIZE (NO_OF_DISK_BLOCKS * BLOCK_SIZE)


/*
Declarations for INODE Entry
*/ 

#define INODE_MAX_FILE_NUM 60
#define INODE_MAX_BLOCK_NUM 4

#define INODEENTRY_FILETYPE 0
#define INODEENTRY_FILENAME 1
#define INODEENTRY_FILESIZE 2
#define INODEENTRY_USERID 3
#define INODEENTRY_PERMISSION 4
#define INODEENTRY_DATABLOCK 8
#define INODE_NUM_DATA_BLOCKS 4
#define INODEENTRY_SIZE 16
#define INODE_SIZE (NO_OF_INODE_BLOCKS * BLOCK_SIZE)


/*
Other declarations
*/

#define NO_BLOCKS_TO_COPY 69        //Rest of the blocks have data. 
#define EXTRA_BLOCKS	1			// Need a temporary block
#define TEMP_BLOCK 69				//Temporary block no: starting from 0.

#define ASSEMBLY_CODE 0
#define DATA_FILE 1

#define FILETYPE_ROOT 1
#define FILETYPE_DATA 2
#define FILETYPE_EXEC 3

#define XFS_ERROR -1

typedef struct{
	char word[BLOCK_SIZE][WORD_SIZE];
}BLOCK;

BLOCK disk[NO_BLOCKS_TO_COPY + EXTRA_BLOCKS];			// disk contains the memory copy of the necessary blocks of the actual disk file.

typedef struct _XOSFILE
{
  char *name;
  int size;

  struct _XOSFILE *next;
} XOSFILE;


XOSFILE* getAllFiles();

/*
  This function lists all the files present on the disk.
*/
void listAllFiles();

/*
  This function deletes an executable file from the disk.
*/
int deleteExecutableFromDisk(char *name);

/*
  This function removes the inode entry corresponding to the first arguement.
*/
int removeInodeEntry(int locationOfInode);

/*
  This function returns the basic block entry(pass by pointer) corresponding to the address specified by the second arguement.
*/
int getDataBlocks(int *basicBlockAddr, int locationOfInode);

/*
  This function loads the executable file corresponding to the first arguement to an appropriate location on the disk.
*/
int loadExecutableToDisk(char *name);

/*
  This function checks if a file having name as the first arguement is present on  the disk file.
*/
int CheckRepeatedName(char *name);

/*
  This function returns the address of a free block on the disk.
*/
int FindFreeBlock();

/*
  This function returns an  empty inode entry if present.
*/
int FindEmptyInodeEntry();

/*
  This function frees the blocks specified by the block numbers present in the first arguement. The second arguement is the size
  of the first argument.
*/
void FreeUnusedBlock(int *freeBlock, int size);

/*
  This function adds the name, size and basic block address of the file to corresponding entry in the inode.
*/
void AddEntryToMemInode(int startIndexInInode, int fileType, char *nameOfFile, int size_of_file, int* addrOfDataBlocks);

/*
  This function copies the necessary contents of a file to the corresponding location specified by the second arguemnt on the disk. The type specifies the type of file 
  to be copied.
*/
int writeFileToDisk(FILE *f, int blockNum, int type);

/*
  This function loads the OS startup code specified by the first arguement to its appropriate location on disk.
*/
int loadOSCode(char* name);

/*
  This function copies the interrupts to the proper location on the disk.
*/
int loadIntCode(char* name, int intNo);

/*
  This function copies the timer interrupt to the proper location on the disk.
*/
int loadTimerCode(char* name);

int loadDiskControllerIntCode(char* name);

int loadConsoleIntCode(char* name);

/*
  This function copies the exception handler to the proper location on the disk.
*/
int loadExHandlerToDisk(char* name);

/*
  This function copies the init program to its proper location on the disk.
*/
int loadINITCode(char* name);

/*
  This function displays the content of the files stored in the disk.
*/
void displayFileContents(char *name);

/*
  This function copies the contents of the disk starting from <startBlock> to <endBlock> to a unix file.
*/
void copyBlocksToFile (int startBlock,int endBlock,char *name);

/*
  This function deletes the INIT code from the disk.
*/
int deleteINITFromDisk();

/*
  This function deletes the OS code from the disk.
*/
int deleteOSCodeFromDisk();

/*
  This function deletes the Timer Interrupt from the disk.
*/
int deleteTimerFromDisk();

int deleteDiskControllerINTFromDisk();

int deleteConsoleINTFromDisk();


/*
  This function deletes the Interrupt <intNo> from the disk.
*/
int deleteIntCode(int intNo);	

/*
  This function deletes the Exception Handler from the disk.
*/
int deleteExHandlerFromDisk();

/*
  This function displays disk free list and the amount of free space in the disk.
*/
void displayDiskFreeList();

/*
  This function loads a data file to the disk.
*/
int loadDataToDisk(char *name);

/*
  This function deletes a data file from the disk.
*/
int deleteDataFromDisk(char *name);

/*
  This function expands environment variables in path
*/
void expandpath(char *path);

/*
  This function adds extensions (.dat or .xsm) for files loaded into xfs
*/
void addext(char *filename, char *ext);
#endif
