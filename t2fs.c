/* Arthur Böckmann Grossi (275607), Cassiano Translatti Furlani(278038) e Ian Fischer Schilling(275603) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"

#define MAX_NUM_FILES 10
#define MAX_STU_CHAR 100
#define STU1 "Arthur Bockmann Grossi - 275607"
#define STU2 "Cassiano Translatti Furlani - 278038"
#define STU3 "Ian Fischer Schilling - 275603"

#define MAX_PATH_NAME_SIZE 301	//CECHIN falou em aula que o maior caminho sera 4 diretorios aninhados, para garantir calculamos para 5 diretorios.

typedef struct files
{
    char fullPathName[MAX_FILE_NAME_SIZE+1];
    BYTE type;
    int currentPointer; // bytes
    struct t2fs_inode inode;
    DWORD numInode;
} fileHandler;

typedef struct cD
{
    char pathName[MAX_FILE_NAME_SIZE +1];
    int block;
} dirDescription;

struct coordinates
{
    unsigned int lastAcessedPosition;
    unsigned int lastReadSector;
    int bytesread;
    struct t2fs_record record;
};

struct read_dir
{
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
};

// SETUP
int setup();

//GETTERS E SETTERS
/*-----------------------------------------------------------------------------
Função: Acha um registro que tenha o mesmo nome do último argumento de path.
Entrada: path -> Caminho absoluto ou relativo do registro(onde o ultimo argumento sera o nome do registro a ser procurado).
Saída: Se a operação foi realizada com sucesso, a função retorna a struct coordinates com o registro do arquivo encontrado, junto com o ultimo setor acessado, ultima posicao acessada e numero de bytes lidos.
		    Em caso de erro, será retornado o registro com TypeVal = TYPEVAL_INVALIDO. (testar struct coordinates.registro.Typeval == TYPEVAL_INVALIDO)
Exemplo: getReg("/p1/p2/p3") => retorna o registro do diretorio p3
        getReg("/p1/p2/p3/texto.txt") => retorna o registro do arquivo texto.txt
				getReg("../p2/p3/texto.txt") => retorna o registro do arquivo texto.txt
-----------------------------------------------------------------------------*/
struct coordinates getReg(char *path);

/*-----------------------------------------------------------------------------
Função: Copia os dados do registro reg para o registro dado por abspath.
Entrada: path -> Caminho absoluto ou relativo do registro que será alterado ou criado (o último argumento é o nome do registro a ser alterado).
          reg -> estrutura com os campos que serão copiados para o registro apontado por regname
Saída: Se a operação foi realizada com sucesso, a função retorna 0.
		    Em caso de erro, será retornado -1.
Exemplo: setReg(reg, "/p1/p2/p3") => alterará o registro do diretorio p3
        setReg(reg, "/p1/p2/p3/texto.txt") => alterará o registro do arquivo texto.txt
-----------------------------------------------------------------------------*/
int setReg(struct t2fs_record reg, char *path);

/*-----------------------------------------------------------------------------
Função:	Procura um registro VALIDO com nome igual a name, no diretorio apontado por dirReg
Entra:	name -> nome do arquivo a ser procurado.
        Se for '.' retorna o mesmo diretorio.
        Se for '..' retorna o diretório pai.
        dirReg -> Registro do diretorio onde o a busca sera feita
Saída:	Se a operação foi realizada com sucesso, a função retorna a struct coordinates com o registro do arquivo encontrado, junto com o ultimo setor acessado, ultima posicao acessada e numero de bytes lidos.
		    Em caso de erro, será retornado o registro dentro da struct coordinates com TypeVal = TYPEVAL_INVALIDO.
-----------------------------------------------------------------------------*/
struct coordinates getReginDir(char *name, struct t2fs_record dirReg); // antigo getRegByName

/*-----------------------------------------------------------------------------
Função:	Acha o ultimo registro valido ou o primeiro registro invalido no diretorio apontado por dirReg
Entra:	dirReg -> Registro do diretorio onde o a busca sera feita
Saída:	Retorna uma estrutura coordinates, contendo a ultima posicao e o ultimo setor acessado (setor e posicao onde sera criado o novo registro);
-----------------------------------------------------------------------------*/
struct coordinates getNewRegCoordinates(struct t2fs_record dirReg);

/*-----------------------------------------------------------------------------
Função:	Faz uma varredura dos blocos do inode passado, com base em variaveis globais da estrutura info. É usada para implementar a readdir2.
Entra:	inode-> inode onde a varredura sera feita
Saída:	Retorna uma estrutura t2fs_record, contendo o registro encontrado.
				Caso de erro, retorna o registro com TypeVal = TYPEVAL_INVALIDO.
-----------------------------------------------------------------------------*/
struct t2fs_record readRegs(struct t2fs_inode inode);

/*-----------------------------------------------------------------------------
Função:	Procura inode com o índice de inodeNumber
Entra:	inodeNumber -> índice do inode a ser retornado.
Saída:	sucesso, retorna o inode
        erro de leitura do setor, retorna blocksFileSize = -1 (obs: testar se é igual a -1, se testar se é menor que 0 da erro)
-----------------------------------------------------------------------------*/
struct t2fs_inode getInode(DWORD inodeNumber);

/*-----------------------------------------------------------------------------
Função: Inode de índice inodeNumber vira uma cópia da estrutura inode passada
Entrada: inodeNum -> número do inode
         inode -> estrutura do inode que sera copiada para o inode apontado por inodeNumber

Saída: Retorna 0 se for realizada com sucesso, senão retorna 1.
-----------------------------------------------------------------------------*/
int setInode(struct t2fs_inode inode, DWORD inodeNumber);

//MANIPULACAO DE CAMINHO
/*-----------------------------------------------------------------------------
Função: Recebe um caminho e retorna esse caminho na forma absoluta
Entrada: path-> caminho a ser transformado

Saída: abspath: devolve o caminho absoluto se converter com sucesso
retorna 0 se converteu com sucesso, senao retorna 1
-----------------------------------------------------------------------------*/
int absolutePath(char *abspath, char *path);

// AUXILIARES
/*---------------------------------------------------------------------------------
Função: Retorna o tipo do primeiro diretório de filename
Entrada: filename -> nome do arquivo ou diretório
Saída: Se a operação for realizada com sucesso, retorna
        0 -> diretório pai
        1 -> diretório atual
        2 -> root
-----------------------------------------------------------------------------------*/
int filenameDir(char *filename);

/*---------------------------------------------------------------------------------
Função: Retorna o primeiro setor correspondente ao bloco
Entrada: block -> numero do bloco
Saída: retorna sempre o primeiro setor do bloco
-----------------------------------------------------------------------------------*/
unsigned int blockFirstSector(unsigned int block);

/*-----------------------------------------------------------------------------------
Função: Percorre os blocos que estão sendo utilizados pelo inode e libera eles.
Se o cPointer for -1 essa função deleta todos os blocos do arquivo e libera o bitmap de blocos do mesmo.
Se tiver outro valor maior ou igual a zero trunca o arquivo a partir de cPointer bytes
Entrada:  inode -> número do inode do arquivo
					cPointer -> ponteiro da posicao corrente do arquivo
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int delBlocks(DWORD inode, int cPointer);

/*-----------------------------------------------------------------------------------
Função: Percorre os arquivos abertos e
Entrada:  inode -> número do inode do arquivo
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int isFileOpen(DWORD inode);

/* GLOBAL VARIABLES */
dirDescription currentDir;
int bytesInBlock;
int firstDataBlock;
int inodeSector;
int inode_per_sector;
DWORD PointersInBlock;
fileHandler openedFiles[MAX_NUM_FILES];
struct t2fs_superbloco superBlock;
struct read_dir info_dir;
int systemReady = 0;



/* MAIN FUNCTIONS */

