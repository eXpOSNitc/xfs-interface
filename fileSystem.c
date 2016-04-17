#include "fileSystem.h"
#include "memOrg.h"
#include<stdio.h>
#include<stdlib.h>


void
listAllFiles ()
{
	XOSFILE *list, *next;

	list = getAllFiles();

	if (!list)
	{
		printf("The disk contains no file or the disk does not exist.\n");
		return;
	}

	while (list)
	{
		printf ("Filename: %s Filesize %d\n", list->name, list->size);
		next = list->next;

		free (list->name);
		free(list);
		list = next;
	}
}
/*
  This function lists all the files present on the disk.
  This is done as follows:
    1. The basic block entry in the memory copy of the disk is searched. If the value is not -1 then the filename is 
      shown as output.
*/
XOSFILE* getAllFiles(){
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0){
	  fprintf(stderr, "Unable to open disk file.\n");
	  return NULL;
	}
	close(fd);
	int i,j;
	XOSFILE *list, *curr_ptr;
	int hasFiles = 0; 	// Flag which indicates if disk has no files

	/* The list works as a sentinel. */
	list = malloc(sizeof(XOSFILE));
	list->next = NULL;
	curr_ptr = list;

	for(j = INODE ; j < INODE + NO_OF_INODE_BLOCKS ; j++)
	{
		for(i = 0 ; i < BLOCK_SIZE ; i = i + INODEENTRY_SIZE)
		{
			if( getValue(disk[j].word[INODEENTRY_DATABLOCK + i]) > 0 )	// Negative value indicates invalid INODE
			{ 	
				hasFiles = 1;
				XOSFILE *new_entry;

				new_entry = malloc (sizeof(XOSFILE));
				new_entry->name = strdup(disk[j].word[i + INODEENTRY_FILENAME]);
				new_entry->size = getValue(disk[j].word[i + INODEENTRY_FILESIZE]);
				curr_ptr->next = new_entry;
				curr_ptr = new_entry;
				curr_ptr->next = NULL;
			}		
		}
	}
	
	curr_ptr = list->next;
	free(list);
	return curr_ptr;
}


/*
  This function checks if a file having name as the first arguement is present on the disk file.
  This is done as follows:
    1. It checks the entry in the fat block. If a file with same name exists then the function returns the relative word
      address of the entry having the same name.
*/
int CheckRepeatedName(char *name){
	int i,j;
	for(j = INODE ; j < INODE + NO_OF_INODE_BLOCKS ; j++)
	{
		for(i = INODEENTRY_FILENAME ; i < BLOCK_SIZE ; i = i + INODEENTRY_SIZE)
		{
			if(strcmp(disk[j].word[i],name) == 0 && getValue(disk[j].word[i]) != -1)		
				return (((j - INODE) * BLOCK_SIZE) + i);
		}
	}
	return (((j - INODE)* BLOCK_SIZE) + i);
}



/*
  This function returns the basic block entry(pass by pointer) corresponding to the address specified by the second arguement.
  Third argument specifies the type of file (assembly code or data file)
  NOTE: locationOfInode - relative word address of the name field in the fat.


int getDataBlocks(int *basicBlockAddr, int locationOfInode)
{
	
	int i,a;
	basicBlockAddr[0] = getValue(disk[INODE + locationOfInode / BLOCK_SIZE].word[locationOfInode % BLOCK_SIZE + INODEENTRY_BASICBLOCK]);
	emptyBlock(TEMP_BLOCK);
	//printf("Basic Block = %d\n",basicBlockAddr[0]);
	readFromDisk(TEMP_BLOCK,basicBlockAddr[0]);
	
	i = 0;
	a = getValue(disk[TEMP_BLOCK].word[i]);
	while ((a > 0) && i < INODE_MAX_BLOCK_NUM)
	{
		//printf("%d %d\t",i+1,a);
		basicBlockAddr[i+1] = a;
		i++;
		a = getValue(disk[TEMP_BLOCK].word[i]);
	}
	
	return 0;
}
*/
/*
  This function returns the  data block entries(pass by pointer) corresponding to the address specified by the locationOfInode.
  Third argument specifies the type of file (assembly code or data file)
  NOTE: locationOfInode - relative word address of the name field in the fat.
*/
int getDataBlocks_(int *dataBlockAddr, int locationOfInode)
{
	
	int i,a;
	for(i=0;i<INODE_NUM_DATA_BLOCKS;i++)
		dataBlockAddr[i] = getValue(disk[INODE + locationOfInode / BLOCK_SIZE].word[locationOfInode % BLOCK_SIZE + INODEENTRY_DATABLOCK + i]);
	return 0;
}


