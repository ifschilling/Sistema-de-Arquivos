#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"
#include "../include/functions.h"

#define MAX_NUM_FILES 10

/* GLOBAL VARIABLES */
dirDescription currentDir;
int bytesInBlock;
int firstDataBlock;
int inodeSector;
int inode_per_sector;
DWORD PointersInBlock;
fileHandler openedFiles[MAX_NUM_FILES];
struct t2fs_superbloco superBlock;



int setup(){
	//Create root directory and prepare the file
	if( (read_sector(0, buffer)) ){
		printf("Could not read first sector\n");
		return -1;
	}

	strncpy(superBlock.id,(char*) buffer, 4); //ID
	if(strcmp(superBlock.id, "T2FS") != 0){
		printf("Error, not T2FS\n");
		return -1;
	}
	char temp[4];

	strncpy(temp, buffer+4, 2); //version
	superBlock.version = (WORD)atoi(temp);

	strncpy(temp, buffer+6, 2);
	superBlock.superblockSize = (WORD)atoi(temp);

	strncpy(temp, buffer+8, 2);
	superBlock.freeBlocksBitmapSize =(WORD)atoi(temp);

	strncpy(temp, buffer+10, 2);
	superBlock.freeInodeBitmapSize =(WORD)atoi(temp);

	strncpy(temp, buffer+12, 2);
	superBlock.inodeAreaSize =(WORD)atoi(temp);

	strncpy(temp, buffer+14, 2);
	superBlock.blockSize =(WORD)atoi(temp);

	strncpy(temp, buffer+12, 2);
	superBlock.diskSize =(DWORD)atoi(temp);

	//Setting up global variables
	bytesInBlock = superBlock.blockSize * SECTOR_SIZE;
	firstDataBlock = (1 + superBlock.freeInodeBitmapSize + superBlock.freeBlocksBitmapSize + superBlock.inodeAreaSize);
	inodeSector = (1 + superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize)*blockSize;
	inodeAux = SECTOR_SIZE/sizeof(struct t2fs_inode);
	PointersInBlock= bytesInBlock/sizeof(DWORD);                     //ponteiros indiretos em um bloco de dados;
	inode_per_sector = SECTOR_SIZE / sizeof(DWORD);	
	
	///Initialise the root directory
	strcpy(currentDir.pathName, "/");
	currentDir.block = firstDataBlock;

	//First two records must be current directory and father directory
	struct t2fs_record rootRecord0, rootRecord1;
	rootRecord0.TypeVal = TYPEVAL_DIRETORIO;
	strcpy(rootRecord0.name, ".");
	rootRecord0.inodeNumber = 0;

	rootRecord1.TypeVal = TYPEVAL_DIRETORIO;
	strcpy(rootRecord1.name, "..");
	rootRecord1.inodeNumber = 0;

	//Inode initialization
	struct t2fs_inode rootInode;
	rootInode.blocksFileSize = 1;
	rootInode.bytesFileSize = 2*sizeof(rootRecord0);
	rootInode.dataPtr[0] = 0;
	rootInode.dataPtr[1] = INVALID_PTR;
	rootInode.singleIndPtr = INVALID_PTR;
	rootInode.doubleIndPtr = INVALID_PTR;

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

	int i;
	for(i=0; i<MAX_NUM_FILES; i++)
		openedFiles[i].type = TYPEVAL_INVALIDO;

	systemReady = 1;
	return 0;
}

struct coordinates getReg(char *path){
	char *token;
	char *nameaux;
	struct t2fs_record dirReg;
	struct coordinates info;
	char buffer[SECTOR_SIZE];

	switch(filenameDir(path)){
		case 0: //diretorio pai
		case 1: // diretorio atual
			// ele vai pro diretório pai após ler o token e chamar o getRegByName
			read_sector(blockFirstSector(currentDir.block), buffer);
			memcpy(&info.record, buffer, sizeof(struct t2fs_inode));
			break;
		case 2: // root
			read_sector(blockFirstSector(firstDataBlock), buffer);
			memcpy(&info.record, buffer, sizeof(struct t2fs_inode));
			break;
	}

