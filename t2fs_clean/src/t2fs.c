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

#define MAX_PATH_NAME_SIZE 301


typedef struct files{
    char fullPathName[MAX_FILE_NAME_SIZE + 1];
    BYTE type;
    int currentPointer; // bytes
    struct t2fs_inode inode;
    DWORD numInode;
} fileHandler;

typedef struct cD{
    char pathName[MAX_FILE_NAME_SIZE + 1];
    int block;
} dirDescription;


void printSB();
void dumpDisco();
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
struct t2fs_record getReg(char *path);

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
struct t2fs_record getReginDir(char *name, struct t2fs_record dirReg); // antigo getRegByName

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


/* GLOBAL VARIABLES */
dirDescription currentDir;
int bytesInBlock;
int firstDataBlock;
int inodeSector;
int inode_per_sector;
DWORD PointersInBlock;
fileHandler openedFiles[MAX_NUM_FILES];
struct t2fs_superbloco superBlock;
int systemReady = 0;

/* MAIN FUNCTIONS */

int identify2(char *name, int size){
    char student[MAX_STU_CHAR] = "";
    int i;
    int st2 = strlen(STU1);
    int st3 = st2 + strlen(STU2);
    int letters;
    strcat(student, STU1);
    strcat(student, STU2);
    strcat(student, STU3);
    if (size >= MAX_STU_CHAR) //Size is equal to or greater than students names
    {
        for (i = 0; i < MAX_STU_CHAR; i++)
            name[i] = student[i];
        return 0;
    }
    else if (size >= 9) //At least 1 characters for each
    {
        letters = (int)size / 3 - 2;
        for (i = 0; i < letters; i++)
        {
            name[i] = student[i];
            name[i + letters + 1] = student[i + st2];
            name[i + 2 * (letters + 1)] = student[i + st3];
        }
        name[letters] = '\n';
        name[2 * (letters) + 1] = '\n';
        name[i + 2 * (letters + 1) + 1] = '\0';
    }
    return -1;
}

FILE2 open2(char *filename){
    if (!systemReady){
        if (setup() == -1)
            return -1;
    }

    struct t2fs_record reg;
    struct t2fs_inode inode;
    char abspath[MAX_PATH_NAME_SIZE];

    reg = getReg(filename);

    if (reg.TypeVal == TYPEVAL_INVALIDO){
        printf("ERROR: file not found!\n");
        return -1;
    }

    int i = 0;
    //searches for the next free openedFiles
    while ((i < MAX_NUM_FILES) && (openedFiles[i].type != TYPEVAL_INVALIDO))
        i++;

    if (i >= MAX_NUM_FILES){
        printf("ERROR: There are already 10 files opened!\n");
        return -1;
    }

    if (absolutePath(abspath, filename)){
        printf("ERROR, could not convert filename to absolutePath! (function: open2) \n");
        return -1;
    }
    int j=0;
    for(j=0; j<10; j++){
        if(strcmp(openedFiles[j].fullPathName, abspath)==0){
            printf("This file is already open\n");
            return j;
        }
    }
    
    strcpy(openedFiles[i].fullPathName, abspath);
    openedFiles[i].type = TYPEVAL_REGULAR;
    openedFiles[i].currentPointer = 0;
    inode = getInode(reg.inodeNumber);
    openedFiles[i].inode = inode;
    openedFiles[i].numInode = reg.inodeNumber;

    return i;
}

