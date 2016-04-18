#ifndef DISKUTILS_H
#define DISKUTILS_H
#include <stdio.h>
#include <string.h>
#include<fcntl.h>
#include "fileSystem.h"

void emptyBlock(int blockNo);
int getValue(char* str);
int getValueAt(int address);
void storeValue(char* str, int num);
void storeValueAt(int address, int num);
void storeStringValueAt(int address, char *value);
int readFromDisk(int virtBlockNumber, int fileBlockNumber);
int writeToDisk(int virtBlockNumber, int fileBlockNumber);
int loadFileToVirtualDisk();
void clearVirtDisk();
void diskFileExists();
/*
  This function frees the blocks specified by the block numbers present in the first arguement. The second arguement is the size
  of the first argument.
*/
void FreeUnusedBlock(int *freeBlock, int size);
#endif