	nameaux = (char *) malloc(strlen(path) +1);
	strcpy(nameaux, path);
	token = strtok(nameaux, "/");

	while(token != NULL){
		info = getReginDir(token, info.record);
		token = strtok(NULL, "/");
	}
	return info;
}

int setReg(struct t2fs_record reg, char *path){
struct coordinates info;
struct t2fs_record regAux;
DWORD buffer[SECTOR_SIZE];

	//primeiro saber se o registro ja existe, ou se precisa ser criado
	info = getReg(path);
	if(info.record.TypeVal == TYPEVAL_INVALIDO){	//registro nao existe, precisa ser criado no diretorio pai
	char *token;
	char lastoken[59];
	char *nameaux = malloc(strlen(path)+1);
		
		strcpy(nameaux, path);
		token = strtok(nameaux, "/");
		
		whille(token != NULL){
			token = strtok(NULL, "/");
			strcpy(lastoken,token);
		}
		strncpy(nameaux, path, (strlen(path)-strlen(lastoken)-1));		//    /p1/p2/p5/p4/p6/texto
		regAux = getReg(nameaux);
		info = getNewRegCoordinates(regAux);
		if(info.bytesread==-1){
			printf("ERROR creating new record, directory is already full (funtion : setReg) \n");
			return -1
		}
		if(read_sector(info.lastReadSector, buffer)){
			printf("ERROR, reading sector (function: setReg) \n");
			return -1;
		}
		memcpy(buffer + (info.lastAcessedPosition * sizeof(struct t2fs_record)), reg, sizeof(struct t2fs_record));
		if(write_sector(info.lastReadSector, buffer)){
			printf("ERROR, writing sector (function: setReg) \n");
			return -1;
		}
	}else{
		if(read_sector(info.lastReadSector, buffer)){
			printf("ERROR, reading sector (function: setReg) \n");
			return -1;
		}
		memcpy(buffer + (info.lastAcessedPosition * sizeof(struct t2fs_record)), reg, sizeof(struct t2fs_record));
		if(write_sector(info.lastReadSector, buffer)){
			printf("ERROR, writing sector (function: setReg) \n");
			return -1;
		}
	}
	return 0;
}