int read2(FILE2 handle, char *buffer, int size){
    if (handle < 0 || handle >= MAX_NUM_FILES){
        printf("ERROR: handle not valid!\n");
        return -1;
    }

    if(size>SECTOR_SIZE){
        printf("ERROR: size is bigger than buffer size\n");
        return -1;
    }

    if(openedFiles[handle].type!=TYPEVAL_REGULAR){
        printf("ERROR: File not opened or not directory type\n");
        return -1;
    }

    struct t2fs_inode fileInode = openedFiles[handle].inode;
    
    if (fileInode.bytesFileSize == -1){
        printf("ERROR: empty file\n");
        return 0;
    }

    int dirBlocks = 0, offsetDirBlock = 0, offsetSingleIndBlock = 0, t = 0, offsetDoubleIndBlock = 0, h = 0, g = 0, j = 0, z = 0;
    DWORD rsector;
    DWORD pointer = 0, indpointer = 0;
    BYTE sindbuffer[SECTOR_SIZE];
    BYTE dindbuffer[SECTOR_SIZE];
    int bytesread = 0; //contabiliza o total de bytes que ja foram lidos
    BYTE prebuffer[SECTOR_SIZE];

    dirBlocks = openedFiles[handle].currentPointer/(superBlock.blockSize*SECTOR_SIZE);
    rsector = openedFiles[handle].currentPointer%(superBlock.blockSize*SECTOR_SIZE);

    for (dirBlocks = 0; dirBlocks < fileInode.blocksFileSize; dirBlocks++){ //percorre blocos
        if (dirBlocks == 0)
            rsector = blockFirstSector(fileInode.dataPtr[0]);
        else if (dirBlocks == 1)
            rsector = blockFirstSector(fileInode.dataPtr[0]);
        else if (dirBlocks < (PointersInBlock + 2)){ //INDIRECAO SIMPLES
        
            if (offsetSingleIndBlock < superBlock.blockSize){
            
                if (t == (SECTOR_SIZE / sizeof(DWORD)) || offsetSingleIndBlock == 0){
                
                    read_sector((blockFirstSector(fileInode.singleIndPtr)) + offsetSingleIndBlock, sindbuffer);
                    offsetSingleIndBlock++;
                    t = 0;
                }
            }

            if (t < (SECTOR_SIZE / sizeof(DWORD))){
            
                memcpy(&pointer, sindbuffer + (t * sizeof(DWORD)), sizeof(DWORD));
                rsector = (blockFirstSector(pointer));
                t++;
            }
        }
        else{ //INDIRECAO DUPLA
        
            if (offsetDoubleIndBlock < superBlock.blockSize)
            {
                if (h == SECTOR_SIZE / sizeof(DWORD) || offsetDoubleIndBlock == 0)
                {
                    read_sector((blockFirstSector(fileInode.doubleIndPtr)) + offsetDoubleIndBlock, dindbuffer);
                    offsetDoubleIndBlock++;
                    h = 0;
                }
            }

            if ((((blockFirstSector(indpointer) + (superBlock.blockSize)) == (j + blockFirstSector(indpointer))) && g == (SECTOR_SIZE / sizeof(DWORD))) || h == 0)
            {
                memcpy(&indpointer, dindbuffer + (h * sizeof(DWORD)), sizeof(DWORD));
                h++;
                j = 0;
            }

            if (j < superBlock.blockSize)
            {
                if (g == (SECTOR_SIZE / sizeof(DWORD)) || j == 0)
                {
                    read_sector((blockFirstSector(indpointer)) + j, sindbuffer);
                    j++;
                    g = 0;
                }
            }

            if (g < (SECTOR_SIZE / sizeof(DWORD)))
            {
                memcpy(&pointer, sindbuffer + (g * sizeof(DWORD)), sizeof(DWORD));
                rsector = (blockFirstSector(pointer));
                g++;
            }
        }

        for (offsetDirBlock = 0; offsetDirBlock < superBlock.blockSize; offsetDirBlock++){ //setores dentro do bloco
            read_sector(rsector + offsetDirBlock, prebuffer);
            z = 0;
            do{
            
                memcpy(buffer, prebuffer + z, sizeof(char));
                    bytesread += sizeof(struct t2fs_record);
                z++;
            } while (z < (SECTOR_SIZE) && bytesread < size);
        }
    }
}

