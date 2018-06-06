/* Arthur Böckmann Grossi (275607), Cassiano Translatti Furlani(278038) e Ian Fischer Schilling(275603)*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"
/* talvez precise mais includes */


#define MAX_STU_CHAR 100
#define STU1 "Arthur Bockmann Grossi - 275607"
#define STU2 "Cassiano Translatti Furlani - 278038"
#define STU3 "Ian Fischer Schilling - 275603"

/* STRUCTURES*/
typedef struct files{   
	char fullPathName[MAX_NAME_FILE_NAME_SIZE+1];
	BYTE type;
	int currentPointer; // bytes
	struct t2fs_inode inode;
  DWORD numInode;
}fileHandler;


typedef struct cD{
    char pathName[MAX_FILE_NAME_SIZE +1];
    int block;
}dirDescription;


/* GLOBAL VARIABLES*/
struct t2fs_superbloco *superBlock;
int systemReady = 0;
BYTE buffer[SECTOR_SIZE];
dirDescription currentDir;

int bytesInBlock;
int firstDataBlock;
int inodeSector;
int inode_per_sector;
int pointers_per_sector;
DWORD indPointersInBlock;

fileHandler *openedFiles[10];


/* AUXILIAR FUNCTIONS' DECLARATION */
int setup();
struct t2fs_inode getInode(DWORD inumber);
fileHandler findFileByName(char *filename);
struct t2fs_record getRegByName(char *name, struct t2fs_record dirReg);
struct t2fs_record getFileRecord(char *filename);
DWORD findInode(char *filename);
int filenameDir(char *filename);
int delBlocks(DWORD inode, int cPointer);
int delRecord(t2fs_inode dirInode, fileHandler fileName);
int modifyInode(t2fs_inode inode, int inodeNum);
int isFileOpen(char *filename);


/* MAIN FUNCTIONS */

