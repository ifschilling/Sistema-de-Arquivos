/* Arthur Böckmann Grossi (275607), Cassiano Translatti Furlani(278038) e Ian Fischer Schilling(275603) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"
#include "../include/functions.h"

#define MAX_STU_CHAR 100
#define STU1 "Arthur Bockmann Grossi - 275607"
#define STU2 "Cassiano Translatti Furlani - 278038"
#define STU3 "Ian Fischer Schilling - 275603"

#define MAX_PATH_NAME_SIZE 301	//CECHIN falou em aula que o maior caminho sera 4 diretorios aninhados, para garantir calculamos para 5 diretorios.

struct read_dir{
	DWORD dirBlocks;
	DWORD rsector;
	DWORD offsetDirBlock;
	DWORD offsetSingleIndBlock;
	DWORD offsetDoubleIndBlock;
	int t, g, h, j, z;
	DWORD bytesRead;
	BYTE sindbuffer[SECTOR_SIZE];
	BYTE dindbuffer[SECTOR_SIZE];
	BYTE buffer[SECTOR_SIZE];
	DWORD pointer, indpointer;
	DIR2 actual_dir_handle;
}

struct read_dir info;

int identify2 (char *name, int size){
	char student[MAX_STU_CHAR] = "";
	int i;
	int st2 = strlen(STU1);
	int st3 = st2 + strlen(STU2);
	int letters;
	strcat(student, STU1);
	strcat(student, STU2);
	strcat(student, STU3);
	if(size >= MAX_STU_CHAR){ //Size is equal to or greater than students names
		for( i = 0; i < MAX_STU_CHAR; i++)
			name[i] = student[i];
		return 0;
	}
	else if( size >= 9){ //At least 1 characters for each
		letters = (int)size/3 - 2;
		for(i = 0; i < letters; i++){
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
	fileHandler file;
	struct coordinates info;
	int d=0;
	char abspath[MAX_PATH_NAME_SIZE];


	if(!systemReady)
		setup();
	info = getReg(filename);
	if(info.record.TypeVal == TYPEVAL_INVALIDO){ //nome valido, nao utilizado ainda
		do{
			if(openedFiles[d].type == TYPEVAL_INVALIDO){
				openedFiles[d].type == TYPEVAL_REGULAR;
				openedFiles[d].currentPointer = 0;
				if(absolutePath(abspath, filename)){
					printf("ERROR, could not convert filename to absolutePath! (function: create2) \n");
					return -1;
				}
				strcpy(openedFiles[d].fullPathName, abspath);
				struct t2fs_inode newFileInode;
				
				newFileInode.blocksFileSize = 0;
				newFileInode.bytesFileSize = 0;
				newFileInode.dataPtr[0] = INVALID_PTR;
				newFileInode.dataPtr[1] = INVALID_PTR;
				newFileInode.singleIndPtr = INVALID_PTR;
				newFileInode.doubleIndPtr = INVALID_PTR;
				
				openedFiles[d].inode = newFileInode;
				openedFiles[d].numInode = searchBitmap2(BITMAP_INODE, 0);
				setInode(newFileInode, openedFiles[d].numInode);
				
				struct t2fs_record newReg;
				char *token;
				char lastoken[59];
				char *auxpath = malloc(strlen(filename)+1);
				
				//nome é só o ultimo argumento do filename
				newReg.TypeVal = TYPEVAL_REGULAR;
				token = strtok(auxpath, "/");
				strcpy(lastoken, token);
				while(token!=NULL){
					token = strtok(NULL, "/");
					strcpy(lastoken, token);
				}
				
				strcpy(newReg.name, lastoken);
				newReg.inodeNumber = openedFIles[d].numInode;
				
				if(setReg(newReg, filename)){
					printf("ERROR, could not create new record inside directory (function: create2) \n");
					return -1;
				}	
				return d;	//handle do arquivo == posicao dele no vetor de arquivos abertos
			}
			d++;
		}while(d<MAX_NUM_FILES);
		
		printf("ERROR, there was already %i opened files! (function: create2)\n", MAX_NUM_FILES);
		return -1;
		
	}else{
		printf("ERROR, there are other file(s) with the same name in this directory! (function: create2)\n");
		return -1;
	}
}

int delete2 (char *filename){
  struct coordinates fileCoord = getReg(filename);
  struct t2fs_record fileReg = fileCoord.record;
  DWORD fileInode = fileReg.inodeNumber;
  
  if( isFileOpen(fileInode)){
    printf("Tried to delete an open file\n");
    return -1;
  }
  
  if(fileReg.TypeVal == TYPEVAL_INVALIDO){
    printf("Tried to delete an invalid file\n");
    return -1;
  }
  
  if(fileReg.TypeVal == TYPEVAL_DIRETORIO){
    printf("Tried to delete a directory\n");
    return -1;
  }
  /*It is a real file, now we can delete it*/
  
  if(delBlocks(fileInode, -1)){
    printf("Error deleting blocks in delete2\n");
    return -1;
  }
  /*Now we free the inode*/
  if( setBitmap2(BITMAP_INODE, fileInode, 0) ){
    printf("Error setting inode bitmap in delete2\n");
    return -1;
  }
  
  fileReg.TypeVal == TYPEVAL_INVALIDO;
  strcpy(fileReg.name, '\0');
  fileReg.inodeNumber = INVALID_PTR;
  /*We now must change the file record*/
  if(setReg(fileReg, filename)){
    printf("Error setting file record in delete2\n");
    return -1;
  }
  
  

}