int close2(FILE2 handle){
    if (handle < 0 || handle >= MAX_NUM_FILES){
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


//FUNCOES NAO IMPLEMENTADAS
FILE2 create2(char * filename){
    return -1;
}
int delete2(char *filename){
    return -1;
}
int write2(FILE2 hanle, char *buffer, int size){
    return -1;
}
int truncate2(FILE2 handle){
    return -1;
}
int seek2 (FILE2 handle, DWORD offset){
    return -1;
}
int mkdir2(char *pathname){
    return -1;
}
int rmdir2(char *pathname){
    return -1;
}
int chdir2(char *pathname){
    return -1;
}
int getcwd2(char *pathname, int size){
    return -1;
}
DIR2 opendir2(char *pathname){
    return -1;
}
int readdir2(DIR2 handle, DIRENT2 *dentry){
    return -1;
}
int closedir2(DIR2 handle){
    return -1;
}

/*************************************************************************
FUNCOES AUXILIARES
*************************************************************************/

int setup(){
    BYTE buffer[SECTOR_SIZE];
    WORD *word_aux;
    DWORD *dword_aux;
    //Create root directory and prepare the file
    if (read_sector(0, buffer)){
        printf("Could not read first sector\n");
        return -1;
    }
    
    strncpy(superBlock.id, (char *)buffer, 4); //ID
    word_aux = (WORD *)(buffer + 4);
    superBlock.version = *word_aux;
    word_aux = (WORD *)(buffer + 6);
    superBlock.superblockSize = *word_aux;
    word_aux = (WORD *)(buffer + 8);
    superBlock.freeBlocksBitmapSize = *word_aux;
    word_aux = (WORD *)(buffer + 10);
    superBlock.freeInodeBitmapSize = *word_aux;
    word_aux = (WORD *)(buffer + 12);
    superBlock.inodeAreaSize = *word_aux;
    word_aux = (WORD *)(buffer + 14);
    superBlock.blockSize = *word_aux;
    dword_aux = (DWORD *)(buffer + 16);
    superBlock.diskSize = *dword_aux;
    //Setting up global variables
    bytesInBlock = superBlock.blockSize * SECTOR_SIZE;
    firstDataBlock = (1 + superBlock.freeInodeBitmapSize + superBlock.freeBlocksBitmapSize + superBlock.inodeAreaSize);
    inodeSector = (1 + superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize) * superBlock.blockSize;
    PointersInBlock = bytesInBlock / sizeof(DWORD);             //ponteiros indiretos em um bloco de dados;
    inode_per_sector = SECTOR_SIZE / sizeof(struct t2fs_inode); // denom era sizeof(DWORD)
    //Initialise the root directory
    strcpy(currentDir.pathName, "/");
    currentDir.block = firstDataBlock;

    int i;
    for (i = 0; i < MAX_NUM_FILES; i++){
        openedFiles[i].type = TYPEVAL_INVALIDO;
    }
    systemReady = 1;

    printSB();
    //dumpDisco();
    
    return 0;
}

struct t2fs_record getReg(char *path){
    char *token;
    char *nameaux;
    //struct t2fs_record dirReg;
    struct t2fs_record reg;
    struct t2fs_inode rootinode;
    BYTE buffer[SECTOR_SIZE];

    switch (filenameDir(path)){
    case 0: //diretorio pai
    case 1: // diretorio atual
            // ele vai pro diretório pai após ler o token e chamar o getRegByName
        read_sector(blockFirstSector(currentDir.block), buffer);
        memcpy(&reg, buffer, sizeof(struct t2fs_record));
        break;
    case 2: // root
        rootinode = getInode(0);
        read_sector(blockFirstSector(rootinode.dataPtr[0]), buffer);
        memcpy(&reg, buffer, sizeof(struct t2fs_record));
        break;
    }
    
    nameaux = (char *)malloc(strlen(path) + 1);
    strcpy(nameaux, path);
    token = strtok(nameaux, "/");

    while (token != NULL){
        reg = getReginDir(token, reg);
        token = strtok(NULL, "/");
    }

    free(nameaux);
    return reg;
}

struct t2fs_record getReginDir(char *name, struct t2fs_record dirReg){
    int dirBlocks = 0, offsetDirBlock = 0, offsetSingleIndBlock = 0, t = 0, offsetDoubleIndBlock = 0, h = 0, g = 0, j = 0, z = 0;
    DWORD rsector;
    DWORD pointer = 0, indpointer = 0;
    BYTE sindbuffer[SECTOR_SIZE];
    BYTE dindbuffer[SECTOR_SIZE];
    int bytesread = 0; //contabiliza o total de bytes que ja foram lidos
    struct t2fs_record regAux;
    struct t2fs_inode inodeAux;
    struct t2fs_record reg;
    BYTE buffer[SECTOR_SIZE];

    if (dirReg.TypeVal == TYPEVAL_DIRETORIO){
        if (strcmp(name, ".") == 0){ // retorna o mesmo diretorio
            reg = dirReg;
            return reg;
        }
        else if (strcmp(name, "..") == 0){ // retorna o diretorio pai
            inodeAux = getInode(dirReg.inodeNumber);
            if (inodeAux.bytesFileSize == -1){
                regAux.TypeVal = TYPEVAL_INVALIDO;
                reg = regAux;
                return reg;
            }
            read_sector(blockFirstSector(inodeAux.dataPtr[0]), sindbuffer);
            memcpy(&regAux, (sindbuffer + sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
            reg = regAux;
            return reg;
        }

        inodeAux = getInode(dirReg.inodeNumber);

        if (inodeAux.bytesFileSize == -1)
        {
            regAux.TypeVal = TYPEVAL_INVALIDO;
            reg = regAux;
            return reg;
        }
        for (dirBlocks = 0; dirBlocks < inodeAux.blocksFileSize; dirBlocks++) //percorre blocos
        {
            if (dirBlocks == 0)
                rsector = (inodeAux.dataPtr[0] * superBlock.blockSize);
            else if (dirBlocks == 1)
                rsector = (inodeAux.dataPtr[1] * superBlock.blockSize);
            else if (dirBlocks < (PointersInBlock + 2)) //INDIRECAO SIMPLES
            {
                if (offsetSingleIndBlock < superBlock.blockSize)
                {
                    if (t == (SECTOR_SIZE / sizeof(DWORD)) || offsetSingleIndBlock == 0)
                    {
                        read_sector((blockFirstSector(inodeAux.singleIndPtr)) + offsetSingleIndBlock, sindbuffer);
                        offsetSingleIndBlock++;
                        t = 0;
                    }
                }

                if (t < (SECTOR_SIZE / sizeof(DWORD)))
                {
                    memcpy(&pointer, sindbuffer + (t * sizeof(DWORD)), sizeof(DWORD));
                    rsector = (blockFirstSector(pointer));
                    t++;
                }
            }
            else //INDIRECAO DUPLA
            {
                if (offsetDoubleIndBlock < superBlock.blockSize)
                {
                    if (h == SECTOR_SIZE / sizeof(DWORD) || offsetDoubleIndBlock == 0)
                    {
                        read_sector((blockFirstSector(inodeAux.doubleIndPtr)) + offsetDoubleIndBlock, dindbuffer);
                        offsetDoubleIndBlock++;
                        h = 0;
                    }
                }

                if ((((blockFirstSector(indpointer) + (superBlock.blockSize)) == (j + blockFirstSector(indpointer))) && g == (SECTOR_SIZE / sizeof(DWORD))) || h == 0)
                {
                    memcpy(&indpointer, dindbuffer + (h * sizeof(DWORD)), sizeof(DWORD));
                    h++;
                    j = 0;
                }

                if (j < superBlock.blockSize)
                {
                    if (g == (SECTOR_SIZE / sizeof(DWORD)) || j == 0)
                    {
                        read_sector((blockFirstSector(indpointer)) + j, sindbuffer);
                        j++;
                        g = 0;
                    }
                }

                if (g < (SECTOR_SIZE / sizeof(DWORD)))
                {
                    memcpy(&pointer, sindbuffer + (g * sizeof(DWORD)), sizeof(DWORD));
                    rsector = (blockFirstSector(pointer));
                    g++;
                }
            }

            for (offsetDirBlock = 0; offsetDirBlock < superBlock.blockSize; offsetDirBlock++) //setores dentro do bloco
            {
                read_sector(rsector + offsetDirBlock, buffer);
                z = 0;
                do
                {
                    memcpy(&regAux, buffer + (z * sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
                    if (regAux.TypeVal == TYPEVAL_REGULAR || regAux.TypeVal == TYPEVAL_DIRETORIO)
                    {
                        bytesread += sizeof(struct t2fs_record);
                        if (strcmp(regAux.name, name) == 0)
                        {
                            reg = regAux;
                            return reg;
                        }
                    }
                    z++;
                } while (z < (SECTOR_SIZE / sizeof(struct t2fs_record)) && bytesread < inodeAux.bytesFileSize);
                if (bytesread >= inodeAux.bytesFileSize)
                {
                    reg.TypeVal = TYPEVAL_INVALIDO;
                    return reg;
                }
            }
        }
    }
    reg.TypeVal = TYPEVAL_INVALIDO;
    return reg;
}

struct t2fs_inode getInode(DWORD inodeNumber){
    int blockOffset = (int)(inodeNumber / inode_per_sector);
    int sectorOffset = (int)(inodeNumber % inode_per_sector);
    struct t2fs_inode inode;
    BYTE buffer[SECTOR_SIZE];

    if (read_sector(inodeSector + blockOffset, buffer)){
        printf("read_sector ERROR! (Function: getInode) \n");
        inode.bytesFileSize = -1;
        return inode;
    }
    memcpy(&inode, buffer + sectorOffset, sizeof(struct t2fs_inode));

    return inode;
}

int setInode(struct t2fs_inode inode, DWORD inodeNumber){
    DWORD iSector, iPosition;
    iSector = inodeSector + (inodeNumber / inode_per_sector);
    iPosition = inodeNumber % inode_per_sector;

    unsigned char iBuf[SECTOR_SIZE];
    if (getBitmap2(BITMAP_INODE, inodeNumber) == 0) //checa se o inode a ser modificado, "ja existe/esta ocupado"
    {
        if (setBitmap2(BITMAP_INODE, inodeNumber, 1))
        {
            printf("ERROR, trying to update inode bitmap! (Function: modifyInode) \n");
            return -1;
        }
    }

    if (read_sector(iSector, iBuf))
    {
        printf("ERROR reading sector: %i in modifyInode \n", iSector);
        return -1;
    }
    /*  Now iBuf has our iNode and others as well, we must modify only the one we want */
    /*  e.g inode number 14, if 3 per sector 14/3 = 4; 14%3 = 1 so sector 4, struct 1*/
    memcpy(iBuf + (iPosition * sizeof(struct t2fs_inode)), &inode, sizeof(struct t2fs_inode));
    if (write_sector(iSector, iBuf))
    {
        printf("ERROR writing sector %i in modifyInode", iSector);
        return -1;
    }
    return 0;
}

int absolutePath(char *abspath, char *path){
    char fullpathname[MAX_FILE_NAME_SIZE + 1];
    char name[6][59], *pathaux, *token, *current;
    int cont = 0;
    pathaux = (char *)malloc(strlen(path) + 1);
    strcpy(pathaux, path);
    switch (filenameDir(pathaux))
    {
    case 0: //diretorio pai
    case 1: // diretorio atual
        // separa o diretorio corrente em tokens e os passa para name
        current = (char *)malloc(strlen(currentDir.pathName) + 1);
        strcpy(current, currentDir.pathName);
        token = strtok(current, "/");
        while (token != NULL)
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
    while (token != NULL)
    {
        if (strcmp(token, ".") == 0)
        {
            // se é o diretorio atual, faz nada
        }
        else if (strcmp(token, "..") == 0)
        {
            // diretorio pai, apaga o ultimo name salvo
            if (cont > 0)
            {
                cont--;
            }
        }
        else
        {
            strcpy(name[cont], token); // passa o token pra uma string do nome
            cont++;
            if (cont >= 6)
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
    for (i = 0; i < cont; i++)
    {
        strcat(fullpathname, "/");
        strcat(fullpathname, name[i]);
    }
    if (strlen(fullpathname) == 0)
    {
        strcpy(fullpathname, "/");
    }

    strcpy(abspath, fullpathname);
    return 0;
}

int filenameDir(char *filename){
    if (filename[0] == '/')
    {
        return 2;
    }
    else if (filename[0] == '.' && filename[1] == '.' && filename[2] == '/')
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

unsigned int blockFirstSector(unsigned int block){
    return (block * superBlock.blockSize);
}


void printSB(){
    printf("\n------INF SUPERBLOCO---------\n");
    printf("id = %c %c %c %c\n", superBlock.id[0], superBlock.id[1], superBlock.id[2], superBlock.id[3]);
    printf("Version = %x\n", superBlock.version);
    printf("superblockSize = %i\n", superBlock.superblockSize);
    printf("freeBlocksBitmapSize = %i\n", superBlock.freeBlocksBitmapSize);
    printf("freeInodeBitmapSize = %i\n", superBlock.freeInodeBitmapSize);
    printf("inodeAreaSize = %i\n", superBlock.inodeAreaSize);
    printf("blocksize = %i\n", superBlock.blockSize);
    printf("diskSize = %i\n", superBlock.diskSize);
    printf("bytesInBlock = %i\n", bytesInBlock);
    printf("firstDataBlock = %i\n", firstDataBlock);
    printf("inodeSector = %i\n", inodeSector);
    printf("PointersInBlock = %i\n", PointersInBlock);
    printf("inode_per_sector = %i\n", inode_per_sector);
    printf("-------------------------------\n");
}

void dumpDisco(){
int i,j;
BYTE buffer[SECTOR_SIZE];

    printf("-----------DUMP DO DISCO--------------\n");
    for(i=0; i<(superBlock.diskSize*superBlock.blockSize); i++){
        read_sector(i, buffer);
        printf("%s|", buffer);
        printf("{");
        for(j=0;j<(SECTOR_SIZE/sizeof(int)); j++)
            printf("%i ", buffer+j*(sizeof(int)));
        printf("}");
    }
    printf("\n--------------------------------------\n");
}