int identify2 (char *name, int size)
{
    char student[MAX_STU_CHAR] = "";
    int i;
    int st2 = strlen(STU1);
    int st3 = st2 + strlen(STU2);
    int letters;
    strcat(student, STU1);
    strcat(student, STU2);
    strcat(student, STU3);
    if(size >= MAX_STU_CHAR)  //Size is equal to or greater than students names
    {
        for( i = 0; i < MAX_STU_CHAR; i++)
            name[i] = student[i];
        return 0;
    }
    else if( size >= 9)  //At least 1 characters for each
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

FILE2 create2 (char *filename)
{
    struct coordinates info;
    int d=0;
    char abspath[MAX_PATH_NAME_SIZE];


    if(!systemReady)
        setup();
    info = getReg(filename);
    if(info.record.TypeVal == TYPEVAL_INVALIDO)  //nome valido, nao utilizado ainda
    {
        do
        {
            if(openedFiles[d].type == TYPEVAL_INVALIDO)
            {
                openedFiles[d].type == TYPEVAL_REGULAR;
                openedFiles[d].currentPointer = 0;
                if(absolutePath(abspath, filename))
                {
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
                if(setInode(newFileInode, openedFiles[d].numInode))
                {
                    printf("ERROR: error in setInode! (function: create2) \n");
                    return -1;
                }

                struct t2fs_record newReg;
                char *token;
                char lastoken[59];
                char *auxpath = malloc(strlen(filename)+1);

                //nome é só o ultimo argumento do filename
                newReg.TypeVal = TYPEVAL_REGULAR;
                token = strtok(auxpath, "/");
                strcpy(lastoken, token);
                while(token!=NULL)
                {
                    token = strtok(NULL, "/");
                    strcpy(lastoken, token);
                }

                strcpy(newReg.name, lastoken);
                newReg.inodeNumber = openedFiles[d].numInode;

                free(auxpath);

                if(setReg(newReg, filename))
                {
                    printf("ERROR, could not create new record inside directory (function: create2) \n");
                    return -1;
                }
                return d;	//handle do arquivo == posicao dele no vetor de arquivos abertos
            }
            d++;
        }
        while(d<MAX_NUM_FILES);

        printf("ERROR, there was already %i opened files! (function: create2)\n", MAX_NUM_FILES);
        return -1;

    }
    else
    {
        printf("ERROR, there are other file(s) with the same name in this directory! (function: create2)\n");
        return -1;
    }
}

int delete2 (char *filename)
{
    struct coordinates fileCoord = getReg(filename);
    struct t2fs_record fileReg = fileCoord.record;
    DWORD fileInode = fileReg.inodeNumber;

    if( isFileOpen(fileInode))
    {
        printf("Tried to delete an open file\n");
        return -1;
    }

    if(fileReg.TypeVal == TYPEVAL_INVALIDO)
    {
        printf("Tried to delete an invalid file\n");
        return -1;
    }

    if(fileReg.TypeVal == TYPEVAL_DIRETORIO)
    {
        printf("Tried to delete a directory\n");
        return -1;
    }
    /*It is a real file, now we can delete it*/

    if(delBlocks(fileInode, -1))
    {
        printf("Error deleting blocks in delete2\n");
        return -1;
    }
    /*Now we free the inode*/
    if( setBitmap2(BITMAP_INODE, fileInode, 0) )
    {
        printf("Error setting inode bitmap in delete2\n");
        return -1;
    }

    fileReg.TypeVal == TYPEVAL_INVALIDO;
    strcpy(fileReg.name, "\0");
    fileReg.inodeNumber = INVALID_PTR;
    /*We now must change the file record*/
    if(setReg(fileReg, filename))
    {
        printf("Error setting file record in delete2\n");
        return -1;
    }



}

FILE2 open2 (char *filename)
{
    struct coordinates reg;
    struct t2fs_inode inode;
    char *abspath;


    reg = getReg(filename);

    if(reg.record.TypeVal == TYPEVAL_INVALIDO)
    {
        printf("ERROR: file not found!\n");
        return -1;
    }

    int i=0;
    //searches for the next free openedFiles
    while((i<MAX_NUM_FILES) && (openedFiles[i].type != TYPEVAL_INVALIDO))
        i++;

    if(i >= MAX_NUM_FILES)
    {
        printf("ERROR: openedFiles is full!\n");
        return -1;
    }

    if(absolutePath(abspath, filename))
    {
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

int close2 (FILE2 handle)
{
    if(handle < 0 || handle >=MAX_NUM_FILES)
    {
        printf("ERROR: handle not valid!\n");
        return -1;
    }

    fileHandler *file;
    file = &(openedFiles[handle]);

    if (file->type != TYPEVAL_REGULAR)
    {
        printf("ERROR: handle file not found!\n");
        return -1;
    }

    file->fullPathName[0] = '\0';
    file->type = TYPEVAL_INVALIDO;
    return 0;
}

int read2 (FILE2 handle, char *buffer, int size)
{
    fileHandler readFile;
    if(handle <0 || handle >= MAX_NUM_FILES)
    {
        printf("Error at read2 invalid handle\n");
        return -1;
    }
    readFile = openedFiles[handle];

    if(readFile.type == TYPEVAL_INVALIDO)
    {
        printf("Tried to read an invalid file\n");
        return -1;
    }

    if(readFile.type == TYPEVAL_DIRETORIO)
    {
        printf("Tried to read a directory in read2\n");
        return -1;
    }

    struct t2fs_inode fileINode = readFile.inode;




    DWORD cPointerBlock = readFile.currentPointer/bytesInBlock;/*Block in the file, not the real block*/
    DWORD cPointerPositionInBlock = readFile.currentPointer%(bytesInBlock*cPointerBlock);/*Byte in the block*/
    DWORD cSector;
    DWORD cPSector = cPointerPositionInBlock/SECTOR_SIZE;/*Sector in block*/
    DWORD cPInSector = cPointerPositionInBlock%SECTOR_SIZE;/*Position in the sector*/
    DWORD bytesRemainingInBlock = bytesInBlock - cPointerPositionInBlock;
    DWORD bytesRemainingInSector = SECTOR_SIZE - cPInSector;

    BYTE cpBuff[SECTOR_SIZE];

    int blocksToRead = (size - bytesRemainingInBlock)/bytesInBlock;
    if(bytesRemainingInBlock != 0)
        blocksToRead++;

    int bytesToRead;
    if ( size + readFile.currentPointer > (readFile.inode.bytesFileSize+1))
    {
        bytesToRead = (readFile.inode.bytesFileSize - readFile.currentPointer)+1;
    }
    else
        bytesToRead = size;




    /*----------------*/

    DWORD blocksRemaining = fileINode.blocksFileSize;
    DWORD iBSector = SECTOR_SIZE/sizeof(DWORD);/*pointers by sector*/

    DWORD dBlocksToDel;
    BYTE delBuff[SECTOR_SIZE];
    DWORD indexSectorsToRead;
    DWORD sFirstBlockSector;
    DWORD bIndex;
    dBlocksToDel = blocksRemaining;
    DWORD arrayOfDataBlocks[dBlocksToDel];
    int i,j, k, l = 0;



    if(blocksRemaining > (2 + PointersInBlock))  /* double Indirection */
    {

        DWORD dfirstBlockSector = blockFirstSector(fileINode.doubleIndPtr);
        dBlocksToDel = blocksRemaining - ( 2 + PointersInBlock);

        bIndex = (dBlocksToDel / PointersInBlock) + 1;/*double indirection block of index*/

        /*We have to read just the correct amount of block index*/
        indexSectorsToRead = bIndex /iBSector;//
        if( (bIndex % iBSector) != 0 )
            indexSectorsToRead++;

        /*array of blocks of index and their dataBlocks*/
        DWORD arrayOfBlockIndex[bIndex];

        k = 0;
        for(i = 0; i < indexSectorsToRead; i++)
        {
            /*block of index in each sector*/
            if( read_sector(dfirstBlockSector + i, delBuff) )
            {
                printf("Error in delBlocks,failed to read sector %i\n", dfirstBlockSector+i);
                return -1;
            }
            for(j = 0; j<iBSector; j++)
            {
                if(k < bIndex)
                {
                    memcpy(&arrayOfBlockIndex[k], (delBuff+(sizeof(DWORD)*j)), sizeof(DWORD));/*This is the block of indexes*/
                    k++;
                }
            }
        }
        blocksRemaining -= dBlocksToDel;
        /* We have all the blocks of index */
        for( i = 0; i < bIndex; i++) /*Each index block*/
        {
            sFirstBlockSector = blockFirstSector(arrayOfBlockIndex[i]);/*First Block of the single index block*/
            for(j = 0; j < superBlock.blockSize; j++)
            {
                if( read_sector(sFirstBlockSector + j, delBuff) )
                {
                    printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
                    return -1;
                }
                for(k = 0; k <iBSector; k++)
                {
                    if(l < dBlocksToDel)
                    {
                        memcpy(&arrayOfDataBlocks[blocksRemaining+l], (delBuff + (k*sizeof(DWORD))), sizeof(DWORD));
                        l++;
                    }
                }
            }

        }

    }
    if(blocksRemaining > 2 )
    {

        dBlocksToDel = blocksRemaining- 2;
        sFirstBlockSector = blockFirstSector(fileINode.singleIndPtr);/*First Block of the single index block*/

        indexSectorsToRead = dBlocksToDel /iBSector;//
        if( (bIndex % iBSector) != 0 )
            indexSectorsToRead++;
        blocksRemaining -=dBlocksToDel;
        l = 0;
        for(j = 0; j < indexSectorsToRead; j++)
        {
            if( read_sector(sFirstBlockSector + j, delBuff) )
            {
                printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
                return -1;
            }
            for(k = 0; k <iBSector; k++)
            {
                if(l < dBlocksToDel)
                {
                    memcpy(&arrayOfDataBlocks[blocksRemaining+l], (delBuff + (k*sizeof(DWORD))), sizeof(DWORD));
                    l++;
                }
            }
        }
    }

    if(blocksRemaining <= 2 )
    {
        l = 0;
        dBlocksToDel = blocksRemaining;
        for(i = 0; i<2; i++)
        {
            if(l < dBlocksToDel)
            {
                memcpy(&arrayOfDataBlocks[l], &(fileINode.dataPtr[l]), sizeof(DWORD));
                l++;
            }
        }
    }
    /*----------------*/
    /*Now we have all the block numbers in arrayOfDataBlocks*/

    /*First we read the currentPointer block only*/
    int offset = bytesRemainingInSector;
    DWORD bytes_Read;
    cSector = blockFirstSector(arrayOfDataBlocks[cPointerBlock]);
    for(i = cPSector; i < superBlock.blockSize; i++)
    {
        if( read_sector(cSector + i, cpBuff) )
        {
            printf("ERROR reading sector read2\n");
            return -1;
        }
        for(j = 0; j < SECTOR_SIZE; j++)
        {
            if(bytesToRead > 0)
            {
                if(i == cPSector)
                {
                    if( bytesRemainingInSector > 0 )
                    {
                        memcpy(buffer + j,cpBuff + (cPInSector+j), sizeof(BYTE));
                        bytesRemainingInSector--;
                        bytesToRead--;
                        bytes_Read++;
                    }
                }
                else /*Not the currentPointer sector*/
                {
                    if( bytesToRead> 0 )
                    {
                        memcpy(buffer + j +offset+ (cPSector-1-i)*(SECTOR_SIZE), cpBuff + j, sizeof(BYTE));
                        bytesToRead--;
                        bytes_Read++;
                    }
                }
            }
        }
    }
    /*Now we read the remaining bytes in the other blocks*/
    for(i = 1; i < blocksToRead-1; i++)
    {
        cSector = blockFirstSector(arrayOfDataBlocks[cPointerBlock+i]);
        for(k = 0; k < superBlock.blockSize; k++) /*Reads all sectors in the block*/
        {
            if( read_sector(cSector + k, cpBuff) )
            {
                printf("ERROR reading sector read2\n");
                return -1;
            }
            for(j = 0; j < SECTOR_SIZE; j++)
            {
                if(bytesToRead > 0)
                {
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

int write2 (FILE2 handle, char *buffer, int size)
{
    if(handle < 0 || handle >= MAX_NUM_FILES)
    {
        printf("ERROR: invalid handle at write2\n");
        return -1;
    }

    fileHandler file;
    file = openedFiles[handle];

    if(file.type != TYPEVAL_REGULAR)
    {
        printf("ERROR: openedFile not valid at write2\n");
        return -1;
    }
    if(file.numInode < 0)
    {
        printf("ERROR: numInode is less than 0 at write2\n");
        return -1;
    }

    file.currentPointer = file.inode.bytesFileSize;
    BYTE buf[SECTOR_SIZE];
    int DWORDinSector = SECTOR_SIZE / sizeof(DWORD);
    int DWORDinBlock = bytesInBlock / sizeof(DWORD);
    int inodeInd = file.currentPointer / bytesInBlock;
    DWORD sing_block, sing_sector, sing_offset, doub_sector, doub_offset;
    DWORD writeBlock;
    int blockaux;

    if(inodeInd < 2)
    {
        // sem indirecao
        writeBlock = file.inode.dataPtr[inodeInd];
    }
    else if(inodeInd < (2 + DWORDinBlock))
    {
        // indirecao simples
        sing_sector = (inodeInd-2) / DWORDinSector + blockFirstSector(file.inode.singleIndPtr);

        if(read_sector(sing_sector, buf))
        {
            printf("ERROR: could not read sector at write2\n");
            return -1;
        }

        sing_offset = (inodeInd-2) % DWORDinSector; // DWORD number inside the sector
        memcpy(&writeBlock, (buf + (sing_offset * sizeof(DWORD))), sizeof(DWORD));
    }
    else
    {
        // indirecao dupla
        doub_sector = ((inodeInd - (2 + DWORDinBlock)) / (DWORDinSector * DWORDinBlock)) + blockFirstSector(file.inode.doubleIndPtr);
        if(read_sector(doub_sector, buf))
        {
            printf("ERROR: could not read sector at write2\n");
            return -1;
        }

        doub_offset = (inodeInd - (2 + DWORDinBlock)) % (DWORDinSector * DWORDinBlock);
        memcpy(&sing_block, (buf + (doub_offset * sizeof(DWORD))), sizeof(DWORD));

        blockaux = (inodeInd - (2 + DWORDinBlock)) / DWORDinBlock; // used to calculate the local inodeInd

        sing_sector = ((inodeInd - (2 + DWORDinBlock + (blockaux*DWORDinBlock))) / DWORDinSector) +blockFirstSector(sing_block);
        sing_offset = ((inodeInd - (2 + DWORDinBlock + (blockaux*DWORDinBlock))) % DWORDinSector);
        if(read_sector(sing_sector, buf))
        {
            printf("ERROR: could not read sector at write2\n");
            return -1;
        }
        memcpy(&writeBlock, (buf + (sing_offset * sizeof(DWORD))), sizeof(DWORD));
    }

    // usar o writeBlock para escrever no arquivo at� ele lotar e ir modificando o currentPointer
    int block_offset = file.currentPointer % bytesInBlock;
    int block_sector = block_offset / SECTOR_SIZE;
    int sector_offset = block_offset % SECTOR_SIZE;
    int writtenBytes = 0;

    if(block_offset > 0)  // there is free space in the block
    {
        if(read_sector(blockFirstSector(writeBlock) + block_sector, buf))
        {
            printf("ERROR: could not read sector at write2\n");
            return -1;
        }

        if((SECTOR_SIZE - sector_offset) < size)
            memcpy(buf+sector_offset, buffer, SECTOR_SIZE - sector_offset);
        else
            memcpy(buf+sector_offset, buffer, size);

        if(write_sector(blockFirstSector(writeBlock) + block_sector, buf))
        {
            printf("ERROR: could not write sector at write2\n");
            return -1;
        }

        if((SECTOR_SIZE - sector_offset) < size)
            writtenBytes = SECTOR_SIZE - sector_offset;
        else
            writtenBytes = size;

        file.currentPointer += writtenBytes;
    }
    while(writtenBytes < size)
    {
        // quando o writeBlock lotar, alocar mais um bloco usando o bitmap
        block_offset = file.currentPointer % bytesInBlock;
        if(block_offset == 0)  // the block is full
        {
            /* ALLOC ANOTHER BLOCK AND PUT IT IN THE writeBlock */
            writeBlock = searchBitmap2(BITMAP_DADOS, 0);
            if(writeBlock == 0)
            {
                printf("ERROR: could not find a free block at write2\n");
                return -1;
            }

            inodeInd++; // goes to the next index in the inode
            file.inode.blocksFileSize++;
            if(inodeInd < 2)
            {
                // sem indirecao
                file.inode.dataPtr[inodeInd] = writeBlock;
            }
            else if(inodeInd < (2 + DWORDinBlock))
            {
                // indirecao simples
                sing_sector = ((inodeInd-2) / DWORDinSector) + blockFirstSector(file.inode.singleIndPtr);

                if(read_sector(sing_sector, buf))
                {
                    printf("ERROR: could not read sector at write2\n");
                    return -1;
                }

                sing_offset = (inodeInd-2) % DWORDinSector; // DWORD number inside the sector
                memcpy((buf + (sing_offset * sizeof(DWORD))), &writeBlock, sizeof(DWORD));
                if(write_sector(sing_sector, buf))
                {
                    printf("ERROR: could not write sector at write2\n");
                    return -1;
                }
            }
            else
            {
                // indirecao dupla
                doub_sector = ((inodeInd - (2 + DWORDinBlock)) / (DWORDinSector * DWORDinBlock)) + blockFirstSector(file.inode.doubleIndPtr);
                doub_offset = (inodeInd - (2 + DWORDinBlock)) % (DWORDinSector * DWORDinBlock);

                if(read_sector(doub_sector, buf))
                {
                    printf("ERROR: could not read sector at write2\n");
                    return -1;
                }

                if(doub_offset == 0)  // need to alloc another sind block
                {
                    sing_block = searchBitmap2(BITMAP_DADOS, 0);
                    if(sing_block == 0)
                    {
                        printf("ERROR: could not find a free block at write2\n");
                        return -1;
                    }
                    memcpy((buf + (doub_offset * sizeof(DWORD))), &sing_block, sizeof(DWORD));
                    if(write_sector(doub_sector, buf))
                    {
                        printf("ERROR: could not write sector at write2\n");
                        return -1;
                    }
                    if(setBitmap2(BITMAP_DADOS, sing_block, 1))
                    {
                        printf("ERROR: could not set bitmap at write2\n");
                        return -1;
                    }
                }
                else
                {
                    memcpy(&sing_block, (buf + (doub_offset * sizeof(DWORD))), sizeof(DWORD));
                }
                blockaux = (inodeInd - (2 + DWORDinBlock)) / DWORDinBlock; // used to calculate the local inodeInd

                sing_sector = ((inodeInd - (2 + DWORDinBlock + (blockaux*DWORDinBlock))) / DWORDinSector) +blockFirstSector(sing_block);
                sing_offset = ((inodeInd - (2 + DWORDinBlock + (blockaux*DWORDinBlock))) % DWORDinSector);
                if(read_sector(sing_sector, buf))
                {
                    printf("ERROR: could not read sector at write2\n");
                    return -1;
                }
                memcpy((buf + (sing_offset * sizeof(DWORD))), &writeBlock, sizeof(DWORD));
                if(write_sector(sing_sector, buf))
                {
                    printf("ERROR: could not write sector at write2\n");
                    return -1;
                }
            }

            if(setBitmap2(BITMAP_DADOS, writeBlock, 1))
            {
                printf("ERROR: could not set bitmap at write2\n");
                return -1;
            }
        }

        block_sector = block_offset / SECTOR_SIZE;

        if(SECTOR_SIZE < size)
            memcpy(buf, buffer +writtenBytes, SECTOR_SIZE);
        else
            memcpy(buf, buffer +writtenBytes, size);

        if(write_sector(blockFirstSector(writeBlock) + block_sector, buf))
        {
            printf("ERROR: could not write sector at write2\n");
            return -1;
        }

        if((SECTOR_SIZE - sector_offset) < size)
            writtenBytes = SECTOR_SIZE - sector_offset;
        else
            writtenBytes = size;

        file.currentPointer += writtenBytes;
    }
    // salvar o file e o inode
    file.inode.bytesFileSize = file.currentPointer;
    if(setInode(file.inode, file.numInode))
    {
        printf("ERROR: could not set the inode at write2\n");
        return -1;
    }
    openedFiles[handle] = file;
    return size;
}

int truncate2 (FILE2 handle)
{
    fileHandler truncFile;

    if(handle < 0 || handle >=MAX_NUM_FILES)
    {
        printf("Error invalid handle in truncate2\n");
        return -1;
    }
    truncFile = openedFiles[handle];

    if(truncFile.type == TYPEVAL_INVALIDO)
    {
        printf("Tried to truncate an invalid file\n");
        return -1;
    }

    if(truncFile.type == TYPEVAL_DIRETORIO)
    {
        printf("Tried to truncate a directory in trunc2\n");
        return -1;
    }
    /*It is a real file, now we can truncate it*/

    if( delBlocks(truncFile.numInode, truncFile.currentPointer) )
    {
        printf("Error truncating blocks in trunc2\n");
        return -1;
    }

    truncFile.inode.blocksFileSize = (truncFile.currentPointer)/bytesInBlock;/*Remaining blocks*/
    if( (truncFile.currentPointer)%bytesInBlock != 0 )
        truncFile.inode.blocksFileSize++;
    truncFile.inode.bytesFileSize = truncFile.currentPointer;/*New bytes file size*/

    if( setInode(truncFile.inode, truncFile.numInode) )
    {
        printf("Error setting inode in truncate2\n");
        return -1;
    }
}

int seek2 (FILE2 handle, DWORD offset)
{
    if(handle < 0 || handle >=MAX_NUM_FILES)
    {
        printf("ERROR: handle not valid!\n");
        return -1;
    }

    if(offset > ((superBlock.blockSize*SECTOR_SIZE)*2)+PointersInBlock*((superBlock.blockSize*SECTOR_SIZE)+(PointersInBlock*(superBlock.blockSize*SECTOR_SIZE)))) 	//tamanho maximo de um arquivo no T2FS
    {
        printf("ERROR, offset is bigger than bytes supported in any file or directory! (function: seek2) \n");
        return -1;
    }

    if(offset == -1)
    {
        openedFiles[handle].currentPointer = openedFiles[handle].inode.bytesFileSize + 1;
    }
    else
    {
        openedFiles[handle].currentPointer = offset;
    }
    return 0;
}

int mkdir2 (char *pathname)
{
    struct coordinates info;

    info = getReg(pathname);
    if(info.record.TypeVal == TYPEVAL_INVALIDO)  //nome valido, nao utilizado ainda
    {
        struct t2fs_inode newFileInode;
        struct t2fs_record dir0;
        struct coordinates dir1;
        int inodeNum;
        struct t2fs_record newReg;
        char *token;
        char lastoken[59];
        char *auxpath = malloc(strlen(pathname)+1);

        //nome é só o ultimo argumento do pathname
        newReg.TypeVal = TYPEVAL_DIRETORIO;
        strcpy(auxpath, pathname);
        token = strtok(auxpath, "/");
        strcpy(lastoken, token);
        while(token!=NULL)
        {
            token = strtok(NULL, "/");
            strcpy(lastoken, token);
        }

        strcpy(newReg.name, lastoken);
        inodeNum = searchBitmap2(BITMAP_INODE, 0);
        newReg.inodeNumber = inodeNum;

        free(auxpath);
        if(setReg(newReg, pathname))
        {
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
    }
    else
    {
        printf("ERROR, there are other directory(s) with the same name in this directory! (function: create2)\n");
        return -1;
    }
}

int rmdir2 (char *pathname)
{
    struct coordinates info;
    struct t2fs_inode dirInode;
    DWORD sector;

    info = getReg(pathname);
    if(info.record.TypeVal == TYPEVAL_INVALIDO || info.record.TypeVal == TYPEVAL_REGULAR)
    {
        printf("ERROR, directory could not be found! (function: rmdir2)\n");
        return -1;
    }

    dirInode = getInode(info.record.inodeNumber);
    if(dirInode.bytesFileSize == -1)
    {
        printf("ERROR, inode directory could not be found! (function: rmdir2)\n");
        return -1;
    }
    if(dirInode.bytesFileSize > 2*sizeof(struct t2fs_record))
    {
        printf("ERROR, directory is not empty! (function: rmdir2)\n");
        return -1;
    }
    else
    {
        if ( delBlocks(info.record.inodeNumber, -1) )
        {
            printf("Error deleting directory blocks in rmdir2\n");
            return -1;
        }

        if( setBitmap2(BITMAP_INODE, info.record.inodeNumber,0))
        {
            printf("Error setting inode bitmap in rmdir2\n");
            return -1;
        }
    }

}

int getcwd2 (char *pathname, int size)
{

    if(size < strlen(currentDir.pathName)+1)
    {
        printf("ERROR, insufficient size! (function: getcwd2) \n");
        return -1;
    }
    else
    {
        strncpy(pathname, currentDir.pathName, size*sizeof(char));
        return 0;
    }
}

DIR2 opendir2 (char *pathname)
{
    struct coordinates reg;
    struct t2fs_inode inode;
    char *abspath;


    reg = getReg(pathname);

    if(reg.record.TypeVal == TYPEVAL_INVALIDO)
    {
        printf("ERROR: file not found!\n");
        return -1;
    }

    int i=0;
    //searches for the next free openedFiles
    while((i<MAX_NUM_FILES) && (openedFiles[i].type == TYPEVAL_INVALIDO))
        i++;

    if(i >= MAX_NUM_FILES)
    {
        printf("ERROR: openedFiles is full!\n");
        return -1;
    }

    if(absolutePath(abspath, pathname))
    {
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

int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
    if(handle < 0 || handle >=MAX_NUM_FILES)
    {
        printf("ERROR: handle not valid! (function: readdir2) \n");
        dentry->fileType = TYPEVAL_INVALIDO;
        return -1;
    }
    if(openedFiles[handle].type != TYPEVAL_DIRETORIO)
    {
        printf("ERROR, this handle does not reference a directory! (function: readdir2) \n");
        dentry->fileType = TYPEVAL_INVALIDO;
        return -1;
    }

    struct t2fs_record regs;
    struct t2fs_inode inodes;

    if(handle != info_dir.actual_dir_handle) 	//mudou o diretorio que sera lido, deve reiniciar todas variaveis
    {
        info_dir.dirBlocks=0;
        info_dir.rsector=0;
        info_dir.offsetDirBlock=0;
        info_dir.offsetSingleIndBlock=0;
        info_dir.offsetDoubleIndBlock=0;
        info_dir.t=0;
        info_dir.g=0;
        info_dir.h=0;
        info_dir.j=0;
        info_dir.z=0;
        info_dir.bytesRead=0;
        info_dir.pointer=0;
        info_dir.indpointer=0;
        info_dir.actual_dir_handle = handle;
    }
    if(info_dir.bytesRead >= (openedFiles[handle].inode.bytesFileSize))
    {
        printf("All valid records were read!\n");
        return -1;
    }

    regs = readRegs(openedFiles[handle].inode);

    dentry->fileType = regs.TypeVal;

    inodes = getInode(regs.inodeNumber);
    if(inodes.blocksFileSize == -1)
    {
        printf("ERROR, could not get inode! (function: readdir) \n");
        return -1;
    }
    dentry->fileSize = inodes.bytesFileSize;
    //copia o nome do registro lido para dentry

    strcpy(dentry->name, regs.name);
    return 0;
}

int closedir2 (DIR2 handle)
{
    if(handle < 0 || handle >=MAX_NUM_FILES)
    {
        printf("ERROR: handle not valid!\n");
        return -1;
    }

    fileHandler *file;
    file = &(openedFiles[handle]);

    if (file->type != TYPEVAL_DIRETORIO)
    {
        printf("ERROR: handle file not found!\n");
        return -1;
    }

    file->fullPathName[0] = '\0';
    file->type = TYPEVAL_INVALIDO;
    return 0;
}

/*************************************************************************
FUNCOES AUXILIARES
*************************************************************************/

int setup()
{
    BYTE buffer[SECTOR_SIZE];
    //Create root directory and prepare the file
    if( (read_sector(0, buffer)) )
    {
        printf("Could not read first sector\n");
        return -1;
    }

    strncpy(superBlock.id,(char*) buffer, 4); //ID
    if(strcmp(superBlock.id, "T2FS") != 0)
    {
        printf("Error, not T2FS\n");
        return -1;
    }
    char temp[4];
    temp[2] = '\0';
    strncpy(temp,(const char*)buffer+4, 2); //version
    superBlock.version = (WORD)atoi(temp);

    strncpy(temp,(const char*) buffer+6, 2);
    superBlock.blockSize = (WORD)atoi(temp);

    strncpy(temp,(const char*) buffer+8, 2);
    superBlock.freeBlocksBitmapSize =(WORD)atoi(temp);

    strncpy(temp, (const char*)buffer+10, 2);
    superBlock.freeInodeBitmapSize =(WORD)atoi(temp);

    strncpy(temp, (const char*)buffer+12, 2);
    superBlock.inodeAreaSize =(WORD)atoi(temp);

    strncpy(temp,(const char*) buffer+14, 2);
    superBlock.blockSize =(WORD)atoi(temp);

    strncpy(temp,(const char*) buffer+12, 2);
    superBlock.diskSize =(DWORD)atoi(temp);

    //Setting up global variables
    bytesInBlock = superBlock.blockSize * SECTOR_SIZE;
    firstDataBlock = (1 + superBlock.freeInodeBitmapSize + superBlock.freeBlocksBitmapSize + superBlock.inodeAreaSize);
    inodeSector = (1 + superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize)*superBlock.blockSize;
    PointersInBlock= bytesInBlock/sizeof(DWORD);                     //ponteiros indiretos em um bloco de dados;
    inode_per_sector = SECTOR_SIZE/sizeof(struct t2fs_inode); // denom era sizeof(DWORD)

    //Initialise the root directory
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

    if( setBitmap2(BITMAP_INODE, 0, 1))
        return -1; // FAILED

    if( setBitmap2(BITMAP_DADOS, 0, 1))
        return -1;

    //We must write in the "disk"
    memcpy(buffer, &rootRecord0, sizeof(rootRecord0));
    memcpy(buffer + sizeof(rootRecord0), &rootRecord1, sizeof(rootRecord0));
    write_sector( (firstDataBlock*SECTOR_SIZE), buffer);

    memcpy(buffer, &rootInode, sizeof(rootInode));
    write_sector(inodeSector, buffer);

    int i;
    for(i=0; i<MAX_NUM_FILES; i++)
        openedFiles[i].type = TYPEVAL_INVALIDO;

    systemReady = 1;
    return 0;
}

struct coordinates getReg(char *path)
{
    char *token;
    char *nameaux;
    struct t2fs_record dirReg;
    struct coordinates info;
    char buffer[SECTOR_SIZE];

    switch(filenameDir(path))
    {
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

    while(token != NULL)
    {
        info = getReginDir(token, info.record);
        token = strtok(NULL, "/");
    }
    free(nameaux);
    return info;
}

int setReg(struct t2fs_record reg, char *path)
{
    struct coordinates info;
    DWORD buffer[SECTOR_SIZE];

    //primeiro saber se o registro ja existe, ou se precisa ser criado
    info = getReg(path);
    if(info.record.TypeVal == TYPEVAL_INVALIDO) 	//registro nao existe, precisa ser criado no diretorio pai
    {
        char *token;
        char lastoken[59];
        char *nameaux = malloc(strlen(path)+1);

        strcpy(nameaux, path);
        token = strtok(nameaux, "/");

        while(token != NULL)
        {
            token = strtok(NULL, "/");
            strcpy(lastoken,token);
        }
        strncpy(nameaux, path, (strlen(path)-strlen(lastoken)-1));		//    /p1/p2/p5/p4/p6/texto
        nameaux[(strlen(path)-strlen(lastoken))] = '\0'; // strncpy doesn`t put a NULL in the end
        info = getReg(nameaux);
        info = getNewRegCoordinates(info.record);
        free(nameaux);
        if(info.bytesread==-1)
        {
            printf("ERROR creating new record, directory is already full (funtion : setReg) \n");
            return -1;
        }
        if(read_sector(info.lastReadSector, buffer))
        {
            printf("ERROR, reading sector (function: setReg) \n");
            return -1;
        }

        memcpy(buffer + (info.lastAcessedPosition * sizeof(struct t2fs_record)), &reg, sizeof(struct t2fs_record));
        if(write_sector(info.lastReadSector, buffer))
        {
            printf("ERROR, writing sector (function: setReg) \n");
            return -1;
        }
    }
    else
    {
        if(read_sector(info.lastReadSector, buffer))
        {
            printf("ERROR, reading sector (function: setReg) \n");
            return -1;
        }
        memcpy(buffer + (info.lastAcessedPosition * sizeof(struct t2fs_record)), &reg, sizeof(struct t2fs_record));
        if(write_sector(info.lastReadSector, buffer))
        {
            printf("ERROR, writing sector (function: setReg) \n");
            return -1;
        }
    }
    return 0;
}

struct coordinates getReginDir(char *name, struct t2fs_record dirReg)
{
    int dirBlocks=0, offsetDirBlock=0, offsetSingleIndBlock=0, t=0, offsetDoubleIndBlock=0, h=0, g=0, j=0, z=0;
    DWORD rsector;
    DWORD pointer=0, indpointer=0;
    BYTE sindbuffer[SECTOR_SIZE];
    BYTE dindbuffer[SECTOR_SIZE];
    int bytesread=0;  //contabiliza o total de bytes que ja foram lidos
    struct t2fs_record regAux;
    struct t2fs_inode inodeAux;
    struct coordinates info;
    char buffer[SECTOR_SIZE];


    if(dirReg.TypeVal==TYPEVAL_DIRETORIO)
    {
        if(strcmp(name, ".") == 0)  // retorna o mesmo diretorio
        {
            info.record = dirReg;
            return info;
        }
        else if(strcmp(name, "..") == 0)    // retorna o diretorio pai
        {
            inodeAux = getInode(dirReg.inodeNumber);
            if(inodeAux.bytesFileSize == -1)
            {
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
        if(inodeAux.bytesFileSize == -1)
        {
            regAux.TypeVal == TYPEVAL_INVALIDO;
            info.record = regAux;
            return info;
        }
        for(dirBlocks=0; dirBlocks < inodeAux.blocksFileSize; dirBlocks++)  //percorre blocos
        {
            if(dirBlocks==0)
                rsector = (inodeAux.dataPtr[0] * superBlock.blockSize);
            else if(dirBlocks==1)
                rsector = (inodeAux.dataPtr[1] * superBlock.blockSize);
            else if(dirBlocks<(PointersInBlock+2))  //INDIRECAO SIMPLES
            {
                if(offsetSingleIndBlock < superBlock.blockSize)
                {
                    if(t==(SECTOR_SIZE/sizeof(DWORD)) || offsetSingleIndBlock == 0)
                    {
                        read_sector((blockFirstSector(inodeAux.singleIndPtr)) + offsetSingleIndBlock, sindbuffer);
                        offsetSingleIndBlock++;
                        t=0;
                    }
                }

                if(t < (SECTOR_SIZE/sizeof(DWORD)))
                {
                    memcpy(&pointer, sindbuffer+(t*sizeof(DWORD)), sizeof(DWORD));
                    rsector = (blockFirstSector(pointer));
                    t++;
                }
            }
            else   //INDIRECAO DUPLA
            {
                if(offsetDoubleIndBlock < superBlock.blockSize)
                {
                    if(h==SECTOR_SIZE/sizeof(DWORD) || offsetDoubleIndBlock == 0)
                    {
                        read_sector((blockFirstSector(inodeAux.doubleIndPtr))+offsetDoubleIndBlock, dindbuffer);
                        offsetDoubleIndBlock++;
                        h=0;
                    }
                }

                if((((blockFirstSector(indpointer)+(superBlock.blockSize)) == (j+blockFirstSector(indpointer))) && g==(SECTOR_SIZE/sizeof(DWORD))) || h==0)
                {
                    memcpy(&indpointer, dindbuffer + (h*sizeof(DWORD)), sizeof(DWORD));
                    h++;
                    j=0;
                }

                if(j<superBlock.blockSize)
                {
                    if(g==(SECTOR_SIZE/sizeof(DWORD)) || j == 0)
                    {
                        read_sector((blockFirstSector(indpointer)) + j, sindbuffer);
                        j++;
                        g=0;
                    }
                }

                if(g<(SECTOR_SIZE/sizeof(DWORD)))
                {
                    memcpy(&pointer, sindbuffer+(g*sizeof(DWORD)), sizeof(DWORD));
                    rsector = (blockFirstSector(pointer));
                    g++;
                }
            }

            for(offsetDirBlock=0; offsetDirBlock<superBlock.blockSize; offsetDirBlock++)  //setores dentro do bloco
            {
                read_sector(rsector + offsetDirBlock, buffer);
                z=0;
                do
                {
                    memcpy(&regAux, buffer+(z*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
                    if(regAux.TypeVal == TYPEVAL_REGULAR || regAux.TypeVal == TYPEVAL_DIRETORIO)
                    {
                        bytesread += sizeof(struct t2fs_record);
                        if(strcmp(regAux.name, name)==0)
                        {
                            info.lastReadSector = rsector+offsetDirBlock;
                            info.lastAcessedPosition = z;
                            info.bytesread = bytesread;
                            info.record = regAux;
                            return info;
                        }
                    }
                    z++;
                }
                while(z<(SECTOR_SIZE/sizeof(struct t2fs_record)) && bytesread<inodeAux.bytesFileSize);
                if(bytesread >=inodeAux.bytesFileSize)
                {
                    info.record.TypeVal = TYPEVAL_INVALIDO;
                    return info;
                }
            }
        }
    }
    info.record.TypeVal = TYPEVAL_INVALIDO;
    return info;
}

struct coordinates getNewRegCoordinates(struct t2fs_record dirReg)
{
    char buffer[SECTOR_SIZE];
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

    if(dirReg.TypeVal==TYPEVAL_DIRETORIO)
    {
        inodeAux = getInode(dirReg.inodeNumber);
        if(inodeAux.bytesFileSize == -1)
        {
            info.record.TypeVal == TYPEVAL_INVALIDO;
            return info;
        }

        if(inodeAux.bytesFileSize==(inodeAux.blocksFileSize * superBlock.blockSize * SECTOR_SIZE)) 	//ARQUIVO CHEIO
        {
            info.bytesread = -1;
            return info;
        }
        /*
        char pathaux[MAX_FILE_NAME_SIZE];
        strcpy(pathaux, "./");
        strcat(pathaux, dirReg.name);
        newReg = getReg(pathaux);
        ARTHUR TA TROLLANDO
        */
        if(((inodeAux.bytesFileSize)%(inodeAux.blocksFileSize * superBlock.blockSize * SECTOR_SIZE))==0)	//checa se o final dos dados coincide com o final de algum bloco
            plus = 1;

        for(dirBlocks=0; dirBlocks < (inodeAux.blocksFileSize+plus); dirBlocks++)  //percorre blocos
        {
            if(dirBlocks==0)
                rsector = (inodeAux.dataPtr[0] * superBlock.blockSize);
            else if(dirBlocks==1)
                rsector = (inodeAux.dataPtr[1] * superBlock.blockSize);
            else if(dirBlocks<(PointersInBlock+2))  //INDIRECAO SIMPLES
            {
                if(offsetSingleIndBlock < superBlock.blockSize)
                {
                    if(t==(SECTOR_SIZE/sizeof(DWORD)) || offsetSingleIndBlock == 0)
                    {
                        read_sector((blockFirstSector(inodeAux.singleIndPtr)) + offsetSingleIndBlock, sindbuffer);
                        offsetSingleIndBlock++;
                        t=0;
                    }
                }

                if(t < (SECTOR_SIZE/sizeof(DWORD)))
                {
                    memcpy(&pointer, sindbuffer+(t*sizeof(DWORD)), sizeof(DWORD));
                    rsector = (blockFirstSector(pointer));
                    t++;
                }
            }
            else   //INDIRECAO DUPLA
            {
                if(offsetDoubleIndBlock < superBlock.blockSize)
                {
                    if(h==SECTOR_SIZE/sizeof(DWORD) || offsetDoubleIndBlock == 0)
                    {
                        read_sector((blockFirstSector(inodeAux.doubleIndPtr))+offsetDoubleIndBlock, dindbuffer);
                        offsetDoubleIndBlock++;
                        h=0;
                    }
                }

                if((((blockFirstSector(indpointer)+(superBlock.blockSize)) == (j+blockFirstSector(indpointer))) && g==(SECTOR_SIZE/sizeof(DWORD))) || h==0)
                {
                    memcpy(&indpointer, dindbuffer + (h*sizeof(DWORD)), sizeof(DWORD));
                    h++;
                    j=0;
                }

                if(j<superBlock.blockSize)
                {
                    if(g==(SECTOR_SIZE/sizeof(DWORD)) || j == 0)
                    {
                        read_sector((blockFirstSector(indpointer)) + j, sindbuffer);
                        j++;
                        g=0;
                    }
                }

                if(g<(SECTOR_SIZE/sizeof(DWORD)))
                {
                    memcpy(&pointer, sindbuffer+(g*sizeof(DWORD)), sizeof(DWORD));
                    rsector = (blockFirstSector(pointer));
                    g++;
                }
            }

            for(offsetDirBlock=0; offsetDirBlock<superBlock.blockSize; offsetDirBlock++)  //setores dentro do bloco
            {
                read_sector(rsector + offsetDirBlock, buffer);
                z=0;
                do
                {
                    memcpy(&regAux, buffer+(z*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
                    if(regAux.TypeVal != TYPEVAL_INVALIDO)
                        bytesread += sizeof(struct t2fs_record);
                    if(regAux.TypeVal == TYPEVAL_INVALIDO)
                    {
                        info.lastReadSector = rsector+offsetDirBlock;
                        info.lastAcessedPosition = z;
                        info.bytesread = bytesread;
                        info.record = regAux;
                        return info;
                    }
                    else if(bytesread>inodeAux.bytesFileSize) 		//se nao houver registros invalidos retornara as "coordenadas apontando" para uma posicao após o ultimo registro valido
                    {
                        info.lastReadSector = rsector+offsetDirBlock;
                        info.lastAcessedPosition = z;
                        info.bytesread = bytesread;
                        return info;
                    }
                    z++;
                }
                while(z<(SECTOR_SIZE/sizeof(struct t2fs_record)));
            }
        }
    }
}

struct t2fs_inode getInode(DWORD inodeNumber)
{
    int blockOffset = (int) (inodeNumber/inode_per_sector);
    int sectorOffset = (int) (inodeNumber % inode_per_sector);
    struct t2fs_inode inode;
    char buffer[SECTOR_SIZE];

    if(read_sector(inodeSector + blockOffset, buffer))
    {
        printf("read_sector ERROR! (Function: getInode) \n");
        inode.bytesFileSize = -1;
        return inode;
    }
    memcpy(&inode, buffer + sectorOffset, sizeof(struct t2fs_inode));

    return inode;
}

struct t2fs_record readRegs(struct t2fs_inode inode)
{
    struct t2fs_record regAux;


    for(; info_dir.dirBlocks < inode.blocksFileSize; info_dir.dirBlocks++)  //percorre blocos
    {
        if(info_dir.dirBlocks==0)
            info_dir.rsector = (inode.dataPtr[0] * superBlock.blockSize);
        else if(info_dir.dirBlocks==1)
            info_dir.rsector = (inode.dataPtr[1] * superBlock.blockSize);
        else if(info_dir.dirBlocks<(PointersInBlock+2))  //INDIRECAO SIMPLES
        {
            if(info_dir.offsetSingleIndBlock < superBlock.blockSize)
            {
                if(info_dir.t==(SECTOR_SIZE/sizeof(DWORD)) || info_dir.offsetSingleIndBlock == 0)
                {
                    read_sector((blockFirstSector(inode.singleIndPtr)) + info_dir.offsetSingleIndBlock, info_dir.sindbuffer);
                    info_dir.offsetSingleIndBlock++;
                    info_dir.t=0;
                }
            }

            if(info_dir.t < (SECTOR_SIZE/sizeof(DWORD)))
            {
                memcpy(&info_dir.pointer, info_dir.sindbuffer+(info_dir.t*sizeof(DWORD)), sizeof(DWORD));
                info_dir.rsector = (blockFirstSector(info_dir.pointer));
                info_dir.t++;
            }
        }
        else   //INDIRECAO DUPLA
        {
            if(info_dir.offsetDoubleIndBlock < superBlock.blockSize)
            {
                if(info_dir.h==SECTOR_SIZE/sizeof(DWORD) || info_dir.offsetDoubleIndBlock == 0)
                {
                    read_sector((blockFirstSector(inode.doubleIndPtr))+info_dir.offsetDoubleIndBlock, info_dir.dindbuffer);
                    info_dir.offsetDoubleIndBlock++;
                    info_dir.h=0;
                }
            }

            if((((blockFirstSector(info_dir.indpointer)+(superBlock.blockSize)) == (info_dir.j+blockFirstSector(info_dir.indpointer))) && info_dir.g==(SECTOR_SIZE/sizeof(DWORD))) || info_dir.h==0)
            {
                memcpy(&info_dir.indpointer, info_dir.dindbuffer + (info_dir.h*sizeof(DWORD)), sizeof(DWORD));
                info_dir.h++;
                info_dir.j=0;
            }

            if(info_dir.j<superBlock.blockSize)
            {
                if(info_dir.g==(SECTOR_SIZE/sizeof(DWORD)) || info_dir.j == 0)
                {
                    read_sector((blockFirstSector(info_dir.indpointer)) + info_dir.j, info_dir.sindbuffer);
                    info_dir.j++;
                    info_dir.g=0;
                }
            }

            if(info_dir.g<(SECTOR_SIZE/sizeof(DWORD)))
            {
                memcpy(&info_dir.pointer, info_dir.sindbuffer+(info_dir.g*sizeof(DWORD)), sizeof(DWORD));
                info_dir.rsector = (blockFirstSector(info_dir.pointer));
                info_dir.g++;
            }
        }

        for(; info_dir.offsetDirBlock<superBlock.blockSize; info_dir.offsetDirBlock++)  //setores dentro do bloco
        {
            read_sector(info_dir.rsector + info_dir.offsetDirBlock, info_dir.buffer);
            info_dir.z=0;
            do
            {
                memcpy(&regAux, info_dir.buffer+(info_dir.z*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
                if(regAux.TypeVal == TYPEVAL_REGULAR || regAux.TypeVal == TYPEVAL_DIRETORIO)
                {
                    info_dir.bytesRead += sizeof(struct t2fs_record);
                    return regAux;
                }
                info_dir.z++;
            }
            while(info_dir.z<(SECTOR_SIZE/sizeof(struct t2fs_record)) && info_dir.bytesRead<inode.bytesFileSize);
            if(info_dir.bytesRead >=inode.bytesFileSize)
            {
                regAux.TypeVal = TYPEVAL_INVALIDO;
                return regAux;
            }
        }
    }
}

int setInode(struct t2fs_inode inode, DWORD inodeNumber)
{
    DWORD iSector, iPosition;
    iSector = inodeSector + (inodeNumber/inode_per_sector);
    iPosition = inodeNumber % inode_per_sector;

    unsigned char iBuf[SECTOR_SIZE];
    if(getBitMap2(BITMAP_INODE, inodeNumber)==0)  //checa se o inode a ser modificado, "ja existe/esta ocupado"
    {
        if(setBitMap2(BITMAP_INODE, inodeNumber, 1))
        {
            printf("ERROR, trying to update inode bitmap! (Function: modifyInode) \n");
            return -1;
        }
    }

    if(read_sector(iSector, iBuf) )
    {
        printf("ERROR reading sector: %i in modifyInode \n", iSector);
        return -1;
    }
    /*  Now iBuf has our iNode and others as well, we must modify only the one we want */
    /*  e.g inode number 14, if 3 per sector 14/3 = 4; 14%3 = 1 so sector 4, struct 1*/
    memcpy(iBuf + (iPosition*sizeof(struct t2fs_inode)), &inode, sizeof(struct t2fs_inode));
    if(write_sector(iSector, iBuf) )
    {
        printf("ERROR writing sector %i in modifyInode", iSector);
        return -1;
    }
    return 0;
}

int absolutePath(char *abspath, char *path)
{
    char fullpathname[MAX_FILE_NAME_SIZE +1];
    char name[6][59], *pathaux, *token, *current;
    int cont = 0;
    pathaux = (char *) malloc(strlen(path) +1);
    strcpy(pathaux, path);
    switch(filenameDir(pathaux))
    {
    case 0: //diretorio pai
    case 1: // diretorio atual
        // separa o diretorio corrente em tokens e os passa para name
        current = (char *) malloc(strlen(currentDir.pathName) +1);
        strcpy(current, currentDir.pathName);
        token = strtok(current, "/");
        while(token != NULL)
        {
            strcpy(name[cont], token); // passa o token pra uma string do nome
            cont++;
            token = strtok(NULL, "/");
        }
        break;
    case 2: // root
        // faz nada, pois o root começa vazio
        break;
    }
    free(current);

    token = strtok(pathaux, "/");
    while(token != NULL)
    {
        if(strcmp(token, ".") == 0)
        {
            // se é o diretorio atual, faz nada
        }
        else if(strcmp(token, "..") == 0)
        {
            // diretorio pai, apaga o ultimo name salvo
            if(cont > 0)
            {
                cont--;
            }
        }
        else
        {
            strcpy(name[cont], token); // passa o token pra uma string do nome
            cont++;
            if(cont >= 6)
            {
                printf("ERROR: too many directories in path!\n");
                return 1;
            }
        }
        token = strtok(NULL, "/");
    }
    free(pathaux);
    //concatena todo o caminho em fullpathname
    int i;
    fullpathname[0] = '\0';
    for(i=0; i<cont; i++)
    {
        strcat(fullpathname, "/");
        strcat(fullpathname, name[i]);
    }
    if(strlen(fullpathname) == 0)
    {
        strcpy(fullpathname, "/");
    }

    strcpy(abspath, fullpathname);
    return 0;
}

int filenameDir(char *filename)
{
    if(filename[0]=='/')
    {
        return 2;
    }
    else if(filename[0]=='.' && filename[1]=='.' && filename[2]=='/')
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

unsigned int blockFirstSector(unsigned int block)
{
    return (block * superBlock.blockSize);
}

int delBlocks(DWORD inode, int cPointer)
{

    //read inode
    struct t2fs_inode delNode= getInode(inode);
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
    if(blocksRemaining > (2 + PointersInBlock))  /* double Indirection */
    {

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
        for(i = 0; i < indexSectorsToRead; i++)
        {
            /*block of index in each sector*/
            if( read_sector(dfirstBlockSector + i, &delBuff) )
            {
                printf("Error in delBlocks,failed to read sector %i\n", dfirstBlockSector+i);
                return -1;
            }
            for(j = 0; j<iBSector; j++)
            {
                if(k < bIndex)
                {
                    memcpy(&arrayOfBlockIndex[k], delBuff+(sizeof(DWORD)*j), sizeof(DWORD));/*This is the block of indexes*/
                    k++;
                }
            }
        }
        /* We have all the blocks of index */
        l = 0;
        for( i = 0; i < bIndex; i++) /*Each index block*/
        {
            sFirstBlockSector = blockFirstSector(arrayOfBlockIndex[i]);/*First Block of the single index block*/
            for(j = 0; j < superBlock.blockSize; j++)
            {
                if( read_sector(sFirstBlockSector + j, &delBuff) )
                {
                    printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
                    return -1;
                }
                for(k = 0; k <iBSector; k++)
                {
                    if(l < dBlocksToDel)
                    {
                        memcpy(&arrayOfDataBlocks[l], delBuff + (k*sizeof(DWORD)), sizeof(DWORD));
                        l++;
                    }
                }
            }
        }

        if(cPointer == -1 || cPointer == 0) /*Now we have to free all the blocks*/
        {
            for(i = 0; i< bIndex; i++)
            {
                if ( setBitmap2(BITMAP_DADOS,arrayOfBlockIndex[i], 0) )
                {
                    printf("ERROR setting bits in delBlocks\n");
                    return -1;
                }
            }

            for(i = 0; i< dBlocksToDel; i++)
            {
                if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) )
                {
                    printf("ERROR setting bits in delBlocks\n");
                    return -1;
                }
            }
            if ( setBitmap2(BITMAP_DADOS,delNode.doubleIndPtr, 0) )
            {
                printf("ERROR setting bits in delBlocks\n");
                return -1;
            }
            blocksRemaining -= dBlocksToDel;
        }
        else /*Truncate*/
        {
            /*We must delete from the last to first entry in the arrayOfDataBlocks*/
            for(i = (dBlocksToDel-1); i >= 0; i--)  /*Delete the last blocks*/
            {
                if(trunc_blocksToDel > 0)
                {
                    if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) )
                    {
                        printf("ERROR setting bits in delBlocks\n");
                        return -1;
                    }
                    trunc_blocksToDel--;
                }
            }

            /*pB = 4; A(0,1,2,3)B(4,5,6,7)C(8,9,x,11) */
            k=0;
            DWORD trunc_matrix[bIndex][PointersInBlock];
            for(i = 0; i< bIndex; i++)
            {
                for(j = 0; j <PointersInBlock; j++)
                {
                    if(k<dBlocksToDel)
                    {
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
            for(i = 0; i< bIndex; i++)
            {
                for(j = 0; j <PointersInBlock; j++)
                {
                    if(k<dBlocksToDel)
                    {
                        if( getBitmap2(BITMAP_DADOS, trunc_matrix[i][j]) == 0) /*Deleted block*/
                        {
                            if(fInd == -1)
                            {
                                fPos = j;
                                fInd = i;
                            }
                        }
                        k++;
                    }
                }
            }
            if( fPos == 0)
            {
                trunc_indexToDel = bIndex-fInd;
            }
            else
            {
                trunc_indexToDel = bIndex- fInd - 1;
            }
            k = trunc_indexToDel;

            for(i = (bIndex -1) ; i>=0; i--)
            {
                if(trunc_indexToDel > 0)
                {
                    if ( setBitmap2(BITMAP_DADOS,arrayOfBlockIndex[i], 0) )
                    {
                        printf("ERROR setting bits in delBlocks\n");
                        return -1;
                    }
                    trunc_indexToDel--;
                }
            }
            if(k == bIndex) //We had to delete all the double indirection blocks
            {
                if ( setBitmap2(BITMAP_DADOS,delNode.doubleIndPtr, 0) )
                {
                    printf("ERROR setting bits in delBlocks\n");
                    return -1;
                }
            }
            blocksRemaining -= dBlocksToDel;

            //end of else truncate
        }
    }

    /*Single indirection*/
    if(blocksRemaining > 2)
    {

        dBlocksToDel = blocksRemaining - 2;
        sFirstBlockSector = blockFirstSector(delNode.singleIndPtr);/*First Block of the single index block*/

        indexSectorsToRead = dBlocksToDel /iBSector;//
        if( (bIndex % iBSector) != 0 )
            indexSectorsToRead++;

        l = 0;
        for(j = 0; j < indexSectorsToRead; j++)
        {
            if( read_sector(sFirstBlockSector + j, &delBuff) )
            {
                printf("Error in delBlocks,failed to read sector %i\n", sFirstBlockSector+j);
                return -1;
            }
            for(k = 0; k <iBSector; k++)
            {
                if(l < dBlocksToDel)
                {
                    memcpy(&arrayOfDataBlocks[l], delBuff + (k*sizeof(DWORD)), sizeof(DWORD));
                    l++;
                }
            }
        }

        /*Release the blocks*/
        if(cPointer == -1 || cPointer == 0)
        {
            if ( setBitmap2(BITMAP_DADOS,delNode.singleIndPtr, 0) )
            {
                printf("ERROR setting bits in delBlocks\n");
                return -1;
            }

            for(i = 0; i< dBlocksToDel; i++)
            {
                if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) )
                {
                    printf("ERROR setting bits in delBlocks\n");
                    return -1;
                }
            }

            blocksRemaining -= dBlocksToDel;
            //end of delete all
        }


        else  /*TRUNCATE*/
        {

            for(i = (dBlocksToDel-1); i >= 0; i--)  /*Delete the last blocks*/
            {
                if(trunc_blocksToDel > 0)
                {
                    if ( setBitmap2(BITMAP_DADOS,arrayOfDataBlocks[i], 0) )
                    {
                        printf("ERROR setting bits in delBlocks\n");
                        return -1;
                    }
                    trunc_blocksToDel--;
                }
            }
            if(trunc_blocksToDel > 0) /*The currentPointer is in the direct blocks*/
            {
                if ( setBitmap2(BITMAP_DADOS,delNode.singleIndPtr, 0) )
                {
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
    if(cPointer == -1 || cPointer == 0) /*DELETE ALL*/
    {
        if( setBitmap2(BITMAP_DADOS, delNode.dataPtr[0], 0) )
        {
            printf("ERROR setting bits in delBlocks\n");
            return -1;
        }
        if( setBitmap2(BITMAP_DADOS, delNode.dataPtr[1], 0) )
        {
            printf("ERROR setting bits in delBlocks\n");
            return -1;
        }
    }
    else if(trunc_blocksToDel != 0) /*TRUNCATE*/
    {

        for(i = 1; i >= 0; i--)  /*Delete the last blocks*/
        {
            if(trunc_blocksToDel > 0)
            {
                if ( setBitmap2(BITMAP_DADOS,delNode.dataPtr[i], 0) )
                {
                    printf("ERROR setting bits in delBlocks\n");
                    return -1;
                }
                trunc_blocksToDel--;
            }
        }
    }

    return 0;

}

int isFileOpen(DWORD inode)
{
    fileHandler temp;
    int i;
    for(i = 0; i < MAX_NUM_FILES; i++)
    {
        temp = openedFiles[i];
        if( temp.numInode == inode)
            return 1;
    }
    return 0;
}
