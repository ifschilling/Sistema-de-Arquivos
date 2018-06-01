/* Arthur Böckmann Grossi (275607), Cassiano Translatti Furlani(278038) e Ian Fischer Schilling(275603)*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"
/* talvez precise mais includes */

#define INODE 0
#define DATA 1

/* STRUCTURES*/
typedef struct files{	
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
struct *t2fs_inode findInode(DWORD inumber)




int identify2(char *name, int size){
	char student[MAX_STU_CHAR] = "";
	int i;
	int st2 = strlen(STU1);
	int st3 = st2 + strlen(STU2);
	int letters;
	strcat(student, STU1);
	strcat(student, STU2);
	strcat(student, STU3);
	if(size >= MAX_STU_CHAR) //Size is equal to or greater than students names
	{
		for( i = 0; i < MAX_STU_CHAR;i++)
		name[i] = student[i];
		return 0;
	}
	else if( size >= 9) //At least 1 characters for each
	{
		letters = (int)size/3 - 2;
		for(i = 0; i < letters; i++)
		{
			name[i] = student[i];
			name[i + letters + 1] = student[i + st2];
			name[i + 2*(letters + 1)] = student[i + st3];
		}
		name[letters] = '\n';
		name[2*(letters) +1]= '\n';
		name[i + 2*(letters + 1) + 1] = '\0';
	}
	return -1;
}

FILE2 create2 (char *filename){
struct t2fs_record regaux;
struct t2fs_inode inodeaux;


	if(!systemReady)
		setup();


	read_sector((currentDir->block) * (superBlock->blockSize), buffer);
	memcpy(&regaux, buffer, sizeof(t2fs_record)); //primeiro registro do diretorio corrente

	&inodeaux = findinode(regaux.inodeNumber);

	memcpy(&regaux, buffer+(2*sizeof(t2fs_record)), sizeof(t2fs_record));

	
	int k;
	int i;
	DWORD rsector;

	for(k=0; k<=indoeaux.blocksFileSize; k++){	//percorre blocos
		if(k=0)
			rsector = inode.dataPtr[0] * superBlock->blockSize;
		else if(k=1)
			rsector = inode.dataPtr[1] * superBlock->blockSize;
		else if(k<=){
			
		}else{

		}

	
		for(int i=0; i<=superblock->blockSize; i++){	//setores dentro do bloco
			read_sector(rsector + i, buffer);


		}
	}
	
	
	
	




	int n=0;
	//varrer registros do diretorio pai, para checar se ha algum com o mesmo nome do arquivo que sera criado
	for(int i=0, i<=((inode.blocksFileSize*SECTOR_SIZE)/superBlock.blocksize), i++){
		n=0;
		do{
			if(strcmp(aux.name, filename)==0){
				prinft("ERRO, filename already being used!");
				return -1;
			}
			
			memcpy(&aux, (buffer+(n*sizeof(t2fs_record))), sizeof(t2fs_record));
			n++;
		}while(aux.name!=NULL || n<(SECTOR_SIZE/sizeof(t2fs_record)));
	}

}


(BLOCOS DO ARQUIVO * TAMANHO DO SETOR) / SETORES P BLOCO

int delete2 (char *filename){

}

FILE2 open2 (char *filename){

}

int close2 (FILE2 handle){

}

int read2 (FILE2 handle, char *buffer, int size){

}

int write2 (FILE2 handle, char *buffer, int size){

}

int truncate2 (FILE2 handle){

}

int seek2 (FILE2 handle, DWORD offset){

}

int mkdir2 (char *pathname){

}

int rmdir2 (char *pathname){

}

int chdir2 (char *pathname){

}

int getcwd2 (char *pathname, int size){

}

DIR2 opendir2 (char *pathname){

}

int readdir2 (DIR2 handle, DIRENT2 *dentry){

}

int closedir2 (DIR2 handle){

}

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
	inodeSector = (1 + superBlock->freeBlocksBitmapSize + superBlock->freeInodeBitmapSize)*blockSize;

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
	struct t2fs_inode rootInode;
	rootInode->blocksFileSize = 1;
	rootInode->bytesFileSize = 2*sizeof(rootRecord0);
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
	memcpy(buffer, rootRecord0, sizeof(rootRecord0));
	memcpy(buffer + sizeof(rootRecord0), rootRecord1, sizeof(rootRecord0));
	write_sector( (firstDataBlock*SECTOR_SIZE), buffer);

	memcpy(buffer, rootInode, sizeof(rootInode));
	write_sector(inodeSector, buffer);
	systemReady = 1;
	int i;
	
	openedFiles = calloc(10, sizeof(fileHandler));
	
	return 0;
}


struct *t2fs_inode findInode(DWORD inumber){
	const long int inodesporsetor = SECTOR_SIZE/sizeof(struct t2fs_inode);	//numero de inodes em 256bytes
	int offset = (inumber - (inumber/inodesporsetor)*inodesporsetor);
	struct t2fs_inode inode;
	

	read_sector(inodeSector+(inumber/inodesporsetor), buffer);
	memcpy(&inode, buffer+offset, sizeof(struct t2fs_inode));

	return &inode;

}