/*-----------------------------------------------------------------------------
Função: Usada para identificar os desenvolvedores do T2FS.
    Essa função copia um string de identificação para o ponteiro indicado por "name".
    Essa cópia não pode exceder o tamanho do buffer, informado pelo parâmetro "size".
    O string deve ser formado apenas por caracteres ASCII (Valores entre 0x20 e 0x7A) e terminado por ‘\0’.
    O string deve conter o nome e número do cartão dos participantes do grupo.

Entra:  name -> buffer onde colocar o string de identificação.
    size -> tamanho do buffer "name" (número máximo de bytes a serem copiados).

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int identify2 (char *name, int size){
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
/*-----------------------------------------------------------------------------
Função: Criar um novo arquivo.
    O nome desse novo arquivo é aquele informado pelo parâmetro "filename".
    O contador de posição do arquivo (current pointer) deve ser colocado na posição zero.
    Caso já exista um arquivo ou diretório com o mesmo nome, a função deverá retornar um erro de criação.
    A função deve retornar o identificador (handle) do arquivo.
    Esse handle será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do arquivo criado.

Entra:  filename -> nome do arquivo a ser criado.

Saída:  Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo).
    Em caso de erro, deve ser retornado um valor negativo.
-----------------------------------------------------------------------------*/
FILE2 create2 (char *filename){
  //x checar se ja existem 10 arquivos abertos
  //x Procurar se ja existe outro registro valido com o mesmo nome do arquivo a ser criado
    //x Se existir retorna -1
  //x Alocar inode para o arquivo novo
  // Varrer o diretorio no qual o arquivo sera criado, procurando por registros invalidos
    // Se existir algum registro invalido, utiliza-lo para o novo arquivo
    // Se nao existir, criar um novo registro ao final do diretorio
  
  struct t2fs_inode newInode;
  struct t2fs_record reg;
  int freeInode;  
  fileHandler file;
  
  
  if(!systemReady)
      setup();

      
  file=findFileByName(filename);
  if(file.type == TYPEVAL_INVALIDO){    //NOME VALIDO, NAO USADO AINDA
    int d=0;
    bool check=0;
    do{
      if(openedFiles[d]->type==TYPEVAL_INVALIDO){
        strcpy(openedFiles[d]->fullPathName, file.fullPathName);  //Funcao findFileByName, mesmo que retornando file=TYPEVAL_INVALIDO, ja preenche corretamente o campo do caminho
        openedFiles[d]->type = TYPEVAL_REGULAR;
        openedFiles[d]->currentPointer = 0;
        openedFiles[d]->size = 0;

        newInode.blocksFileSize = 0;
        newInode.bytesFileSize = 0;
        newInode.dataPtr[0] = INVALID_PTR;
        newInode.dataPtr[1] = INVALID_PTR;
        newInode.singleIndPtr = INVALID_PTR;
        newInode.doubleIndPtr = INVALID_PTR;
        newInode.reservado[2] = NULL;

        openedFiles[d]->inodeNumber = searchBitmap2(BITMAP_INODE, 0);
        modifyInode(newinode, openedFiles[d]->inodeNumber);

        
        //CRIAR O REGISTRO DO NOVO ARQUIVO DENTRO DO DIRETORIO
        char *pathless = malloc(strlen(filename));  //precisamos de caminho
        strcpy(pathless, filename);
        
        //antes de criar um novo registro, checar se ja nao existem registros invalidos no diretorio que possam ser usados
        check = 1;
      }
      d++;
    }while(check==0 && d<10);
    
    if(check==0){       //havia 10 aquivos abertos
      printf("ERROR, there was already 10 opened files! \n");
      return -1;
    }
    
  }else{
    prinft("ERROR, filename already being used! \n");
    return -1;
  }
}
/*-----------------------------------------------------------------------------
Função: Apagar um arquivo do disco.
    O nome do arquivo a ser apagado é aquele informado pelo parâmetro "filename".

Entra:  filename -> nome do arquivo a ser apagado.

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename){ 
  //  Procura se existe o arquivo
  //  Se não existir retorna -1
  //  Acha o inode do arquivo
  //  Acha o registro do diretório do arquivo
  //  Modifica o diretório (delRecord)
  //  Libera os blocos do arquivo(delBlocks)
  //  Altera bitmap do inode
  
  fileHandler delFile;
  if( ifFileOpen(filename) ){
    printf("File is open, can't delete\n");
    return -1;
  }
  
  delFile = findFileByName(filename);
  
  if( delFile.type == TYPEVAL_INVALIDO ){
    printf("No file with such name exists\n");
    return -1;
  }
  
  if( delFile.type == TYPEVAL_DIRETORIO ){
    printf("Tried to delete a directory with delete2\n");
    return -1;
  }
  
  //now we must delete the blocks
  if( !delBlocks(delFile.inode, -1) ){
    printf("Error deleting blocks in delete2\n");
    return -1;
  }
  
  //modify the record
  t2fs_inode dirInode(filename);//gets the directory inode
  if( !delRecord(dirInode, filename) ){
    printf ("Error deleting record in delete2\n");
    return -1;
  }
  int fileInodeNum = findInode(filename);
  if( !setBitmap2(BITMAP_INODE,fileInodeNum,0) ){
    printf("Error setting inode bitmap\n");
    return -1;
  }
   
}
/*-----------------------------------------------------------------------------
Função: Abre um arquivo existente no disco.
    O nome desse novo arquivo é aquele informado pelo parâmetro "filename".
    Ao abrir um arquivo, o contador de posição do arquivo (current pointer) deve ser colocado na posição zero.
    A função deve retornar o identificador (handle) do arquivo.
    Esse handle será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do arquivo criado.
    Todos os arquivos abertos por esta chamada são abertos em leitura e em escrita.
    O ponto em que a leitura, ou escrita, será realizada é fornecido pelo valor current_pointer (ver função seek2).

Entra:  filename -> nome do arquivo a ser apagado.

Saída:  Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo)
    Em caso de erro, deve ser retornado um valor negativo
-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename){
  fileHandler file;
	file = findFileByName(filename);
	file.currentPointer = 0;
	
	if(file.type == TYPEVAL_INVALIDO){
		printf("ERROR: file not found!\n");
		return -1;
	}
	
	int i=0;
	// searches for the next free openedFiles
	while((i<10) && (openedFiles[i].type == TYPEVAL_INVALIDO))
		i++;
	
	if(i >= 10){
		printf("ERROR: openedFiles is full!\n");
		return -1;
	}
	
	openedFiles[i] = file;
	return i;
}
/*-----------------------------------------------------------------------------
Função: Fecha o arquivo identificado pelo parâmetro "handle".

Entra:  handle -> identificador do arquivo a ser fechado

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle){
  fileHandler *file;
	file = &(openedFiles[handle]);
	
	if (file->type != TYPEVAL_REGULAR) {
		printf("ERROR: handle file not found!\n");
		return 1;
	}

	file->fullPathName[0] = '\0';
	file->type = TYPEVAL_INVALIDO;
	return 0;
}
/*-----------------------------------------------------------------------------
Função: Realiza a leitura de "size" bytes do arquivo identificado por "handle".
    Os bytes lidos são colocados na área apontada por "buffer".
    Após a leitura, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último lido.

Entra:  handle -> identificador do arquivo a ser lido
    buffer -> buffer onde colocar os bytes lidos do arquivo
    size -> número de bytes a serem lidos

Saída:  Se a operação foi realizada com sucesso, a função retorna o número de bytes lidos.
    Se o valor retornado for menor do que "size", então o contador de posição atingiu o final do arquivo.
    Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Realiza a escrita de "size" bytes no arquivo identificado por "handle".
    Os bytes a serem escritos estão na área apontada por "buffer".
    Após a escrita, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último escrito.

Entra:  handle -> identificador do arquivo a ser escrito
    buffer -> buffer de onde pegar os bytes a serem escritos no arquivo
    size -> número de bytes a serem escritos

Saída:  Se a operação foi realizada com sucesso, a função retorna o número de bytes efetivamente escritos.
    Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Função usada para truncar um arquivo.
    Remove do arquivo todos os bytes a partir da posição atual do contador de posição (CP)
    Todos os bytes a partir da posição CP (inclusive) serão removidos do arquivo.
    Após a operação, o arquivo deverá contar com CP bytes e o ponteiro estará no final do arquivo

Entra:  handle -> identificador do arquivo a ser truncado

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int truncate2 (FILE2 handle){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Reposiciona o contador de posições (current pointer) do arquivo identificado por "handle".
    A nova posição é determinada pelo parâmetro "offset".
    O parâmetro "offset" corresponde ao deslocamento, em bytes, contados a partir do início do arquivo.
    Se o valor de "offset" for "-1", o current_pointer deverá ser posicionado no byte seguinte ao final do arquivo,
        Isso é útil para permitir que novos dados sejam adicionados no final de um arquivo já existente.

Entra:  handle -> identificador do arquivo a ser escrito
    offset -> deslocamento, em bytes, onde posicionar o "current pointer".

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int seek2 (FILE2 handle, DWORD offset){
  fileHandler *file;
	file = &(openedFiles[handle]);

	if (offset > file->inode.bytesFileSize) {
		printf("ERROR: offset greater than filesize!\n");
		return 1;
	}

	if(offset == -1){
		file->currentPointer = file->inode.bytesFileSize;
	}
	else {
		file->currentPointer = offset;
	}

	return 0;
}
/*-----------------------------------------------------------------------------
Função: Criar um novo diretório.
    O caminho desse novo diretório é aquele informado pelo parâmetro "pathname".
        O caminho pode ser ser absoluto ou relativo.
    São considerados erros de criação quaisquer situações em que o diretório não possa ser criado.
        Isso inclui a existência de um arquivo ou diretório com o mesmo "pathname".

Entra:  pathname -> caminho do diretório a ser criado

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int mkdir2 (char *pathname){
    /* TO-DO */
  // /user/ian/trab/
}
/*-----------------------------------------------------------------------------
Função: Apagar um subdiretório do disco.
    O caminho do diretório a ser apagado é aquele informado pelo parâmetro "pathname".
    São considerados erros quaisquer situações que impeçam a operação.
        Isso inclui:
            (a) o diretório a ser removido não está vazio;
            (b) "pathname" não existente;
            (c) algum dos componentes do "pathname" não existe (caminho inválido);
            (d) o "pathname" indicado não é um arquivo;

Entra:  pathname -> caminho do diretório a ser criado

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int rmdir2 (char *pathname){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Altera o diretório atual de trabalho (working directory).
        O caminho desse diretório é informado no parâmetro "pathname".
        São considerados erros:
            (a) qualquer situação que impeça a realização da operação
            (b) não existência do "pathname" informado.

Entra:  pathname -> caminho do novo diretório de trabalho.

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
        Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int chdir2 (char *pathname){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Informa o diretório atual de trabalho.
        O "pathname" do diretório de trabalho deve ser copiado para o buffer indicado por "pathname".
            Essa cópia não pode exceder o tamanho do buffer, informado pelo parâmetro "size".
        São considerados erros:
            (a) quaisquer situações que impeçam a realização da operação
            (b) espaço insuficiente no buffer "pathname", cujo tamanho está informado por "size".

Entra:  pathname -> buffer para onde copiar o pathname do diretório de trabalho
        size -> tamanho do buffer pathname

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
        Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getcwd2 (char *pathname, int size){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Abre um diretório existente no disco.
    O caminho desse diretório é aquele informado pelo parâmetro "pathname".
    Se a operação foi realizada com sucesso, a função:
        (a) deve retornar o identificador (handle) do diretório
        (b) deve posicionar o ponteiro de entradas (current entry) na primeira posição válida do diretório "pathname".
    O handle retornado será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do diretório.

Entra:  pathname -> caminho do diretório a ser aberto

Saída:  Se a operação foi realizada com sucesso, a função retorna o identificador do diretório (handle).
    Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
DIR2 opendir2 (char *pathname){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Realiza a leitura das entradas do diretório identificado por "handle".
    A cada chamada da função é lida a entrada seguinte do diretório representado pelo identificador "handle".
    Algumas das informações dessas entradas devem ser colocadas no parâmetro "dentry".
    Após realizada a leitura de uma entrada, o ponteiro de entradas (current entry) deve ser ajustado para a próxima entrada válida, seguinte à última lida.
    São considerados erros:
        (a) qualquer situação que impeça a realização da operação
        (b) término das entradas válidas do diretório identificado por "handle".

Entra:  handle -> identificador do diretório cujas entradas deseja-se ler.
    dentry -> estrutura de dados onde a função coloca as informações da entrada lida.

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero ( e "dentry" não será válido)
-----------------------------------------------------------------------------*/
int readdir2 (DIR2 handle, DIRENT2 *dentry){
    /* TO-DO */
}
/*-----------------------------------------------------------------------------
Função: Fecha o diretório identificado pelo parâmetro "handle".

Entra:  handle -> identificador do diretório que se deseja fechar (encerrar a operação).

Saída:  Se a operação foi realizada com sucesso, a função retorna "0" (zero).
    Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle){
    /* TO-DO */
}