/*
  This function frees the blocks specified by the block number present in the first arguement. The second arguement is the size
  of the first argument.
  The memory copy is not committed.
*/
void FreeUnusedBlock(int *freeBlock, int size){
	int i=0;
	for( i = 0 ; i < size && freeBlock[i] != -1 && freeBlock[i] != 0; i++){
		//printf("Block Num = %d\nLocation = %d", freeBlock[i],freeBlock[i] % BLOCK_SIZE );
		storeValue( disk[DISK_FREE_LIST + freeBlock[i] / BLOCK_SIZE].word[freeBlock[i] % BLOCK_SIZE] , 0 );
		emptyBlock(TEMP_BLOCK);
		writeToDisk(TEMP_BLOCK,freeBlock[i]);
	}
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
	storeValue(disk[blockNumber].word[startWordNumber + INODEENTRY_FILENAME], -1);
	for(i=0;i<INODE_NUM_DATA_BLOCKS;i++)
		storeValue(disk[blockNumber].word[startWordNumber + INODEENTRY_DATABLOCK + i], -1);
	storeValue(disk[blockNumber].word[startWordNumber + INODEENTRY_FILESIZE], 0);
	return 0;
}


/*
  This function deletes an executable file from the disk.
  NOTE: 1. Memory copy is committed to disk.
	2. Due to a technical glitch any string which is already stored on the disk will have to be searched in the
	  memory copy after appending a newline.
  
*/
int deleteExecutableFromDisk(char *name)
{
	int locationOfInode,i,blockAddresses[INODE_MAX_BLOCK_NUM];   //0-basic block , 1,2,3-code+data blocks
	for(i=0;i<INODE_MAX_BLOCK_NUM;i++)
		blockAddresses[i]=0;
	locationOfInode = CheckRepeatedName(name);
	if(locationOfInode >= INODE_SIZE){
		printf("File \'%s\' not found!\n",name);
		return -1;
	}
	if(strstr(name,".xsm") == NULL)
	{
		printf("\'%s\' is not a valid executable file!\n",name);
		return -1;
	}
	
	getDataBlocks_(blockAddresses,locationOfInode);		
	FreeUnusedBlock(blockAddresses, INODE_MAX_BLOCK_NUM);
	removeInodeEntry(locationOfInode);
	for(i = INODE ; i < INODE + NO_OF_INODE_BLOCKS ; i++){
		writeToDisk(i,i);
	}
	for( i=DISK_FREE_LIST ; i<DISK_FREE_LIST + NO_OF_FREE_LIST_BLOCKS; i++)
		writeToDisk(i,i);
	
	return 0;	
}

/*
  This function deletes a data file from the disk.
*/
int deleteDataFromDisk(char *name)
{
	int locationOfInode,i,blockAddresses[INODE_MAX_BLOCK_NUM+1];
	for(i=0;i<INODE_MAX_BLOCK_NUM;i++)
		blockAddresses[i]=0;   
	locationOfInode = CheckRepeatedName(name);
	if(locationOfInode >= INODE_SIZE)
	{
		printf("File \'%s\' not found!\n",name);
		return -1;
	}
	if(strstr(name,".dat") == NULL)
	{
		printf("\'%s\' is not a valid data file!\n",name);
		return -1;
	}
	
	getDataBlocks_(blockAddresses,locationOfInode);		
	FreeUnusedBlock(blockAddresses, INODE_MAX_BLOCK_NUM);
	removeInodeEntry(locationOfInode);
	for(i = INODE ; i < INODE + NO_OF_INODE_BLOCKS ; i++){
		writeToDisk(i,i);
	}
	for( i=DISK_FREE_LIST ; i<DISK_FREE_LIST + NO_OF_FREE_LIST_BLOCKS; i++)
		writeToDisk(i,i);
	return 0;
}



/*
  This function deletes the INIT code from the disk.
*/
int deleteINITFromDisk()
{
	emptyBlock(TEMP_BLOCK);
	int i;
	for (i=0; i<NO_OF_INIT_BLOCKS; i++)
		writeToDisk(TEMP_BLOCK,INIT_BASIC_BLOCK+i);	

	//writeToDisk(TEMP_BLOCK,INIT_BASIC_BLOCK);
	//writeToDisk(TEMP_BLOCK,INIT_BASIC_BLOCK+1);
	//writeToDisk(TEMP_BLOCK,INIT_BASIC_BLOCK+2);
	return 0;
}