FILE2 open2 (char *filename){
	struct coordinates reg;
	struct t2fs_inode inode;
	char *abspath;
	
	
  reg = getReg(filename);
	
	if(reg.record.TypeVal == TYPEVAL_INVALIDO){
			printf("ERROR: file not found!\n");
			return -1;
	}

	int i=0;
	//searches for the next free openedFiles
	while((i<MAX_NUM_FILES) && (openedFiles[i].type != TYPEVAL_INVALIDO))
		i++;

	if(i >= MAX_NUM_FILES){
		printf("ERROR: openedFiles is full!\n");
		return -1;
	}
	
	if(absolutePath(abspath, filename)){
		printf("ERROR, could not convert filename to absolutePath! (function: open2) \n");
		return -1;
	}
	strcpy(openedFiles[i].fullPathName, abspath);	
	openedFiles[i].type = TYPEVAL_REGULAR;
	openedFiles[i].currentPointer = 0;
	inode = getInode(reg.record.inodeNumber);
	openedFiles[i].inode = inode;
	openedFiles[i].numInode = reg.record.inodeNumber;
	
	return i;
}

int close2 (FILE2 handle){
	if(handle < 0 || handle >MAX_NUM_FILES){
			printf("ERROR: handle not valid!\n");
			return -1;
	}
	
	fileHandler *file;
	file = &(openedFiles[handle]);

	if (file->type != TYPEVAL_REGULAR){
			printf("ERROR: handle file not found!\n");
			return -1;
	}

	file->fullPathName[0] = '\0';
	file->type = TYPEVAL_INVALIDO;
	return 0;
}