/* AUXILIAR FUNCTIONS */
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

    strncpy(temp, buffer+4, 2); //version
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

    //Setting up global variables
    bytesInBlock = superBlock->blockSize * SECTOR_SIZE;
    firstDataBlock = (1 + superBlock->freeInodeBitmapSize + superBlock->freeBlocksBitmapSize + superBlock->inodeAreaSize);
    inodeSector = (1 + superBlock->freeBlocksBitmapSize + superBlock->freeInodeBitmapSize)*blockSize;
    inodeAux = SECTOR_SIZE/sizeof(struct t2fs_inode);
    indPointersInBlock= bytesInBlock/sizeof(DWORD);                     //ponteiros indiretos em um bloco de dados;
  
    ///Initialise the root directory
    strcpy(currentDir.pathName, "/");
    currentDir.block = firstDataBlock;

    //First two records must be current directory and father directory
    struct t2fs_record rootRecord0, rootRecord1;
    rootRecord0->TypeVal = TYPEVAL_DIRETORIO;
    rootRecord0->name = ".";
    rootRecord0->inodeNumber = 0;

    rootRecord1->TypeVal = TYPEVAL_DIRETORIO;
    rootRecord1->name = "..";
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
   
    
    openedFiles = calloc(10, sizeof(fileHandler));
    
    return 0;
}