/*
  This function deletes the OS code from the disk.
*/
int deleteOSCodeFromDisk()
{
	emptyBlock(TEMP_BLOCK);
	int i;
	for (i=0;i<OS_STARTUP_CODE_SIZE; i++)
		writeToDisk(TEMP_BLOCK,OS_STARTUP_CODE+i);
	return 0;
}

/*
  This function deletes the Timer Interrupt from the disk.
*/
int deleteTimerFromDisk()
{
	emptyBlock(TEMP_BLOCK);
	int i;
	for (i=0;i<TIMERINT_SIZE; i++)
		writeToDisk(TEMP_BLOCK,TIMERINT+i);
	return 0;
}

int deleteDiskControllerINTFromDisk()
{
	emptyBlock(TEMP_BLOCK);
	int i;
	for (i=0;i<DISKCONTROLLER_INT_SIZE; i++)
		writeToDisk(TEMP_BLOCK,DISKCONTROLLER_INT+i);
	return 0;
}

int deleteConsoleINTFromDisk()
{
	emptyBlock(TEMP_BLOCK);
	int i;
	for (i=0;i<CONSOLE_INT_SIZE; i++)
		writeToDisk(TEMP_BLOCK,CONSOLE_INT+i);
	return 0;
}
/*
  This function deletes the Interrupt <intNo> from the disk.
*/
int deleteIntCode(int intNo)
{
	emptyBlock(TEMP_BLOCK);
	int i;
	for (i=0;i<INT1_SIZE; i++)
		writeToDisk(TEMP_BLOCK,((intNo - 1) * INT1_SIZE)  + INT1 + i);
	return 0;
}

/*
  This function deletes the Exception Handler from the disk.
*/
int deleteExHandlerFromDisk()
{
	emptyBlock(TEMP_BLOCK);
	int i;
	for (i=0;i<EX_HANDLER_SIZE; i++)
		writeToDisk(TEMP_BLOCK,EX_HANDLER + i);
	return 0;
}

/*
  This function returns the address of a free block on the disk.
  The value returned will be the relative word address of the corresponding entry in the free list.
*/
int FindFreeBlock(){
	int i,j;
	for(i = DISK_FREE_LIST ; i < DISK_FREE_LIST + NO_OF_FREE_LIST_BLOCKS ;i++){
		for(j = 0 ; j < BLOCK_SIZE; j++){
			if( getValue(disk[i].word[j]) == 0 ){
				storeValue( disk[i].word[j] , 1 );	
				return ((i-DISK_FREE_LIST)*BLOCK_SIZE + j);
			}
		}
	}
	return -1;	
}




