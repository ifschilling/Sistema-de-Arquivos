/* Arthur Böckmann Grossi (275607), Cassiano Translatti Furlani(278038) e Ian Fischer Schilling(275603)*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"
/* talvez precise mais includes */

/* STRUCTURES*/
typedef struct files
{	
	char *fileName;
	int currentPointer;
	int handle;
	
	
}fileHandler;

/* GLOBAL VARIABLES*/
int systemReady = 0;
BYTE buffer[SECTOR_SIZE];
struct t2fs_superbloco superBlock;

WORD sectorsByBlock;
DWORD numberOfTotalBLocks;
WORD numBlocksINode;

WORD freeInodeBitmap;
WORD freeBlocksBitmap; 

/* CREATED FUNCTIONS */
int setup();




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
	/* Alocar memoria para armazenar o registro do novo arquivo */
	struct t2fs_record *registro;
	registro = calloc(1, sizeof(t2fs_record));

	/* Inicializar registro */
	registro->TypeVal = TYPEVAL_REGULAR;
	registro->name = filename;
	registro->inodeNumber = ;


}

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
	if( (read_sector(0, &buffer)) ){
		printf("Could not read first sector\n");
		return -1;
	}
	else{
		char idT2FS[4];
		char byte, word[2], dword[4];

		strncpy(idT2FS, buffer, 4); //ID
		if( strcmp(idT2FS, "T2FS") ){
			printf("Error, not T2FS\n");
			return -1;
		}
		else{
			strcpy(word, buffer[8]);
			strcat(word, buffer[9]);

			freeBlocksBitmap = (WORD)atoi(word);

			strcpy(word, buffer[10]);
			strcat(word, buffer[11]);
			
			freeInodeBitmap = (WORD)atoi(word);

			strcpy(word, buffer[12]);
			strcat(word, buffer[13]);
			
			numBlocksINode = (WORD)atoi(word);

			strcpy(word, buffer[14]);
			strcat(word, buffer[15]);
			
			sectorsByBlock= (WORD)atoi(word);

			strcpy(word, buffer[16]);
			strcat(word, buffer[17]);
			strcat(word, buffer[18]);
			strcat(word, buffer[19]);
			
			numberOfTotalBLocks = atoi(word);


			int firstDataSector = (numBlocksINode + freeInodeBitmap + freeBlocksBitmap + 1) * sectorsByBlock;
			int firstDataBlock = (numBlocksINode + freeInodeBitmap + freeBlocksBitmap + 1);
			int inodeSector = (1 + freeBlocksBitmap + freeInodeBitmap) * sectorsByBlock;

			//Prepare root directory registers
			struct t2fs_record rootDir1, rootDir2;
			rootDir1.TypeVal = TYPEVAL_DIRETORIO;
			rootDir1.name = "root";
			rootDir1.inodeNumber = 0;

			rootDir2.TypeVal = TYPEVAL_DIRETORIO;
			rootDir2.name = "root";
			rootDir2.inodeNumber = 0;

			struct t2fs_inode rootNode; 
			rootNode.blocksFileSize = 1;
			rootNode.bytesFileSize = 2*sizeof(struct t2fs_record);
			rootNode.dataPtr[0] = firstDataBlock;
			rootNOde.dataPtr[1] = INVALID_PTR;	
			rootNode.singleIndPtr = INVALID_PTR;  
			rootNode.doubleIndPtr = INVALID_PTR;
			rootNode.reservado[2] = {0,0};

			buffer[0] = rootDir1.TypeVal;
			buffer[1] = '\0';
			strcat(buffer, rootDir1.name);
			int temp;
			sprintf(temp, "%i",rootDir1.inodeNumber);
			strcat(buffer, temp);

			int size = strlen(buffer);
			buffer[size++] = rootDir2.TypeVal;
			buffer[size] = '\0';
			strcat(buffer, rootDir2.name)
			sprintf(temp, "%i", rootDir2.inodeNumber);
			strcat(buffer, temp);

			if( (write_sector(firstDataSector, buffer)) )//Write root directory registers
				return -1;

			//Convert inode data to a buffer format(string)
			sprintf(temp, "%i", rootNode.blocksFileSize);
			strcpy(buffer, temp);
			sprintf(temp, "%i", rootNode.bytesFileSize);
			strcat(buffer, temp);
			sprintf(temp, "%i", rootNode.dataPtr[0]);
			strcat(buffer, temp);
			sprintf(temp, "%i", rootNode.dataPtr[1]);
			strcat(buffer, temp);
			sprintf(temp, "%i", rootNode.singleIndPtr);
			strcat(buffer, temp);
			sprintf(temp, "%i", rootNode.doubleIndPtr);
			strcat(buffer, temp);
			sprintf(temp, "%i", rootNode.reservado[0]);
			strcat(buffer, temp);
			sprintf(temp, "%i", rootNode.reservado[1]);
			strcat(buffer, temp);

			if( (write_sector(inodeSector, buffer)) )//Write in the inode area
				return -1;

			if( !(setBitMap2(BITMAP_INODE, 0, 1)) )//inode 0 occupied
				return -1;

			if( !(setBitMap2(BITMAP_DADOS, firstDataBlock, 1)) )//block 0 occupied
				return -1;


		}

	}
}