/*-----------------------------------------------------------------------------
Função:	Procura Inode com indice passado em inumber
Entra:	inumber -> indice do inode a ser retornado.
Saída:	sucesso, retorna o inode
        caso de erro de leitura do setor, retorna blocksFileSize = -1 (testar se é igual a -1, se testar se é menor que 0 da erro)
-----------------------------------------------------------------------------*/
struct t2fs_inode getInode(DWORD inumber){
    int blockOffset = (int) (inumber/inodesPerSector);
    int sectorOffset = (int) (inumber % inodesPerSector);
    struct t2fs_inode inode;

    if(read_sector(inodeSector + blockOffset, buffer)){
      printf("read_sector ERROR! (Function: getInode) \n");
      inode.bytesFileSize = -1;
      return inode;
    }
    memcpy(&inode, buffer + sectorOffset, sizeof(struct t2fs_inode));
  
    return inode;
}
  
/*-----------------------------------------------------------------------------
Função:	Procura o arquivo a partir do nome absoluto ou relativo "filename".
		Preenche todo fileHandler (fullPathName, type, currentPointer e inode).
Entra:	filename -> nome do arquivo a ser procurado.
Saída:	Se a operação foi realizada com sucesso, a função retorna o fileHandler do arquivo.
			    type = TYPEVAL_REGULAR;
		    Em caso de erro, será retornado com type = TYPEVAL_INVALIDO.
-----------------------------------------------------------------------------*/
fileHandler findFileByName(char *filename){ /* strcat mal usado, ele está alterando o currentDir.pathname */
  filehandler file;
  char *token;
  struct t2fs_record regAux;
  struct t2fs_inode inodeAux;
  char nameAux[MAX_FILE_NAME_SIZE];
  char auxPathName[MAX_FILE_NAME_SIZE];
  
  
  strcpy(nameAux, filename);
  
  
  token = strtok(nameAux, "/");
  
  if(nameAux[0]=='/'){   //CAMINHO ABSOLUTO  regAux <= regDirRaiz
    
    read_sector(blockFirstSector(firstDataBlock), buffer);
    memcpy(&regAux, buffer, sizeof(struct t2fs_record));
    strcpy(file.fullPathName, filename);
  }else{                //CAMINHO RELATIVO
    strcpy(auxPathName, currentDir.pathName);
    if(!strcmp(token, ".")){ //regAux <= regDirCorrente
      read_sector(blockFirstSector(currentDir.block), buffer);
      memcpy(&regAux, buffer, sizeof(struct t2fs_record));
      token = strtok(NULL, "/");
      strcpy(file.fullPathName, strcat(auxPathName, filename));
    }
    else if(!strcmp(token, "..")){  //regAux <= regDirPai
      read_sector(blockFirstSector(currentDir.block), buffer);
      memcpy(&regAux, buffer+(1*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
      token = strtok(NULL, "/");
      strcpy(file.fullPathName, strcat(auxPathName, filename+3));
    }
    else{  //regAux <= regDirCorrente
      read_sector(blockFirstSector(currentDir.block), buffer);
      memcpy(&regAux, buffer, sizeof(struct t2fs_record));
      strcpy(file.fullPathName, strcat(auxPathName, filename));
    }
  }
  
  do{
    regAux = getRegByName(token, regAux);
    if(regAux.TypeVal == TYPEVAL_INVALIDO){
      printf("ERROR: could not find the directory! (Function: findFileByName) \n");
      file.type = TYPEVAL_INVALIDO;
      return file;
    }
    
    token = strtok(NULL, "/");
  }while(token!=NULL);
  
  
  //regAux <= regArquivoProcurado
  inodeAux = getInode(regAux.inodeNumber);
  if(inodeAux.bytesFileSize == -1){
    file.type == TYPEVAL_INVALIDO;
    return file;
  }
  
  file.type == TYPEVAL_REGULAR;
  file.currentPointer = 0;
  memcpy(&file.inode, &inodeAux, sizeof(struct t2fs_inode));
  return file;
}

/*-----------------------------------------------------------------------------
Função:	Procura um registro VALIDO com nome igual a name, no diretorio apontado por dirReg
Entra:	name -> nome do arquivo a ser procurado.
        Se for '.' retorna o mesmo diretorio.
        Se for '..' retorna o diretório pai.
        dirReg -> Registro do diretorio onde o a busca sera feita
Saída:	Se a operação foi realizada com sucesso, a função retorna o registro do arquivo encontrado
		    Em caso de erro, será retornado o registro com TypeVal = TYPEVAL_INVALIDO.
-----------------------------------------------------------------------------*/
struct t2fs_record getRegByName(char *name, struct t2fs_record dirReg){
int dirBlocks=0, offsetDirBlock=0, offsetSingleIndBlock=0, t=0, offsetDoubleIndBlock=0, h=0, g=0, j=0, z=0;
DWORD rsector;
DWORD pointer, indpointer;
BYTE sindbuffer[SECTOR_SIZE];
BYTE dindbuffer[SECTOR_SIZE];
DWORD bytesread=0;  //contabiliza o total de bytes que ja foram lidos
struct t2fs_inode regAux;
struct t2fs_inode inodeAux;
  
  if(strcmp(name, ".") == 0){ // retorna o mesmo diretorio
    return dirReg;
  }
  else if(strcmp(name, "..") == 0){ // retorna o diretorio pai
    inodeAux = getInode(dirReg.inodeNumber);
    if(inodeAux.bytesFileSize == -1){
      regAux.TypeVal == TYPEVAL_INVALIDO;
      return regAux;
    }
    read_sector(blockFirstSector(inodeAux->dataPtr[0]), sindbuffer);
    memcpy(&regAux, (sindbuffer + sizeof(struct t2fs_inode)), sizeof(struct t2fs_inode)));
    return regAux;
  }
  
  if(dirReg.TypeVal==TYPEVAL_DIRETORIO){    
    inodeAux = getInode(dirReg.inodeNumber);
    if(inodeAux.bytesFileSize == -1){
      regAux.TypeVal == TYPEVAL_INVALIDO;
      return regAux;
    }

    for(dirBlocks=0; dirBlocks < inodeAux->blocksFileSize; dirBlocks++){   //percorre blocos
        if(dirBlocks==0)
            rsector = (inodeAux->dataPtr[0] * superBlock->blockSize);
        else if(dirBlocks==1)
            rsector = (inodeAux->dataPtr[1] * superBlock->blockSize);
        else if(dirBlocks<pointersSingleInd){               //INDIRECAO SIMPLES
            if(offsetSingleIndBlock < superblock->blockSize){
                read_sector((blockFirstSector(inodeAux->singleIndPtr)) + offsetSingleIndBlock, sindbuffer);
                offsetSingleIndBlock++;

                if(t < (pointersSingleInd/superBlock->blockSize)){
                    memcpy(&pointer, sindbuffer+(t*sizeof(DWORD)), sizeof(DWORD));
                    rsector = (blockFirstSector(pointer));
                    t++;
                }
            }
        }else{                                            //INDIRECAO DUPLA

            if(offsetDoubleIndBlock<superBlock->blockSize){
                read_sector((blockFirstSector(inodeAux->doubleIndPtr))+offsetDoubleIndBlock, dindbuffer);
                offsetDoubleIndBlock++;

                if(h<(pointersDoubleInd/superBlock->blockSize)){
                    memcpy(&indpointer, dindbuffer + (h*sizeof(DWORD)), sizeof(DWORD));
                    h++;

                    if(j<superblock->blockSize){
                        read_sector((blockFirstSector(indpointer)) + j, indbuffer);
                        j++;

                        if(g<(pointersSingleInd/superBlock->blockSize)){
                            memcpy(&pointer, indbuffer+(g*sizeof(DWORD)), sizeof(DWORD));
                            rsector = (blockFirstSector(pointer));
                            g++;
                        }
                    }
                }
            }
        }
        for(offsetDirBlock=0; offsetDirBlock<superblock->blockSize; offsetDirBlock++){  //setores dentro do bloco
            read_sector(rsector + offsetDirBlock, buffer);
            do{
                memcpy(&regAux, buffer+(z*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
                if(regAux.TypeVal == TYPEVAL_REGULAR || regAux.TypeVal == TYPEVAL_DIRETORIO){
                  bytesread += sizeof(struct t2fs_record);
                  if(strcmp(regAux.name, name)==0)
                    return regAux;
                }
            }while(bytesread<inodeAux->bytesFileSize);
        }
    }
  }
  
regAux.TypeVal = TYPEVAL_INVALIDO;

return regAux;
  
}

/*---------------------------------------------------------------------------------
Função: Procura o t2fs_record do arquivo especificado pelo nome "filename", podendo ser um caminho relativo ou absoluto. 
Entrada: filename -> nome absoluto ou relativo do arquivo ou diretório.
Saída: Se a operação foi realizada com sucesso, a função retorna o registro do arquivo encontrado
		    Em caso de erro, será retornado o registro com TypeVal = TYPEVAL_INVALIDO.
-----------------------------------------------------------------------------------*/
struct t2fs_record getFileRecord(char *filename){
  char *token;
  char *nameaux;
  int namesize;
  dirDescription dirReg;
  
  nameaux = (char *) malloc(strlen(filename) +1);
  strcpy(nameaux, filename);
  namesize = strlen(nameaux);
  token = strtok(nameaux, "/");
  
  switch(filenameDir(filename)){
    case 0: //diretorio pai
    case 1: // diretorio atual
      dirReg = currentDir;
      break;
    case 2: // root
      dirReg = 
      break;
    default:
      dirReg.
      break;
  }
  
  
  getRegByName(char *name, struct t2fs_record dirReg)
}

/*---------------------------------------------------------------------------------
Função: Procura o inode do arquivo especificado pelo nome "filename" 
Entrada: filename -> nome do arquivo ou diretório
Saída: Se a operação for realizada com sucesso, retorna o número de um inode(t2fs_inode)
       Em caso de erro retorna -1;

-----------------------------------------------------------------------------------*/
DWORD findInode(char *filename){
  char *token;
  char *nameaux;
  
  nameaux = (char *) malloc(strlen(filename) +1);
  strcpy(nameaux, filename);
  token = strtok(nameaux, "/");
  
  switch(filenameDir(filename)){
    case 0:
      
      break;
    case 1:
      
      break;
    case 2:
      
      break;
    default:
      
      break;
  }
  
  
  
  //              /p1/p2/p3/p4/p5/teste.txt
  //              ./p4/p5/teste.txt
  //              ../p7/p8/p9/teste1.txt
  //              /teste.txt
  
  token = strtok(filename, "/");
  
  if(filename[0]=='/'){   //CAMINHO ABSOLUTO  regAux <= regDirRaiz
    read_sector(firstDataBlock * superBlock.blockSize, buffer);
    memcpy(&regAux, buffer, sizeof(struct t2fs_record));
    strcpy(file.fullPathName, filename);
  }else{    //CAMINHO RELATIVO
    if(strcmp(token, ".")){ //regAux <= regDirCorrente
      read_sector(currentDir.block * superBlock.blockSize, buffer);
      memcpy(&regAux, buffer, sizeof(struct t2fs_record));
      token = strtok(NULL, "/");
      strncpy(auxPathName, filename+1, strlen(filename));
      file.fullPathName = strcat(currentdir.pathName, auxPathName);
    }else if(strcmp(token, "..")){  //regAux <= regDirPai
      read_sector(currentDir.block * superBlock.blockSize, buffer);
      memcpy(&regAux, buffer+(1*sizeof(struct t2fs_record)), sizeof(struct t2fs_record));
      token = strtok(NULL, "/");
      strncpy(auxPathName, filename+2, strlen(filename));
      file.fullPathName = strcat(currentdir.pathName, auxPathName);
    }
  }
  
  do{
    regAux = getRegByName(token, regAux);
    if(regAux.TypeVal == TYPEVAL_INVALIDO){
      printf("ERROR, could not find the directory! (Function: findFileByName) \n")
      file.type = TYPEVAL_INVALIDO;
      return file;
    }
    
    token = strtok(NULL, "/");
  }while(token!=NULL);
  
  
  //regAux <= regArquivoProcurado
  inodeAux = getInode(regAux.inodeNumber);
  if(inodeAux.blocksFileSize==0){
    printf("ERROR, inode has 0 blocks! (Function: findFileByName) \n")
    file.type == TYPEVAL_INVALIDO;
    return file;
  }
  
  file.type == TYPEVAL_REGULAR;
  file.currentPointer = 0;
  file.inode = inodeAux;
  return file;
}

/*---------------------------------------------------------------------------------
Função: Retorna o tipo do primeiro diretório de filename
Entrada: filename -> nome do arquivo ou diretório
Saída: Se a operação for realizada com sucesso, retorna
        0 -> diretório pai
        1 -> diretório atual
        2 -> root
       Em caso de erro retorna -1;

-----------------------------------------------------------------------------------*/  
int filenameDir(char *filename){
  if(filename[0]=='/'){
    return 2;
  }else if(filename[0]=='.' && filename[1]=='.' && filename[2]=='/'){
    return 0;
  }else{
    return 1;
  }
}

/*---------------------------------------------------------------------------------
Função: Retorna o primeiro setor correspondente ao bloco
Entrada: block -> numero do bloco
Saída: retorna sempre o primeiro setor do bloco
-----------------------------------------------------------------------------------*/
unsigned int blockFirstSector(unsigned int block){
  return (block * superBlock->blockSize);
}

/*-----------------------------------------------------------------------------------
Função: Percorre os blocos que estão sendo utilizados pelo inode e libera eles
Entrada:  inode -> número do inode do arquivo 
          cPointer -> Se -1 apaga todos os bytes, se outro valor maior que zero apaga 
          a partir do "cPointer"(inclusive)
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int delBlocks(DWORD inode, int cPointer){
  
  //read inode 
  t2fs_inode delNode= getInode(inode);
  DWORD blocksRemaining = delNode.blocksFileSize;
  DWORD dBlocksToDel;
  
  DWORD remainingBlocks;/*Used in truncate*/
  DWORD remainingBytes;/*Used in truncate*/
  
  
  // Começando do final para o início
  if(delNode.blocksFileSize > (2 + indPointersInBlock)){
    /* Temos indireção dupla*/
    DWORD dIndPtr[indPointersInBlock][indPointersInBlock];
    dBlocksToDel = delNode.blocksFileSize - (2 + indPointersInBlock);
    DWORD dFirstBLock = delNode.doubleIndPtr;
    char tempBuff[SECTOR_SIZE];
    if( !read_sector(blockFirstSector(dFirstBLock), tempBuff) ){
      printf("ERROR reading sector %i in delBlocks\n", blockFirstSector(dFirstBLock));
      return -1;
    }
    memcpy();
    
    
    
  }
  
  
  
   
  return 0;
  
}

/*-----------------------------------------------------------------------------------
Função: Libera o registro do diretorio do arquivo
Entrada: dirInode -> número do inode do diretório 
          filename -> nome do arquivo a ser invalidado(absoluto ou relativo)
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int delRecord(t2fs_inode dirInode, fileHandler fileName){
  /*  TO-DO */
}

/*-----------------------------------------------------------------------------------
Função: Modifica o inode de número inodeNum (Inode de indice inodeNum vira uma copia da estrutura inode passada)
Entrada: inode -> estrutura do inode 
          inodeNum -> número do inode
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int modifyInode(t2fs_inode inode, int inodeNum){
  DWORD iSector, iPosition;
  iSector = inodeSector + (inodeNum/inode_per_sector);
  iPosition = inodeNum % inode_per_sector;
  
  unsigned char iBuf[SECTOR_SIZE];
  if(getBitMap2(BITMAP_INODE, inodeNum)==0) //checa se o inode a ser modificado, "ja existe/esta ocupado"
      if(setBitMap2(BITMAP_INODE, inodeNum, 1)) 
        printf("ERROR, trying to update inode bitmap! (Function: modifyInode) \n");
  
  if( !read_sector(iSector, iBuf) ){
    printf("ERROR reading sector: %i in modifyInode \n", iSector);
    return -1;
  }
  /*  Now iBuf has our iNode and others as well, we must modify only the one we want */
  /*  e.g inode number 14, if 3 per sector 14/3 = 4; 14%3 = 1 so sector 4, struct 1*/
  memcpy(iBuf + (iPosition*sizeof(t2fs_inode)), inode, sizeof(t2fs_inode));
  if( !write_sector(iSector, iBuf) ){
    printf("ERROR writing sector %i in modifyInode", iSector);
    return -1;  
  }
  return 0;  
}

/*-----------------------------------------------------------------------------------
Função: Checa se o arquivo ou diretorio com o nome filename já está aberto
Entrada: filename -> nome do arquivo (relativo ou absoluto)
Saída: Retorna 0 se o arquivo ou diretório não estiver aberto, se não, retorna um número diferente de 0
-----------------------------------------------------------------------------------*/
int isFileOpen(char *filename){
  /*TO-DO*/
}