struct coordinates getReginDir(char *name, struct t2fs_record dirReg){
int dirBlocks=0, offsetDirBlock=0, offsetSingleIndBlock=0, t=0, offsetDoubleIndBlock=0, h=0, g=0, j=0, z=0;
DWORD rsector;
DWORD pointer=0, indpointer=0;
BYTE sindbuffer[SECTOR_SIZE];
BYTE dindbuffer[SECTOR_SIZE];
int bytesread=0;  //contabiliza o total de bytes que ja foram lidos
struct t2fs_record regAux;
struct t2fs_inode inodeAux;
struct coordinates info;

	
	if(dirReg.TypeVal==TYPEVAL_DIRETORIO){
		if(strcmp(name, ".") == 0){ // retorna o mesmo diretorio
			info.record = dirReg;
			return info; 
		}else if(strcmp(name, "..") == 0){  // retorna o diretorio pai
			inodeAux = getInode(dirReg.inodeNumber);
			if(inodeAux.bytesFileSize == -1){
				regAux.TypeVal == TYPEVAL_INVALIDO;
				info.record = regAux;
				return info;
			}
			read_sector(blockFirstSector(inodeAux.dataPtr[0]), sindbuffer);
			memcpy(&regAux, (sindbuffer + sizeof(struct t2fs_inode)), sizeof(struct t2fs_inode));
			info.lastReadSector = 0;
			info.lastAcessedPosition = 1;
			info.record = regAux;
			return info;
		}

		inodeAux = getInode(dirReg.inodeNumber);
		if(inodeAux.bytesFileSize == -1){
			regAux.TypeVal == TYPEVAL_INVALIDO;
			info.record = regAux;
			return info;
		}
		for(dirBlocks=0; dirBlocks < inodeAux.blocksFileSize; dirBlocks++){ //percorre blocos
			if(dirBlocks==0)
				rsector = (inodeAux.dataPtr[0] * superBlock.blockSize);
			else if(dirBlocks==1)
				rsector = (inodeAux.dataPtr[1] * superBlock.blockSize);
			else if(dirBlocks<(PointersInBlock+2)){ //INDIRECAO SIMPLES
					if(offsetSingleIndBlock < superBlock.blockSize){
						if(t==(SECTOR_SIZE/sizeof(DWORD)) || offsetSingleIndBlock == 0){
							read_sector((blockFirstSector(inodeAux.singleIndPtr)) + offsetSingleIndBlock, sindbuffer);
							offsetSingleIndBlock++;
							t=0;
						}
					}

					if(t < (SECTOR_SIZE/sizeof(DWORD))){
						memcpy(&pointer, sindbuffer+(t*sizeof(DWORD)), sizeof(DWORD));
						rsector = (blockFirstSector(pointer));
						t++;
					}
			}else{ //INDIRECAO DUPLA
				if(offsetDoubleIndBlock < superBlock.blockSize){
					if(h==SECTOR_SIZE/sizeof(DWORD) || offsetDoubleIndBlock == 0){
						read_sector((blockFirstSector(inodeAux.doubleIndPtr))+offsetDoubleIndBlock, dindbuffer);
						offsetDoubleIndBlock++;
						h=0;
					}
				}

				if((((blockFirstSector(indpointer)+(superblock.blockSize)) == (j+blockFirstSector(indpointer))) && g==(SECTOR_SIZE/sizeof(DWORD))) || h==0){
					memcpy(&indpointer, dindbuffer + (h*sizeof(DWORD)), sizeof(DWORD));
					h++;
					j=0;
				}
			
				if(j<superBlock.blockSize){
					if(g==(SECTOR_SIZE/sizeof(DWORD)) || j == 0){
						read_sector((blockFirstSector(indpointer)) + j, indbuffer);
						j++;
						g=0;
					}
				}

				if(g<(SECTOR_SIZE/sizeof(DWORD))){
					memcpy(&pointer, indbuffer+(g*sizeof(DWORD)), sizeof(DWORD));
					rsector = (blockFirstSector(pointer));
					g++;
				}
			}
			
			for(offsetDirBlock=0; offsetDirBlock<superBlock.blockSize; offsetDirBlock++){ //setores dentro do bloco
				read_sector(rsector + offsetDirBlock, buffer);
				z=0;
				do{
					memcpy(&regAux, buffer+(z*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
					if(regAux.TypeVal == TYPEVAL_REGULAR || regAux.TypeVal == TYPEVAL_DIRETORIO){
						bytesread += sizeof(struct t2fs_record);
						if(strcmp(regAux.name, name)==0){
							info.lastReadSector = rsector+offsetDirBlock;
							info.lastAcessedPosition = z;
							info.bytesread = bytesread;
							info.record = regAux;
							return info;
						}
					}
					z++;
				}while(z<(SECTOR_SIZE/sizeof(struct t2fs_record)) && bytesread<inodeAux.bytesFileSize);
				if(bytesread >=inodeAux.bytesFileSize){
					info.record.TypeVal = TYPEVAL_INVALIDO;
					return info;
				}
			}
		}
	}
	info.record.TypeVal = TYPEVAL_INVALIDO;
	return info;
}

struct coordinates getNewRegCoordinates(struct t2fs_record dirReg){
	int dirBlocks=0, offsetDirBlock=0, offsetSingleIndBlock=0, t=0, offsetDoubleIndBlock=0, h=0, g=0, j=0, z=0;
	int plus =0;
	DWORD rsector;
	DWORD pointer, indpointer;
	BYTE sindbuffer[SECTOR_SIZE];
	BYTE dindbuffer[SECTOR_SIZE];
	int bytesread =0;
	struct t2fs_record regAux;
	struct t2fs_inode inodeAux;
	struct t2fs_record newReg;
	struct coordinates info;

	if(dirReg.TypeVal==TYPEVAL_DIRETORIO){
		inodeAux = getInode(dirReg.inodeNumber);
		if(inodeAux.bytesFileSize == -1){
			regAux.TypeVal == TYPEVAL_INVALIDO;
			return regAux;
		}
		
		if(indeAux.bytesFileSize==(inodeAux.blocksFileSize * superBlock.blockSize * SECTOR_SIZE)){	//ARQUIVO CHEIO
			info.bytesread = -1;
			return info
		}
		
		newreg = getReg(path);
		
		if(((indeAux.bytesFileSize)%(inodeAux.blocksFileSize * superBlock.blockSize * SECTOR_SIZE))==0)	//checa se o final dos dados coincide com o final de algum bloco
			plus = 1;	
			
		for(dirBlocks=0; dirBlocks < (inodeAux.blocksFileSize+plus); dirBlocks++){ //percorre blocos
			if(dirBlocks==0)
				rsector = (inodeAux.dataPtr[0] * superBlock.blockSize);
			else if(dirBlocks==1)
				rsector = (inodeAux.dataPtr[1] * superBlock.blockSize);
			else if(dirBlocks<(PointersInBlock+2)){ //INDIRECAO SIMPLES
					if(offsetSingleIndBlock < superBlock.blockSize){
						if(t==(SECTOR_SIZE/sizeof(DWORD)) || offsetSingleIndBlock == 0){
							read_sector((blockFirstSector(inodeAux.singleIndPtr)) + offsetSingleIndBlock, sindbuffer);
							offsetSingleIndBlock++;
							t=0;
						}
					}

					if(t < (SECTOR_SIZE/sizeof(DWORD))){
						memcpy(&pointer, sindbuffer+(t*sizeof(DWORD)), sizeof(DWORD));
						rsector = (blockFirstSector(pointer));
						t++;
					}
			}else{ //INDIRECAO DUPLA
				if(offsetDoubleIndBlock < superBlock.blockSize){
					if(h==SECTOR_SIZE/sizeof(DWORD) || offsetDoubleIndBlock == 0){
						read_sector((blockFirstSector(inodeAux.doubleIndPtr))+offsetDoubleIndBlock, dindbuffer);
						offsetDoubleIndBlock++;
						h=0;
					}
				}

				if((((blockFirstSector(indpointer)+(superblock.blockSize)) == (j+blockFirstSector(indpointer))) && g==(SECTOR_SIZE/sizeof(DWORD))) || h==0){
					memcpy(&indpointer, dindbuffer + (h*sizeof(DWORD)), sizeof(DWORD));
					h++;
					j=0;
				}
			
				if(j<superBlock.blockSize){
					if(g==(SECTOR_SIZE/sizeof(DWORD)) || j == 0){
						read_sector((blockFirstSector(indpointer)) + j, indbuffer);
						j++;
						g=0;
					}
				}

				if(g<(SECTOR_SIZE/sizeof(DWORD))){
					memcpy(&pointer, indbuffer+(g*sizeof(DWORD)), sizeof(DWORD));
					rsector = (blockFirstSector(pointer));
					g++;
				}
			}

			for(offsetDirBlock=0; offsetDirBlock<superBlock.blockSize; offsetDirBlock++){ //setores dentro do bloco
				read_sector(rsector + offsetDirBlock, buffer);
				z=0;
				do{
					memcpy(&regAux, buffer+(z*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
					if(regAux.TypeVal !== TYPEVAL_INVALIDO)
						bytesread += sizeof(struct t2fs_record);	
							if(regAux.TypeVal == TYPEVAL_INVALIDO){
									info.lastReadSector = rsector+offsetDirBlock;
									info.lastAcessedPosition = z;
									info.bytesread = bytesread;
									info.record = regAux;
									return info;
								}else if(bytesread>inodeAux.bytesFileSize){		//se nao houver registros invalidos retornara as "coordenadas apontando" para uma posicao após o ultimo registro valido
									info.lastReadSector = rsector+offsetDirBlock;
									info.lastAcessedPosition = z;
									info.bytesread = bytesread;
									return info;
								}
					z++;
				}while(z<(SECTOR_SIZE/sizeof(struct t2fs_record)));
			}
		}
	}
}

struct t2fs_inode getInode(DWORD inodeNumber){
int blockOffset = (int) (inodeNNumber/inodes_per_sector);
int sectorOffset = (int) (inodeNumber % inodes_per_sector);
struct t2fs_inode inode;

	if(read_sector(inodeSector + blockOffset, buffer)){
			printf("read_sector ERROR! (Function: getInode) \n");
			inode.bytesFileSize = -1;
			return inode;
	}
	memcpy(&inode, buffer + sectorOffset, sizeof(struct t2fs_inode));

	return inode;
}

struct t2fs_record readRegs(struct t2fs_inode inode){
struct t2fs_record regAux;

	
	for(; info.dirBlocks < inode.blocksFileSize; info.dirBlocks++){ //percorre blocos
		if(info.dirBlocks==0)
			rsector = (inode.dataPtr[0] * superBlock.blockSize);
		else if(info.dirBlocks==1)
			info.rsector = (inode.dataPtr[1] * superBlock.blockSize);
		else if(info.dirBlocks<(PointersInBlock+2)){ //INDIRECAO SIMPLES
				if(info.offsetSingleIndBlock < superBlock.blockSize){
					if(info.t==(SECTOR_SIZE/sizeof(DWORD)) || info.offsetSingleIndBlock == 0){
						read_sector((blockFirstSector(inode.singleIndPtr)) + info.offsetSingleIndBlock, info.sindbuffer);
						info.offsetSingleIndBlock++;
						info.t=0;
					}
				}

				if(info.t < (SECTOR_SIZE/sizeof(DWORD))){
					memcpy(&info.pointer, info.sindbuffer+(info.t*sizeof(DWORD)), sizeof(DWORD));
					info.rsector = (blockFirstSector(info.pointer));
					info.t++;
				}
		}else{ //INDIRECAO DUPLA
			if(info.offsetDoubleIndBlock < superBlock.blockSize){
				if(info.h==SECTOR_SIZE/sizeof(DWORD) || info.offsetDoubleIndBlock == 0){
					read_sector((blockFirstSector(inode.doubleIndPtr))+info.offsetDoubleIndBlock, info.dindbuffer);
					info.offsetDoubleIndBlock++;
					info.h=0;
				}
			}

			if((((blockFirstSector(info.indpointer)+(superblock.blockSize)) == (info.j+blockFirstSector(info.indpointer))) && info.g==(SECTOR_SIZE/sizeof(DWORD))) || info.h==0){
				memcpy(&info.indpointer, info.dindbuffer + (info.h*sizeof(DWORD)), sizeof(DWORD));
				info.h++;
				info.j=0;
			}

			if(info.j<superBlock.blockSize){
				if(info.g==(SECTOR_SIZE/sizeof(DWORD)) || info.j == 0){
					read_sector((blockFirstSector(info.indpointer)) + info.j, info.indbuffer);
					info.j++;
					info.g=0;
				}
			}

			if(info.g<(SECTOR_SIZE/sizeof(DWORD))){
				memcpy(&info.pointer, info.indbuffer+(info.g*sizeof(DWORD)), sizeof(DWORD));
				info.rsector = (blockFirstSector(info.pointer));
				info.g++;
			}
		}

		for(; info.offsetDirBlock<superBlock.blockSize; info.offsetDirBlock++){ //setores dentro do bloco
			read_sector(info.rsector + info.offsetDirBlock, info.buffer);
			info.z=0;
			do{
				memcpy(&regAux, info.buffer+(info.z*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
				if(regAux.TypeVal == TYPEVAL_REGULAR || regAux.TypeVal == TYPEVAL_DIRETORIO){
					info.bytesread += sizeof(struct t2fs_record);
					return regAux;
				}
				info.z++;
			}while(info.z<(SECTOR_SIZE/sizeof(struct t2fs_record)) && info.bytesread<inode.bytesFileSize);
			if(info.bytesread >=inode.bytesFileSize){
				regAux.TypeVal = TYPEVAL_INVALIDO;
				return regAux;
			}
		}
	}
}

int setInode(struct t2fs_inode inode, DWORD inodeNumber){
DWORD iSector, iPosition;
iSector = inodeSector + (inodeNumber/inode_per_sector);
iPosition = inodeNumber % inode_per_sector;

	unsigned char iBuf[SECTOR_SIZE];
	if(getBitMap2(BITMAP_INODE, inodeNumber)==0) //checa se o inode a ser modificado, "ja existe/esta ocupado"
			if(setBitMap2(BITMAP_INODE, inodeNumber, 1))
					printf("ERROR, trying to update inode bitmap! (Function: modifyInode) \n");

	if(read_sector(iSector, iBuf) ){
			printf("ERROR reading sector: %i in modifyInode \n", iSector);
			return -1;
	}
	/*  Now iBuf has our iNode and others as well, we must modify only the one we want */
	/*  e.g inode number 14, if 3 per sector 14/3 = 4; 14%3 = 1 so sector 4, struct 1*/
	memcpy(iBuf + (iPosition*sizeof(t2fs_inode)), inode, sizeof(t2fs_inode));
	if(write_sector(iSector, iBuf) ){
			printf("ERROR writing sector %i in modifyInode", iSector);
			return -1;
	}
	return 0;
}

int absolutePath(char *abspath, char *path){
	char fullpathname[MAX_FILE_NAME_SIZE +1];
	char name[6][59], *pathaux, *token, *current;
	int cont = 0;
	pathaux = (char *) malloc(strlen(path) +1);
	strcpy(pathaux, path);
	switch(filenameDir(pathaux)){
		case 0: //diretorio pai
		case 1: // diretorio atual
			// separa o diretorio corrente em tokens e os passa para name
			current = (char *) malloc(strlen(currentDir.pathName) +1);
			strcpy(current, currentDir.pathName);
			token = strtok(current, "/");
			while(token != NULL){
				strcpy(name[cont], token); // passa o token pra uma string do nome
				cont++;
				token = strtok(NULL, "/");
			}
			break;
		case 2: // root
			// faz nada, pois o root começa vazio
			break;
	}
	
	token = strtok(pathaux, "/");
	while(token != NULL){
		if(strcmp(token, ".") == 0){
			// se é o diretorio atual, faz nada
		}
		else if(strcmp(token, "..") == 0){
			// diretorio pai, apaga o ultimo name salvo
			if(cont > 0){
				cont--;
			}
		}
		else{
			strcpy(name[cont], token); // passa o token pra uma string do nome
			cont++;
			if(cont >= 6){
				printf("ERROR: too many directories in path!\n");
	    	return 1;
			}
		}
		token = strtok(NULL, "/");
	}
	
	//concatena todo o caminho em fullpathname
	int i;
	fullpathname[0] = '\0';
	for(i=0; i<cont; i++){
		strcat(fullpathname, "/");
		strcat(fullpathname, name[i]);
	}
	if(strlen(fullpathname) == 0){
		strcpy(fullpathname, "/");
	}
	
	strcpy(abspath, fullpathname);
	return 0;
}

int filenameDir(char *filename){
	if(filename[0]=='/'){
  	return 2;
  }
  else if(filename[0]=='.' && filename[1]=='.' && filename[2]=='/'){
	  return 0;
  }
  else{
  	return 1;
  }
}

unsigned int blockFirstSector(unsigned int block){
	return (block * superBlock.blockSize);
}

int delBlocks(DWORD inode, int cPointer){
  
  //read inode 
  t2fs_inode delNode= getInode(inode);
  DWORD blocksRemaining = delNode.blocksFileSize;
  DWORD iBSector = SECTOR_SIZE/sizeof(DWORD);/*pointers by sector*/

	DWORD dBlocksToDel;
	BYTE delBuff;
	DWORD indexSectorsToRead;
	DWORD sFirstBlockSector;
	DWORD bIndex, bDataFinalPosition;
	dBlocksToDel = blocksRemaining;

/*-----------------------------------------------------------------------*/
	DWORD trunc_blocksToKeep = cPointer/bytesInBlock;/*biB 16; cp 17(0,5,15)(16,31) */
	if(cPointer % bytesInBlock != 0)
				trunc_blocksToKeep++;

	DWORD trunc_indexToDel, trunc_deletedBlocks = 0;
	DWORD trunc_firstBlockToDel = trunc_blocksToKeep + 1;
	DWORD trunc_blocksToDel = delNode.blocksFileSize - trunc_blocksToKeep;
/*-----------------------------------------------------------------------*/
	DWORD arrayOfDataBlocks[dBlocksToDel];
  
	int i, j;
	int k, l;

      // From end to the first block
	if(blocksRemaining > (2 + PointersInBlock)){ /* double Indirection */
  	
		DWORD dfirstBlockSector = blockFirstSector(delNode.doubleIndPtr);
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
    	if( read_sector(dfirstBlockSector + i, &delBuff) ){
	    	printf("Error in delBlocks,failed to read sector %i\n", dfirstBlockSector+i);
	    	return -1;
	    }
	    for(j = 0; j<iBSector;j++){
	    	if(k < bIndex){
	    		memcpy(&arrayOfBlockIndex[k], delBuff+(sizeof(DWORD)*j), sizeof(DWORD));/*This is the block of indexes*/
	    		k++;
	    	}
	    }
	  }
		/* We have all the blocks of index */
		l = 0;
		for( i = 0; i < bIndex, i++){/*Each index block*/
	  	sFirstBlockSector = blockFirstSector(arrayOfBlockIndex[i]);/*First Block of the single index block*/
	  	for(j = 0; j < superBlock.blockSize; j++){
	  		if( read_sector(sFirstBlockSector + j, &delBuff) ){
	    		printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
	    		return -1;
	    	}
	    	for(k = 0; k <iBSector; k++){
	    		if(l < dBlocksToDel){
	    			memcpy(&arrayOfDataBlocks[l], delBuff + (k*sizeof(DWORD)), sizeof(DWORD));
	    			l++;
	    		}
	    	}
	  	}
	  }

	  if(cPointer == -1 || cPointer == 0){/*Now we have to free all the blocks*/
		  for(i = 0; i< bIndex; i++){
		  	if ( setBitmap2(BITMAP_DADOS,arrayOfBlockIndex[i], 0) ){
          printf("ERROR setting bits in delBlocks\n");
          return -1;
        }
		  }
      
      for(i = 0; i< dBlocksToDel; i++){
		  	if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) ){
          printf("ERROR setting bits in delBlocks\n");
          return -1;
        }
		  }
		  if ( setBitmap2(BITMAP_DADOS,delNode.doubleIndPtr, 0) ){
          printf("ERROR setting bits in delBlocks\n");
          return -1;
        }
      blocksRemaining -= dBlocksToDel;
	  }
	  else{/*Truncate*/
	  	/*We must delete from the last to first entry in the arrayOfDataBlocks*/
	  	for(i = (dBlocksToDel-1); i >= 0; i--;){ /*Delete the last blocks*/
	  		if(trunc_blocksToDel > 0){
	  			if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) ){
         		printf("ERROR setting bits in delBlocks\n");
         		return -1;
       		}
       		trunc_blocksToDel--;
	  		}
	  	}

	  	/*pB = 4; A(0,1,2,3)B(4,5,6,7)C(8,9,x,11) */
  		k=0;
  		DWORD trunc_matrix[bIndex][PointersInBlock];
  		for(i = 0; i< bIndex; i++){
  			for(j = 0; j <PointersInBlock; j++){
  				if(k<dBlocksToDel){
  					trunc_matrix[i][j] = arrayOfDataBlocks[(i*PointersInBlock)+j];
  					k++;
  				}
  				
  			}
  		}
  		/*Find how many index blocks we can delete*/
  		int fInd = -1;
  		int fPos;
  		trunc_indexToDel = 0;
  		k = 0;
  		for(i = 0; i< bIndex; i++){
  			for(j = 0; j <PointersInBlock; j++){
  				if(k<dBlocksToDel){
  					if( getBitmap2(trunc_matrix[i][j]) == 0){/*Deleted block*/
  						if(fInd == -1){
  							fPos = j;
  							fInd = i;
  						}
  					}
  					k++;
  				}
  			}
  		}
  		if( fPos == 0){
  			trunc_indexToDel = bIndex-fInd;
  		}
  		else{
  			trunc_indexToDel = bIndex- find - 1;
  		}
	  	k = trunc_indexToDel;

	  	for(i = (bIndex -1) ; i>=0; i--){
	  			if(trunc_indexToDel > 0){
	  				if ( setBitmap2(BITMAP_DADOS,arrayOfBlockIndex[i], 0) ){
         			printf("ERROR setting bits in delBlocks\n");
         			return -1;
       			}
       			trunc_indexToDel--;
	  			}
	  	}
	  	if(k == bIndex){//We had to delete all the double indirection blocks
	  		 if ( setBitmap2(BITMAP_DADOS,delNode.doubleIndPtr, 0) ){
          printf("ERROR setting bits in delBlocks\n");
          return -1;
        }
	  	}
	  	blocksRemaining -= dBlocksToDel;

	  	//end of else truncate
	  }	  	
  }

  /*Single indirection*/
  if(blocksRemaining > 2){
  	
		dBlocksToDel = blocksRemaining - 2;
		sFirstBlockSector = blockFirstSector(delNode.singleIndPtr);/*First Block of the single index block*/

    indexSectorsToRead = dBlocksToDel /iBSector;//
    if( (bIndex % iBSector) != 0 )
    	indexSectorsToRead++;

    l = 0;
  	for(j = 0; j < indexSectorsToRead; j++){
  		if( read_sector(sFirstBlockSector + j, &delBuff) ){
    		printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
    		return -1;
    	}
    	for(k = 0; k <iBSector; k++){
    		if(l < dBlocksToDel){
    			memcpy(&arrayOfDataBlocks[l], delBuff + (k*sizeof(DWORD)), sizeof(DWORD));
    			l++;
    		}
    	}
  	}

  	/*Release the blocks*/
  	if(cPointer == -1 || cPointer == 0){
	  	if ( setBitmap2(BITMAP_DADOS,delNode.singleIndPtr, 0) ){
	      printf("ERROR setting bits in delBlocks\n");
	      return -1;
	    }

	  	for(i = 0; i< dBlocksToDel; i++){
			  if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) ){
	        printf("ERROR setting bits in delBlocks\n");
	        return -1;
	      }
			}

	      blocksRemaining -= dBlocksToDel;
	      //end of delete all
	  }
	  
  	
  	else{ /*TRUNCATE*/	

		  for(i = (dBlocksToDel-1); i >= 0; i--;){ /*Delete the last blocks*/
		  		if(trunc_blocksToDel > 0){
		  			if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) ){
	         		printf("ERROR setting bits in delBlocks\n");
	         		return -1;
	       		}
	       		trunc_blocksToDel--;
		  		}
		  	}
		  if(trunc_blocksToDel > 0){/*The currentPointer is in the direct blocks*/
		  	if ( setBitmap2(BITMAP_DADOS,delNode.singleIndPtr, 0) ){
		      printf("ERROR setting bits in delBlocks\n");
		      return -1;
		    }
		  }

		  blocksRemaining -= dBlocksToDel;
  	//end of truncate
  	}
    
    return 0;   
  }

  /*Direct blocks*/
  if(cPointer == -1 || cPointer == 0){/*DELETE ALL*/
  	if( setBitmap2(BITMAP_DADOS, delNode.dataPtr[0], 0) ){
  		printf("ERROR setting bits in delBlocks\n");
  		return -1;
  	}
  	if( setBitmap2(BITMAP_DADOS, delNode.dataPtr[1], 0) ){
  		printf("ERROR setting bits in delBlocks\n");
  		return -1;
  	}
  }
  else if(trunc_blocksToDel != 0){/*TRUNCATE*/

  	 for(i = 1; i >= 0; i--;){ /*Delete the last blocks*/
		  		if(trunc_blocksToDel > 0){
		  			if ( setBitmap2(BITMAP_DADOS,delNode.dataPtr[i], 0) ){
	         		printf("ERROR setting bits in delBlocks\n");
	         		return -1;
	       		}
	       		trunc_blocksToDel--;
		  		}
		  	}
  }
  
  return 0;
  
}

int isFileOpen(DWORD inode){
  fileHandler temp;
  int i;
  for(i = 0; i < MAX_NUM_FILES; i++){
    temp = openedFiles[i];
    if( temp.numInode == inode)
      return 1;
  }
  return 0;
}