/*
  This function returns an  empty fat entry if present.
  NOTE: The return address will be the relative word address corresponding to the filename entry in the basic block.
*/
int FindEmptyInodeEntry(){
	int i,j,entryFound = 0,entryNumber = 0;
	for(j = INODE ; j < INODE + NO_OF_INODE_BLOCKS ; j++){
		for(i = INODEENTRY_DATABLOCK; i < BLOCK_SIZE ; i = i + INODEENTRY_SIZE){
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
	return (entryNumber-INODEENTRY_DATABLOCK);
}



/*
  This function adds the name, size and basic block address of the file to corresponding entry in the fat.
  The first arguement is a relative address
*/
void AddEntryToMemInode(int startIndexInInode, int fileType, char *nameOfFile, int size_of_file, int* addrOfDataBlocks){
	int i;
	storeValue( disk[INODE + (startIndexInInode / BLOCK_SIZE)].word[startIndexInInode % BLOCK_SIZE + INODEENTRY_FILETYPE] , fileType );
	strcpy(disk[INODE + (startIndexInInode / BLOCK_SIZE)].word[startIndexInInode % BLOCK_SIZE + INODEENTRY_FILENAME],nameOfFile);
	storeValue( disk[INODE + (startIndexInInode / BLOCK_SIZE)].word[startIndexInInode % BLOCK_SIZE + INODEENTRY_FILESIZE] , size_of_file );
	for(i=0;i<INODE_NUM_DATA_BLOCKS;i++)
		storeValue( disk[INODE + (startIndexInInode / BLOCK_SIZE)].word[startIndexInInode % BLOCK_SIZE + INODEENTRY_DATABLOCK + i] , addrOfDataBlocks[i] );
}



/*
  This file copies the necessary contents of a file to the corresponding location specified by the second arguemnt on the disk.
  The file is first copied to the memory copy of the disk. This is then committed to the actual disk file.
  NOTE: 1. EOF is set only after reading beyond the end of the file. This is the reason why the if condition is needed is needed.
	2. Also the function must read till EOF or BLOCK_SIZE line so that successive read proceeds accordingly
*/
int writeFileToDisk(FILE *f, int blockNum, int type)
{
	int i, line=0,j;
	char buffer[32],s[16],temp[100],c;
	emptyBlock(TEMP_BLOCK);
	if(type==0)			//writing files with assembly code
	{
		char *instr, *arg1, *arg2, *string_start;
		int line_count=0,flag=0,k=0;
		for(i = 0; i < (BLOCK_SIZE/2); i++)
		{
			fgets(temp,100,f);
			
			string_start=strchr(temp,'"');
			if(string_start==NULL)
			{
				for(k=0;k<31;k++)
					buffer[k]=temp[k];
				buffer[k]='\0';
			}
			else
			{
				if(strlen(string_start)<=16)
				{
					for(k=0;k<31;k++)
						buffer[k]=temp[k];
					buffer[k]='\0';
				}
				else
				{
					for(k=0;k<(strlen(temp)-strlen(string_start)+15);k++)
					{
						buffer[k]=temp[k];
					}
					buffer[k-1]='"';
					buffer[k]='\0';
				}
			}
		
			
			if(strlen(buffer)>3)
			{
				if(buffer[strlen(buffer)-1]=='\n')
					buffer[strlen(buffer)-1]='\0';
				instr=strtok(buffer," ");
				arg1=strtok(NULL," ");
				arg2=strtok(NULL,",");
			
				bzero(s,16);
				if(arg1!=NULL)
				{
					sprintf(s,"%s %s",instr,arg1);
					for(j=strlen(s);j<16;j++)
						s[j]='\0';
					strcpy(disk[TEMP_BLOCK].word[line_count],s);
					if(arg2!=NULL)
					{
						strcpy(s,arg2);
						for(j=strlen(s);j<16;j++)
							s[j]='\0';
						strcpy(disk[TEMP_BLOCK].word[line_count+1],s);
				
					}
					else
					{
						for(j=0;j<16;j++)
							s[j]='\0';
						strcpy(disk[TEMP_BLOCK].word[line_count+1],s);
					}
					line_count=line_count+2;
				}
				else
				{
					sprintf(s,"%s",instr);
					for(j=strlen(s);j<=16;j++)
						strcat(s,"\0");
					strcpy(disk[TEMP_BLOCK].word[line_count],s);
					bzero(s,16);
					for(j=0;j<16;j++)
						s[j]='\0';
					strcpy(disk[TEMP_BLOCK].word[line_count+1],s);
					line_count=line_count+2;
			
				}
			
			}
			
			 if(feof(f)){
				strcpy(disk[TEMP_BLOCK].word[line_count], "");
				writeToDisk(TEMP_BLOCK,blockNum);
				return -1;
			 }
			
		}
		writeToDisk(TEMP_BLOCK,blockNum);
		return 1;
	}	
	else if(type==1)			//writing data files
	{
		char buffer1[16],c;
		for(i = 0; i < BLOCK_SIZE; i++)
		{
			fgets(buffer1,16,f);
			strcpy(disk[TEMP_BLOCK].word[i],buffer1);
			if(feof(f))
			{
				strcpy(disk[TEMP_BLOCK].word[i], "");
				writeToDisk(TEMP_BLOCK,blockNum);
				return -1;
			}	
		}
		writeToDisk(TEMP_BLOCK,blockNum);
		return 1;
	}

}

int
fs_load_exec (const char *fname)
{
	/* Determine whether we have space in the root file or not. */
	int n_files = 0;//inode_get_count ();
	char temp_file[66];

	if (n_files >= INODE_MAX_FILE_NUM)
	{
		fprintf (stderr, "Disk full.\n");
		return XFS_ERROR;
	}
}




/*
  This function loads the executable file corresponding to the first arguement to an appropriate location on the disk.
  This function systematically uses the above functions to do this action.
*/
int loadExecutableToDisk(char *name)
{
	FILE *fileToBeLoaded;
	int freeBlock[INODE_MAX_BLOCK_NUM];
	int i,j,k,l,file_size=0,num_of_lines=0,num_of_blocks_reqd=0;
	for(i=0;i<INODE_MAX_BLOCK_NUM;i++)
		freeBlock[i]=-1;
	char c='\0',*s;
	char filename[50];
	s = strrchr(name,'/');
	if(s!=NULL)
		strcpy(filename,s+1);
	else
		strcpy(filename,name);	
	
	filename[15]='\0';
		
	addext(filename,".xsm");

	expandpath(name);
	fileToBeLoaded = fopen(name, "r");
	if(fileToBeLoaded == NULL){
	    printf("File %s not found.\n", name);
	    return -1;
	  }
	if(fileToBeLoaded == NULL){
		printf("The file could not be opened");
		return -1;
	}
	
	while(c!=EOF)
	{
		c=fgetc(fileToBeLoaded);
		if(c=='\n')
			num_of_lines++;
	}
	
	num_of_blocks_reqd = (num_of_lines / (BLOCK_SIZE/2)) + 1;
	
	if(num_of_blocks_reqd > INODE_MAX_BLOCK_NUM)
	{
		printf("The size of file exceeds %d blocks",INODE_MAX_BLOCK_NUM);
		return -1;
	}
	
	fseek(fileToBeLoaded,0,SEEK_SET);
	
	for(i = 0; i < num_of_blocks_reqd + 1; i++)
	{
		if((freeBlock[i] = FindFreeBlock()) == -1){
				printf("Insufficient disk space!\n");
				FreeUnusedBlock(freeBlock, INODE_MAX_BLOCK_NUM);
				return -1;
			}
	}
	i = CheckRepeatedName(filename);
	if( i < INODE_SIZE ){
		printf("Disk already contains the file with this name. Try again with a different name.\n");
		FreeUnusedBlock(freeBlock, INODE_MAX_BLOCK_NUM);
		return -1;
	}
	
	k = FindEmptyInodeEntry();		
	if( k == -1 ){
		FreeUnusedBlock(freeBlock, INODE_MAX_BLOCK_NUM);
		printf("No free INODE entry found.\n");
		return -1;			
	}
	
	
	for(i = DISK_FREE_LIST ;i < DISK_FREE_LIST + NO_OF_FREE_LIST_BLOCKS; i++)		//updating disk free list in disk
		writeToDisk(i, i);
	emptyBlock(TEMP_BLOCK);				//note:need to modify this
	
	for(i=0;i<num_of_blocks_reqd;i++)
	{
		j = writeFileToDisk(fileToBeLoaded, freeBlock[i], ASSEMBLY_CODE);
		file_size++;
	}
	


	AddEntryToMemInode(k, FILETYPE_EXEC,filename, file_size * BLOCK_SIZE, freeBlock);	
	for(i = INODE; i < INODE + NO_OF_INODE_BLOCKS ; i++){
		writeToDisk(i,i);				
	}
	
      close(fileToBeLoaded);
      return 0;
}



/*
  This function loads a data file to the disk.
*/
int loadDataToDisk(char *name)
{
	FILE *fileToBeLoaded;
	int freeBlock[INODE_MAX_BLOCK_NUM];
	int i,j,k,num_of_chars=0,num_of_blocks_reqd=0,file_size=0,num_of_words=0;
	for(i=0;i<INODE_MAX_BLOCK_NUM;i++)
		freeBlock[i]=-1;
	char c='\0',*s;
	char filename[50],buf[16];
	s = strrchr(name,'/');
	if(s!=NULL)
		strcpy(filename,s+1);
	else
		strcpy(filename,name);	
	
	filename[15]='\0';
	addext(filename,".dat");

	expandpath(name);
	fileToBeLoaded = fopen(name, "r");
	if(fileToBeLoaded == NULL)
	{
		printf("File \'%s\' not found.!\n", name);
		return -1;
	}
	if(fileToBeLoaded == NULL)
	{
		printf("The file could not be opened!");
		return -1;
	}
	
	fseek(fileToBeLoaded, 0L, SEEK_END);
	
	num_of_chars = ftell(fileToBeLoaded);
	
	fseek(fileToBeLoaded,0,SEEK_SET);
	while(1)
	{
		fgets(buf,16,fileToBeLoaded);
		num_of_words++;
		if(feof(fileToBeLoaded))
			break;
	}
	num_of_blocks_reqd = (num_of_words/512) + 1;
	//printf("\n Chars = %d, Words = %d, Blocks(chars) = %d, Blocks(words) = %d",num_of_chars,num_of_words,num_of_blocks_reqd,(num_of_words/512));
	if(num_of_blocks_reqd > INODE_MAX_BLOCK_NUM)
	{
		printf("The size of file exceeds %d blocks",INODE_MAX_BLOCK_NUM);
		return -1;
	}
	
	fseek(fileToBeLoaded,0,SEEK_SET);
	
	for(i = 0; i < num_of_blocks_reqd; i++)
	{
		if((freeBlock[i] = FindFreeBlock()) == -1){
				printf("not sufficient space in disk to hold a new file.\n");
				FreeUnusedBlock(freeBlock, INODE_MAX_BLOCK_NUM);
				return -1;
			}
	}
	i = CheckRepeatedName(filename);
	if( i < INODE_SIZE )
	{
		printf("Disk already contains the file with this name. Try again with a different name.\n");
		FreeUnusedBlock(freeBlock, INODE_MAX_BLOCK_NUM);
		return -1;
	}
	
	k = FindEmptyInodeEntry();		
	if( k == -1 )
	{
		FreeUnusedBlock(freeBlock, INODE_MAX_BLOCK_NUM);
		printf("No free INODE entry found.\n");
		return -1;			
	}
	
	
	for(i = DISK_FREE_LIST ;i < DISK_FREE_LIST + NO_OF_FREE_LIST_BLOCKS; i++)		//updating disk free list in disk
		writeToDisk(i, i);
	emptyBlock(TEMP_BLOCK);				//note:need to modify this
	
	
	for(i=0;i<num_of_blocks_reqd;i++)//load the file
	{
		j = writeFileToDisk(fileToBeLoaded, freeBlock[i], DATA_FILE);
		file_size++;
	}
	
	  
	AddEntryToMemInode(k, FILETYPE_DATA, filename, file_size * BLOCK_SIZE, freeBlock);		
	for(i = INODE; i < INODE + NO_OF_INODE_BLOCKS ; i++){
		writeToDisk(i,i);				//updating disk fat entry note:check for correctness
	}
	
      close(fileToBeLoaded);
      return 0;
	
}


/*
  This function copies the init program to its proper location on the disk.
*/
int loadINITCode(char* infile )
{
	FILE * fp;
	int i,j;
	expandpath(infile);
	char fileName[66];
	
	labels_reset ();	
	labels_resolve (infile, fileName, MEM_INIT_BASIC_BLOCK * PAGE_SIZE);
	
	fp = fopen(fileName, "r");
	if(fp == NULL)
	{
		printf("File \'%s\' not found.\n", fileName);
		return -1;
	}
	
	for(i=0; i<NO_OF_INIT_BLOCKS; i++)
	{
		j = writeFileToDisk(fp, INIT_BASIC_BLOCK + i, ASSEMBLY_CODE);
		if(j != 1)
			break;
	}
	//j = writeFileToDisk(fp, INIT_BASIC_BLOCK, ASSEMBLY_CODE);		//writing executable file to disk
	
	//if(j == 1)
	//	j = writeFileToDisk(fp, INIT_BASIC_BLOCK + 1, ASSEMBLY_CODE);		//if the file is longer than one page.  
	//if(j == 1)
		//writeFileToDisk(fp, INIT_BASIC_BLOCK + 2, ASSEMBLY_CODE);
	close(fp);
	return 0;
  
}



/*
  This function loads the OS startup code specified by the first argument to its appropriate location on disk.
  The code is first copied to memory copy. If this copying proceeds properly then the memory copy is committed to the disk.
*/
int loadOSCode(char* infile){

	emptyBlock(TEMP_BLOCK);
	writeToDisk(TEMP_BLOCK,OS_STARTUP_CODE);
	expandpath(infile);
	
	char fileName[66];
	
	labels_reset ();	
	labels_resolve (infile, fileName, MEM_OS_STARTUP_CODE * PAGE_SIZE);
	
	FILE* fp = fopen(fileName, "r");
	
	int i,j;
	if(fp == NULL)
	{
		printf("File \'%s\' not found.\n", fileName);
		return -1;
	}
	
	for(i=0;i<OS_STARTUP_CODE_SIZE;i++)
	{
		j = writeFileToDisk(fp, OS_STARTUP_CODE + i, ASSEMBLY_CODE);
		if (j != 1)
			break;
	}
	if(j==1)
	{
		printf("OS Code exceeds %d block\n",OS_STARTUP_CODE_SIZE);
		deleteOSCodeFromDisk();
		//emptyBlock(TEMP_BLOCK);
		//writeToDisk(TEMP_BLOCK,OS_STARTUP_CODE);
	}
	close(fp);
	return 0;
}

int loadDiskControllerIntCode(char* infile)
{
	expandpath(infile);
	
	char fileName[66];
	
	labels_reset ();	
	labels_resolve (infile, fileName, MEM_DISKCONTROLLER_INT * PAGE_SIZE);
	
	FILE* fp = fopen(fileName, "r");
	int i,j;
	if(fp == NULL)
	{
		printf("File %s not found.\n", fileName);
		return -1;
	}
	
	for(i=0;i<DISKCONTROLLER_INT_SIZE;i++)
	{
		j = writeFileToDisk(fp, DISKCONTROLLER_INT + i, ASSEMBLY_CODE);
		if (j != 1)
			break;
	}
	if(j==1)
	{
		printf("Disk Controller Interrupt Code exceeds %d block\n",DISKCONTROLLER_INT_SIZE);
		deleteDiskControllerINTFromDisk();
		//emptyBlock(TEMP_BLOCK);
		//writeToDisk(TEMP_BLOCK,TIMERINT);
	}
	close(fp);
	return 0;
}

int loadConsoleIntCode(char* infile)
{
	expandpath(infile);
	
	char fileName[66];
	
	labels_reset ();	
	labels_resolve (infile, fileName, MEM_CONSOLE_INT * PAGE_SIZE);
	
	FILE* fp = fopen(fileName, "r");
	int i,j;
	if(fp == NULL)
	{
		printf("File %s not found.\n", fileName);
		return -1;
	}
	
	for(i=0;i<CONSOLE_INT_SIZE;i++)
	{
		j = writeFileToDisk(fp, CONSOLE_INT + i, ASSEMBLY_CODE);
		if (j != 1)
			break;
	}
	if(j==1)
	{
		printf("Console Interrupt Code exceeds %d block\n",CONSOLE_INT);
		deleteConsoleINTFromDisk();
		//emptyBlock(TEMP_BLOCK);
		//writeToDisk(TEMP_BLOCK,TIMERINT);
	}
	close(fp);
	return 0;
}

/*
  This function copies the interrupts to the proper location on the disk.
*/
int loadIntCode(char* infile, int intNo)
{
	expandpath(infile);
	
	char fileName[66];
	
	labels_reset ();	
	labels_resolve (infile, fileName, (((intNo - 1) * MEM_INT1_SIZE)  + MEM_INT1 )  * PAGE_SIZE);
	
	FILE* fp = fopen(fileName, "r");
	int i,j;
	if(fp == NULL)
	{
		printf("File \'%s\' not found.\n", fileName);
		return -1;
	}
	
	for(i=0;i<INT1_SIZE;i++)
	{
		//printf ("Int block 1 - %d = %d\n",i,((intNo - 1) * INT1_SIZE)  + INT1 + i);
		j = writeFileToDisk(fp, ((intNo - 1) * INT1_SIZE)  + INT1 + i, ASSEMBLY_CODE);
		if(j != 1)
			break;
	}
	if(j==1)
	{
		printf("Interrupt Code exceeds %d block\n",INT1_SIZE);
		deleteIntCode(intNo);
		//emptyBlock(TEMP_BLOCK);
		//writeToDisk(TEMP_BLOCK,intNo + INT1 -1);
	}
	close(fp);
	return 0;
}

/*
  This function copies the timer interrupt to the proper location on the disk.
*/
int loadTimerCode(char* infile)
{
	expandpath(infile);
	
	char fileName[66];
	
	labels_reset ();	
	labels_resolve (infile, fileName, MEM_TIMERINT * PAGE_SIZE);
	
	FILE* fp = fopen(fileName, "r");
	int i,j;
	if(fp == NULL)
	{
		printf("File %s not found.\n", fileName);
		return -1;
	}
	
	for(i=0;i<TIMERINT_SIZE;i++)
	{
		j = writeFileToDisk(fp, TIMERINT + i, ASSEMBLY_CODE);
		if (j != 1)
			break;
	}
	if(j==1)
	{
		printf("Timer Interrupt Code exceeds %d block\n",TIMERINT_SIZE);
		deleteTimerFromDisk();
		//emptyBlock(TEMP_BLOCK);
		//writeToDisk(TEMP_BLOCK,TIMERINT);
	}
	close(fp);
	return 0;
}

/*
  This function copies the exception handler to the proper location on the disk.
*/
int loadExHandlerToDisk(char* infile)
{
	expandpath(infile);
	
	char fileName[66];
	
	labels_reset ();	
	labels_resolve (infile, fileName, MEM_EX_HANDLER * PAGE_SIZE);
	
	FILE* fp = fopen(fileName, "r");
	int i,j;
	if(fp == NULL)
	{
		printf("File %s not found.\n", fileName);
		return -1;
	}
	
	for(i=0;i<EX_HANDLER_SIZE;i++)
	{
		j = writeFileToDisk(fp, EX_HANDLER + i, ASSEMBLY_CODE);
		if(j != 1)
			break;
	}
	if(j==1)
	{
		printf("Exception Handler exceeds one block\n");
		deleteExHandlerFromDisk();
		//emptyBlock(TEMP_BLOCK);
		//writeToDisk(TEMP_BLOCK,EX_HANDLER);
	}
	close(fp);
	return 0;
}



/*
  This function displays the content of the files stored in the disk.
*/
void displayFileContents(char *name)
{
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0){
	  printf("Unable to Open Disk File!\n");
	  return;
	}
	
	close(fd);
	int i,j,k,l,flag=0,locationOfInode;
	int blk[512];
	
	for(i=0;i<511;i++)
		blk[i] = 0;
	
	locationOfInode = CheckRepeatedName(name);
	if(locationOfInode >= INODE_SIZE){
		printf("File \'%s\' not found!\n",name);
		return;
	}
	
	
	getDataBlocks_(blk,locationOfInode);

	k = 1;
	while (blk[k] > 0)
	{
		emptyBlock(TEMP_BLOCK);
		readFromDisk(TEMP_BLOCK,blk[k]);
		for(l=0;l<BLOCK_SIZE;l++)
		{
			if(strcmp(disk[TEMP_BLOCK].word[l],"\0")!=0)
				printf("%s   \n",disk[TEMP_BLOCK].word[l]);
		}
		//printf("next block\n");
		emptyBlock(TEMP_BLOCK);
		k++;
	}
}

/*
  This function copies the contents of the disk starting from <startBlock> to <endBlock> to a unix file.
*/
void copyBlocksToFile (int startblock,int endblock,char *filename)
{
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0){
	  printf("Unable to Open Disk File!\n");
	  return;
	}
	close(fd);
	int i,j;
	FILE *fp;
	expandpath(filename);
	fp = fopen(filename,"w");
	if(fp == NULL)
	{
		printf("File \'%s\' not found!\n", filename);
	}
	else
	{
		for(i = startblock; i <= endblock; i++)
		{
			emptyBlock(TEMP_BLOCK);
			readFromDisk(TEMP_BLOCK,i);
			for(j=0;j<BLOCK_SIZE;j++)
			{
				fprintf(fp,"%s\n",disk[TEMP_BLOCK].word[j]);
			}
		}
		fclose(fp);
	}
	
}

