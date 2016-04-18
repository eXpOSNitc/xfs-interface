#include "fileSystem.h"
/*
  This function returns an  empty fat entry if present.
  NOTE: The return address will be the relative word address corresponding to the filename entry in the basic block.
*/
int FindEmptyInodeEntry(){
	int i,j,entryFound = 0,entryNumber = 0;
	for(j = INODE ; j < INODE + NO_OF_INODE_BLOCKS ; j++){
		for(i = INODE_ENTRY_DATABLOCK; i < BLOCK_SIZE ; i = i + INODE_ENTRY_SIZE){
			if( getValue(disk[j].word[i]) == -1  ){
				entryNumber = (((j - INODE) * BLOCK_SIZE) + i);
				entryFound = 1;
				break;
			}
		}
		if(entryFound == 1)
			break;
	}
	if( entryNumber > INODE_SIZE ){
		printf("INODE  is full.\n");
		return -1;
	}
	return (entryNumber-INODE_ENTRY_DATABLOCK);
}



/*
  This function adds the name, size and basic block address of the file to corresponding entry in the fat.
  The first arguement is a relative address
*/
void AddEntryToMemInode(int startIndexInInode, int fileType, char *nameOfFile, int size_of_file, int* addrOfDataBlocks){
	int i,baseAddress=INODE * BLOCK_SIZE + startIndexInInode;
	storeValueAt( baseAddress + INODE_ENTRY_FILETYPE, fileType );
	storeStringValueAt( baseAddress + INODE_ENTRY_FILENAME, nameOfFile);
	storeValueAt( baseAddress + INODE_ENTRY_FILESIZE , size_of_file );
	for(i=0;i<INODE_NUM_DATA_BLOCKS;i++)
		storeValueAt( baseAddress + INODE_ENTRY_DATABLOCK + i , addrOfDataBlocks[i] );
}


/*
  This function removes the fat entry corresponding to the first arguement.
  NOTE: locationOfInode - relative word address of the name field in the fat.
  This is done as follows:
    1. The name field is set to empty string.
    2. The basic block entry is set to -1.
  The memory copy is not committed.
*/
int removeInodeEntry(int locationOfInode){
	int i;
	int blockNumber = INODE + locationOfInode / BLOCK_SIZE;
	int startWordNumber = locationOfInode % BLOCK_SIZE;
	storeValue(disk[blockNumber].word[startWordNumber + INODE_ENTRY_FILENAME], -1);
	for(i=0;i<INODE_NUM_DATA_BLOCKS;i++)
		storeValue(disk[blockNumber].word[startWordNumber + INODE_ENTRY_DATABLOCK + i], -1);
	storeValue(disk[blockNumber].word[startWordNumber + INODE_ENTRY_FILESIZE], 0);
	return 0;
}


/*
  This function returns the  data block entries(pass by pointer) corresponding to the address specified by the locationOfInode.
  Third argument specifies the type of file (assembly code or data file)
  NOTE: locationOfInode - relative word address of the name field in the fat.
*/
int getDataBlocks_(int *dataBlockAddr, int locationOfInode)
{
	
	int i,a;
	for(i=0;i<INODE_NUM_DATA_BLOCKS;i++)
		dataBlockAddr[i] = getValue(disk[INODE + locationOfInode / BLOCK_SIZE].word[locationOfInode % BLOCK_SIZE + INODE_ENTRY_DATABLOCK + i]);
	return 0;
}