int read2 (FILE2 handle, char *buffer, int size){
  fileHandler readFile;
	if(handle <0 || handle >= MAX_NUM_FILES){
		printf("Error at read2 invalid handle\n");
		return -1;
	}
	readFile = openedFiles[handle];
  
  if(readFile.type == TYPEVAL_INVALIDO){
    printf("Tried to read an invalid file\n");
    return -1;
  }
  
  if(readFile.type == TYPEVAL_DIRETORIO){
    printf("Tried to read a directory in read2\n");
    return -1;
  }
  
	t2fs_inode fileINode = readFile.inode;
	



	DWORD cPointerBlock = currentPointer/bytesInBlock;/*Block in the file, not the real block*/
  DWORD cPointerPositionInBlock = currentPointer%(bytesInBlock*cPointerBlock);/*Byte in the block*/
  DWORD cSector;
  DWORD cPSector = cPointerPositionInBlock/SECTOR_SIZE;/*Sector in block*/
  DWORD cPInSector = cPointerPositionInBlock%SECTOR_SIZE;/*Position in the sector*/
  DWORD bytesRemainingInBlock = bytesInBlock - cPointerPositionInBlock;
  DWORD bytesRemainingInSector = SECTOR_SIZE - cPInSector;

  char cpBuff[SECTOR_SIZE];

  int blocksToRead = (size - bytesRemainingInBlock)/bytesInBlock;
	if(bytesRemainingInBlock != 0)
    blocksToRead++;
  
    int bytesToRead;
    if ( size + currentPointer > (readFile.inode.bytesFileSize+1)){
      bytesToRead = (readFile.inode.bytesFileSize - currentPointer)+1;
    }
    else
      bytesToRead = size;
 


  
  /*----------------*/

  DWORD blocksRemaining = fileInode.blocksFileSize;
  DWORD iBSector = SECTOR_SIZE/sizeof(DWORD);/*pointers by sector*/

	DWORD dBlocksToDel;
	BYTE delBuff;
	DWORD indexSectorsToRead;
	DWORD sFirstBlockSector;
	DWORD bIndex, bDataFinalPosition;
	dBlocksToDel = blocksRemaining;
	DWORD arrayOfDataBlocks[dBlocksToDel];
	int i,j, k, l = 0;



	if(blocksRemaining > (2 + PointersInBlock)){ /* double Indirection */
  	
		DWORD dfirstBlockSector = blockFirstSector(fileInode.doubleIndPtr);
		dBlocksToDel = blocksRemaining - ( 2 + PointersInBlock);

		bIndex = (dBlocksToDel / PointersInBlock) + 1;/*double indirection block of index*/
		bDataFinalPosition = dBlocksToDel % PointersInBlock;/*Last valid block position*/

    /*We have to read just the correct amount of block index*/
		indexSectorsToRead = bIndex /iBSector;//
		if( (bIndex % iBSector) != 0 )
			indexSectorsToRead++;
   
		/*array of blocks of index and their dataBlocks*/
		DWORD arrayOfBlockIndex[bIndex];
		
		k = 0;
    for(i = 0; i < indexSectorsToRead; i++){/*block of index in each sector*//
    	if( read_sector(dfirstBlockSector + i, delBuff) ){
	    	printf("Error in delBlocks,failed to read sector %i\n", dfirstBlockSector+i);
	    	return -1;
	    }
	    for(j = 0; j<iBSector;j++){
	    	if(k < bIndex){
	    		memcpy(&arrayOfBlockIndex[k], *(delBuff+(sizeof(DWORD)*j)), sizeof(DWORD));/*This is the block of indexes*/
	    		k++;
	    	}
	    }
	  }
	  blocksRemaining -= dBlocksToDel;
		/* We have all the blocks of index */
		for( i = 0; i < bIndex, i++){/*Each index block*/
	  	sFirstBlockSector = blockFirstSector(arrayOfBlockIndex[i]);/*First Block of the single index block*/
	  	for(j = 0; j < superBlock.blockSize; j++){
	  		if( read_sector(sFirstBlockSector + j, &delBuff) ){
	    		printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
	    		return -1;
	    	}
	    	for(k = 0; k <iBSector; k++){
	    		if(l < dBlocksToDel){
	    			memcpy(&arrayOfDataBlocks[blocksRemaining+l], *(delBuff + (k*sizeof(DWORD))), sizeof(DWORD));
	    			l++;
	    		}
	    	}
	  	}
	  
	  }

  }
  if(blocksRemaining > 2 ){
	
		dBlocksToDel = blocksRemaining- 2;
		sFirstBlockSector = blockFirstSector(fileInode.singleIndPtr);/*First Block of the single index block*/

	  indexSectorsToRead = dBlocksToDel /iBSector;//
	  if( (bIndex % iBSector) != 0 )
	  	indexSectorsToRead++;
	  blocksRemaining -=dBlocksToDel;
	  l = 0;
		for(j = 0; j < indexSectorsToRead; j++){
			if( read_sector(sFirstBlockSector + j, delBuff) ){
	  		printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
	  		return -1;
	  	}
	  	for(k = 0; k <iBSector; k++){
	  		if(l < dBlocksToDel){
	  			memcpy(&arrayOfDataBlocks[blocksRemaining+l], *(delBuff + (k*sizeof(DWORD))), sizeof(DWORD));
	  			l++;
	  		}
	  	}
		}
	}

	if(blocksRemaining <= 2 ){
		l = 0;
		dBlocksToDel = blocksRemaining;
		for(i = 0; i<2; i++){
			if(l < dBlocksToDel){
	  			memcpy(&arrayOfDataBlocks[l], fileInode.dataPtr[l], sizeof(DWORD));
	  			l++;
	  		}
		}
  }
  /*----------------*/
		/*Now we have all the block numbers in arrayOfDataBlocks*/
  
    /*First we read the currentPointer block only*/
  int offset = bytesRemainingInSector;
  DWORD bytes_Read;
    cSector = blockToSector(arrayOfDataBlocks[cPointerBlock]);
    for(i = cPSector; i < superBlock.blockSize; i++){
      if( read_sector(cSector + i, cpBuff) ){
        printf("ERROR reading sector read2\n");
        return -1;
      }
      for(j = 0; j < SECTOR_SIZE; j++){
        if(bytesToRead > 0){
          if(i == cPSector){
            if( bytesRemainingInSector > 0 ){
              memcpy(buffer + j,cpBuff + (cPInSector+j), sizeof(BYTE));
              bytesRemainingInSector--;
              bytesToRead--;
              bytes_Read++;
            }
          }
          else{/*Not the currentPointer sector*/
            if( bytesToRead> 0 ){
              memcpy(buffer + j +offset+ (cPSector-1-i)*(SECTOR_SIZE), cpBuff + j, sizeof(BYTE));
              bytesToRead--;
               bytes_Read++;
            }
          }
        }
      } 
    }
  /*Now we read the remaining bytes in the other blocks*/
  for(i = 1; i < blocksToRead-1; i++){
    cSector = blockToSector(arrayOfDataBlocks[cPointerBlock+i]);
    for(k = 0; k < superBlock.blockSize; k++){/*Reads all sectors in the block*/
      if( read_sector(cSector + k, cpBuff) ){
        printf("ERROR reading sector read2\n");
        return -1;
       }
      for(j = 0; j < SECTOR_SIZE; j++){
       if(bytesToRead > 0){
         memcpy(buffer + size - bytesToRead, cpBuff + j, sizeof(BYTE));       
         bytesToRead--;
         bytes_Read++;
       }
      }
    }
  }
  
  readFile.currentPointer +=  bytes_Read++;
  openedFiles[handle] = readFile;
  return size-bytesToRead;
  
}

