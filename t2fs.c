#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"


#define MAX_NAME_SIZE 256
/* STRUCTURES*/
typedef struct files
{	
	char fullPathName[MAX_NAME_SIZE];
	BYTE type;
	int currentPointer;
	int handle;
	int size;

}fileHandler;

typedef struct cD{
	char pathName[MAX_NAME_SIZE];
	int block;
}dirDescription;

/* GLOBAL VARIABLES*/

struct t2fs_superbloco *superBlock;
int systemReady = 0;
BYTE buffer[SECTOR_SIZE];
dirDescription *currentDir;

int bytesInBlock;
int firstDataBlock;
int inodeSector;

fileHandler *openedFiles[10];

/* CREATED FUNCTIONS */
int setup();


/* 
Funtion: initialises the file system by reading the superblock
Return 	0 if successful
		-1 if error	
*/

int setup(){
	//Create root directory and prepare the file
	if( (read_sector(0, buffer)) ){
		printf("Could not read first sector\n");
		return -1;
	}

	superBlock = calloc(1, sizeof(struct t2fs_superbloco));

	strncpy(superBlock->id,(char*) buffer, 4); //ID
	if( strcmp(superBlock->id, "T2FS") ){
		printf("Error, not T2FS\n");
		return -1;
	}
	char temp[4];

	strncpy(temp, buffer+4, 2);	//version
	superBlock->version = (WORD)atoi(temp);

	strncpy(temp, buffer+6, 2);	
	superBlock->superblockSize = (WORD)atoi(temp);

	strncpy(temp, buffer+8, 2);	
	superBlock->freeBlocksBitmapSize =(WORD)atoi(temp);

	strncpy(temp, buffer+10, 2);	
	superBlock->freeInodeBitmapSize =(WORD)atoi(temp);

	strncpy(temp, buffer+12, 2);	
	superBlock->inodeAreaSize =(WORD)atoi(temp);

	strncpy(temp, buffer+14, 2);	
	superBlock->blockSize =(WORD)atoi(temp);

	strncpy(temp, buffer+12, 2);	
	superBlock->diskSize =(DWORD)atoi(temp);

	bytesInBlock = superBlock->blockSize * SECTOR_SIZE;
	firstDataBlock = (1 + superBlock->freeInodeBitmapSize + superBlock->freeBlocksBitmapSize + superBlock->inodeAreaSize);
	inodeSector = (1 + superBlock->freeBlocksBitmapSize + superBlock->freeInodeBitmapSize)*SECTOR_SIZE;

	///Initialise the root directory
	currentDir = calloc(1, sizeof(dirDescription));
	strcpy(currentDir->pathName, "/");
	currentDir->block = firstDataBlock;

	//First two records must be current directory and father directory
	struct t2fs_record rootRecord0, rootRecord1;
	rootRecord->TypeVal = TYPEVAL_DIRETORIO;
	rootRecord->name = "root";
	rootRecord->inodeNumber = 0;

	rootRecord1->TypeVal = TYPEVAL_DIRETORIO;
	rootRecord1->name = "root";
	rootRecord1->inodeNumber = 0;

	//Inode initialization
	struct t2fs_inode rootInode0;
	rootInode->blocksFileSize = 1;
	rootInode->bytesFileSize = 2*sizeof(struct t2fs_record);
	rootInode->dataPtr[0] = 0;
	rootInode->dataPtr[1] = INVALID_PTR;
	rootInode->singleIndPtr = INVALID_PTR;
	rootInode->doubleIndPtr = INVALID_PTR;

	//Now we set the bitmap of blocks and Inode

	if( setBitMap2(BITMAP_INODE, 0, 1))
		return -1; // FAILED

	if( setBitMap2(BITMAP_DADOS, 0, 1))
		return -1;

	//We must write in the "disk"
	memcpy(buffer, rootRecord0, sizeof(stuct t2fs_record));
	memcpy(buffer + sizeof(struct t2fs_record), rootRecord1, sizeof(stuct t2fs_record));
	write_sector( (firstDataBlock*SECTOR_SIZE), buffer);

	memcpy(buffer, rootInode0, sizeof(struct t2fs_inode));
	write_sector(inodeSector, buffer);
	systemReady = 1;
	return 0;
}