/*
  This function displays disk free list and the amount of free space in the disk.
*/
void displayDiskFreeList()
{
	int fd;
	fd = open(DISK_NAME, O_RDONLY, 0666);
	if(fd < 0){
	  printf("Unable to Open Disk File!\n");
	  return;
	}
	close(fd);
	int i,j,no_of_free_blocks=0;
	for(j = 0; j < NO_OF_FREE_LIST_BLOCKS; j++)
	{
		for(i = 0; i < BLOCK_SIZE; i++)
		{
			printf("%d \t - \t %s  \n",i,disk[DISK_FREE_LIST+j].word[i]);
			if(getValue(disk[DISK_FREE_LIST+j].word[i])==0)
				no_of_free_blocks++;
		}
	}
	printf("\nNo of Free Blocks = %d",no_of_free_blocks);
	printf("\nTotal no of Blocks = %d",NO_OF_DISK_BLOCKS);
}



// To expand environment variables in path
void expandpath(char *path) 		
{
	char *rem_path = strdup(path);
	char *token = strsep(&rem_path, "/");
	if(rem_path!=NULL)
		sprintf(path,"%s/%s",getenv(++token)!=NULL?getenv(token):token-1,rem_path);
	else
		sprintf(path,"%s",getenv(++token)!=NULL?getenv(token):token-1);
}

void addext(char *filename, char *ext)
{
	int l = strlen(filename);
	if(l>=16)
	{
		strcpy(filename+11,ext);
		return;
	}
	if(strcmp(filename+l-4,ext)!=0)
	{
		strcat(filename,ext);
		l = strlen(filename);

		if(l>=16)
		{
			strcpy(filename+11,ext);
			return;
		}
	}
}