int write2 (FILE2 handle, char *buffer, int size){
	if(handle < 0 || handle >= MAX_NUM_FILES){
		printf("ERROR: invalid handle at write2\n");
		return -1;
	}
	
	fileHandler file;
	file = openedFiles[handle];
	
	if(file.type != TYPEVAL_REGULAR){
		printf("ERROR: openedFile not valid at write2\n");
		return -1;
	}
	if(file.currentPointer < 0){
		printf("ERROR: currentPointer is less than 0 at write2\n");
		return -1;
	}
	if(file.numInode < 0){
		printf("ERROR: numInode is less than 0 at write2\n");
		return -1;
	}
	
	char buf[SECTOR_SIZE];
	int DWORDinSector = SECTOR_SIZE / sizeof(DWORD);
	int DWORDinBlock = bytesInBlock / sizeof(DWORD);
	int inodeInd = file.currentPointer / bytesInBlock;
	DWORD sing_sector, sing_offset, doub_sector, doub_offset;
	DWORD writeBlock;
	
	if(inodeInd < 2){
		// sem indirecao
		writeBlock = file.inode.dataPtr[inodeInd];
	}
	else if(inodeInd < (2 + DWORDinBlock)){
		// indirecao simples
		sing_sector = (inodeInd-2) / DWORDinSector + blockFirstSector(file.inode.singleIndPtr);
		
		if(read_sector(sing_sector, buf)){
			printf("ERROR: could not read sector at write2\n");
			return -1;
		}
		
		sing_offset = (inodeInd-2) % DWORDinSector; // DWORD number inside the sector
		memcpy(&writeBlock, (buf + (sing_offset * sizeof(DWORD))), sizeof(DWORD));
	}
	else{
		// indirecao dupla
		doub_sector = (inodeInd - (2 + DWORDinBlock)) / ((DWORDinSector * DWORDinBlock) + blockFirstSector(file.inode.doubleIndPtr);
		if(read_sector(doub_sector, buf)){
			printf("ERROR: could not read sector at write2\n");
			return -1;
		}
		
		doub_offset = (inodeInd - (2 + DWORDinBlock)) % ((DWORDinSector * DWORDinBlock);
		memcpy(&sing_sector, (buf + (doub_offset * sizeof(DWORD))), sizeof(DWORD));
		
		if(read_sector(sing_sector, buf)){
			printf("ERROR: could not read sector at write2\n");
			return -1;
		}
		
		sing_offset = (inodeInd - (2 + DWORDinBlock)) % DWORDinSector;
		memcpy(&writeBlock, (buf + (sing_offset * sizeof(DWORD))), sizeof(DWORD));
	}

	// usar o writeBlock para escrever no arquivo até ele lotar, ir modificando o currentPointer e diminuindo size

	// quando o writeBlock lotar, alocar mais um bloco usando o bitmap

	// salvar o file
}

int truncate2 (FILE2 handle){
  fileHandler truncFile;
   
  if(handle < 0 || handle >MAX_NUM_FILES){
    printf("Error invalid handle in truncate2\n");
    return -1;
  }
	truncFile = openedFiles[handle];  
  
  if(fileReg.TypeVal == TYPEVAL_INVALIDO){
    printf("Tried to truncate an invalid file\n");
    return -1;
  }
  
  if(fileReg.TypeVal == TYPEVAL_DIRETORIO){
    printf("Tried to truncate a directory in trunc2\n");
    return -1;
  }
  /*It is a real file, now we can truncate it*/
  
  if( delBlocks(truncFile.numInode, truncFile.currentPointer) ){
    printf("Error truncating blocks in trunc2\n");
    return -1;
  }
  
  truncFile.inode.blocksFileSize = (truncFile.currentPointer)/bytesInBlock;/*Remaining blocks*/
  if( (truncFile.currentPointer)%bytesInBlock != 0 )
    truncFile.inode.blocksFileSize++;
  truncFile.inode.bytesFileSize = truncFile.currentPointer;/*New bytes file size*/
  
  if( setInode(truncFile.inode, truncFile.numInode) ){
    printf("Error setting inode in truncate2\n");
    return -1;
  }
}

int seek2 (FILE2 handle, DWORD offset){
	if(handle < 0 || handle >MAX_NUM_FILES){
			printf("ERROR: handle not valid!\n");
			return -1;
	}
	
	if(offset > ((superblock.blockSize*SECTOR_SIZE)*2)+PointersInBlock*((superblock.blockSize*SECTOR_SIZE)+(PointersInBlock*(superblock.blockSize*SECTOR_SIZE)))){	//tamanho maximo de um arquivo no T2FS
	printf("ERROR, offset is bigger than bytes supported in any file or directory! (function: seek2) \n");
	return -1;
	}

	if(offset == -1){
		openedFiles[handle].currentPointer = openedFiles[handle].inode.bytesFileSize + 1;
	}else{
		openedFiles[handle].currentPointer = offset;
	}
	return 0;
}

int mkdir2 (char *pathname){
	fileHandler file;
	struct coordinates info;
	int d=0;
	char abspath[MAX_PATH_NAME_SIZE];


	if(!systemReady)
		setup();
	info = getReg(pathname);
	if(info.record.TypeVal == TYPEVAL_INVALIDO){ //nome valido, nao utilizado ainda
		struct t2fs_inode newFileInode;
		struct t2fs_record dir0;
		struct coordinates dir1;
		int inodeNum;
		struct t2fs_record newReg;
		char *token;
		char lastoken[59];
		char *auxpath = malloc(strlen(filename)+1);

		//nome é só o ultimo argumento do pathname
		newReg.TypeVal = TYPEVAL_DIRETORIO;
		strcpy(auxpath, pathname);
		token = strtok(auxpath, "/");
		strcpy(lastoken, token);
		while(token!=NULL){
			token = strtok(NULL, "/");
			strcpy(lastoken, token);
		}
	
		strcpy(newReg.name, lastoken);
		inodeNum = searchBitmap2(BITMAP_INODE, 0);
		newReg.inodeNumber = inodeNum;
		
		if(setReg(newReg, pathname)){
			printf("ERROR, could not create new record inside directory (function: create2) \n");
			return -1;
		}
		
		dir0.TypeVal = TYPEVAL_DIRETORIO;
		strcpy(dir0.name, lastoken);
		dir0.inodeNumber = inodeNum;
		
		strncpy(auxpath, pathname, strlen(pathname)-strlen(lastoken)-1);
		dir1 = getReg(auxpath);
		setReg(dir1.record, pathname);
		
		newFileInode.blocksFileSize = 1;
		newFileInode.bytesFileSize = 2*sizeof(struct t2fs_record);
		newFileInode.dataPtr[0] = searchBitmap2(BITMAP_DADOS, 0);
		setBitmap2(newFileInode.dataPtr[0], BITMAP_DADOS, 1);

		newFileInode.dataPtr[1] = INVALID_PTR;
		newFileInode.singleIndPtr = INVALID_PTR;
		newFileInode.doubleIndPtr = INVALID_PTR;
		setInode(newFileInode, inodeNum);
		return 0;
	}else{
		printf("ERROR, there are other directory(s) with the same name in this directory! (function: create2)\n");
		return -1;
	}
}

int rmdir2 (char *pathname){
	/* TO-DO */
}

int getcwd2 (char *pathname, int size){
	
	if(size < strlen(currentDir.pathName)+1){
		printf("ERROR, insufficient size! (function: getcwd2) \n");
		return -1;
	}else{
		strncpy(pathname, currentDir.pathName, size*sizeof(char));
		return 0;
	}
}

DIR2 opendir2 (char *pathname){
	struct coordinates reg;
	struct t2fs_inode inode;
	char *abspath;
	
	
  reg = getReg(pathname);
	
	if(reg.record.TypeVal == TYPEVAL_INVALIDO){
			printf("ERROR: file not found!\n");
			return -1;
	}

	int i=0;
	//searches for the next free openedFiles
	while((i<MAX_NUM_FILES) && (openedFiles[i].type == TYPEVAL_INVALIDO))
		i++;

	if(i >= MAX_NUM_FILES){
		printf("ERROR: openedFiles is full!\n");
		return -1;
	}
	
	if(absolutePath(abspath, pathname)){
		printf("ERROR, could not convert pathname to absolutePath! (function: open2) \n");
		return -1;
	}
	strcpy(openedFiles[i].fullPathName, abspath);	
	openedFiles[i].type = TYPEVAL_DIRETORIO;
	openedFiles[i].currentPointer = 0;
	inode = getInode(reg.record.inodeNumber);
	openedFiles[i].inode = inode;
	openedFiles[i].numInode = reg.record.inodeNumber;
	
	return i;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){
	if(handle < 0 || handle >MAX_NUM_FILES){
			printf("ERROR: handle not valid! (function: readdir2) \n");
			dentry.fileType = TYPEVAL_INVALIDO;
			return -1;
	}
	if(openedFiles[handle].type != TYPEVAL_DIRETORIO){
		printf("ERROR, this handle does not reference a directory! (function: readdir2) \n");
		dentry.fileType = TYPEVAL_INVALIDO;
		return -1;
	}
	
	struct t2fs_record regs;
	struct t2fs_inode inodes;
	
	if(handle != info.actual_dir_handle){	//mudou o diretorio que sera lido, deve reiniciar todas variaveis
		info.dirBlocks=0;
		info.rsector=0;
		info.offsetDirBlock=0;
		info.offsetSingleIndBlock=0;
		info.offsetDoubleIndBlock=0;
		info.t=0; info.g=0; info.h=0; info.j=0; info.z=0;
		info.bytesRead=0;
		info.pointer=0;
		info.indpointer=0;
		info.actual_dir_handle = handle;
	}
	if(info.bytesRead >= (openedFiles[handle].inode.bytesFileSize)){
		printf("All valid records were read!\n");
		return -1;
	}
	
	regs = readRegs(openedFiles[handle].inode);
	
	dentry.fileType = regs.TypeVal;
	
	inodes = getInode(regs.inodeNumber);
	if(inodes.blocksFileSize == -1){
		printf("ERROR, could not get inode! (function: readdir) \n");
		return -1;
	}
	dentry.fileSize = inodes.bytesFileSize;
	//copia o nome do registro lido para dentry
	
	strcpy(dentry.name, regs.name);
	return 0;
}

int closedir2 (DIR2 handle){
	if(handle < 0 || handle >MAX_NUM_FILES){
			printf("ERROR: handle not valid!\n");
			return -1;
	}
	
	fileHandler *file;
	file = &(openedFiles[handle]);

	if (file->type != TYPEVAL_DIRETORIO){
			printf("ERROR: handle file not found!\n");
			return -1;
	}

	file->fullPathName[0] = '\0';
	file->type = TYPEVAL_INVALIDO;
	return 0;
}