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
	char fullPathName[MAX_NAME_SIZE];
	BYTE type;
	int currentPointer; // bytes
	struct t2fs_inode inode;
}fileHandler;


typedef struct cD{
    char pathName[MAX_NAME_SIZE];
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
int setup();//OK
struct t2fs_inode getInode(DWORD inumber);
boolean validRegName(char *filename);

fileHandler findFileByName(char *filename);
t2fs_inode findInode(char *filename);
int delBlocks(t2fs_inode inode, int cPointer);
int delRecord(t2fs_inode dirInode, fileHandler fileName);


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
  // Procurar se ja existe outro registro valido com o mesmo nome do arquivo a ser criado
    // Se existir retorna -1
  // Alocar inode para o arquivo novo
  // Varrer o diretorio no qual o arquivo sera criado, procurando por registros invalidos
    // Se existir algum registro invalido, utiliza-lo para o novo arquivo
    // Se nao existir, criar um novo registro ao final do diretorio
  
  

  
  struct t2fs_inode newInode;
  struct t2fs_record reg;
  int freeInode;

  if(!systemReady)
      setup();
  if(validRegName){
    int d;
    bool check=0;
    for(d=0;d<10;d++)
        if(openedFiles[d]->type==TYPEVAL_INVALIDO){
          strcpy(openedFiles[d]->fullPathName, strcat(strcat(currentDir->pathName, "/"), filename));
          openedFiles[d]->type = TYPEVAL_REGULAR;
          openedFiles[d]->currentPointer = 0;

          
          newInode.blocksFileSize = 1;
          newInode.bytesFileSize = 0;
          newInode.dataPtr[0] = searchBitmap2(BITMAP_DADOS, 0);
          newInode.dataPtr[1] = INVALID_PTR;
          newInode.singleIndPtr = INVALID_PTR;
          newInode.doubleIndPtr = INVALID_PTR;
          newInode.reservado[2] = NULL;
          
          // initialize new inode
          freeInode = searchBitmap2(BITMAP_INODE, 0);
          int offset_in_block = (int) (freeInode / inode_per_sector);/* fI = 13; iS = 5; / 2 % 3 */ 
          int offset_in_sector = (int) (freeInode % inode_per_sector);
          read_sector(inodeSector + offset_in_block, buffer);
          memcpy(buffer+(offset_in_sector*sizeof(struct t2fs_inode)),newInode, sizeof(struct t2fs_inode));
          write_sector(inodeSector + offset_in_block, buffer);
          
          if(setBitmap2(BITMAP_INODE, freeInode, 1)<0){   //atualiza bitmap de inodes
            printf("Could not set bitmap block\n");
			      return -1;
          }   
          
          if(setBitmap2(BITMAP_DADOS, searchBitmap2(BITMAP_DADOS, 0), 1)<0){      //atualiza bitmap de dados
            printf("Could not set bitmap block\n");
			      return -1;
          }
          
          openedFiles[d]->inodeNumber = freeInode;
          
          //CRIAR O REGISTRO DO NOVO ARQUIVO DENTRO DO DIRETORIO CORRENTE   //OBS: CUIDAR NA HORA DE ALOCAR O REGISTRO, VER SE JA NAO EXISTE ALGUM OUTRO REGISTRO COM O MESMO NOME DO ARQUIVO QUE ESTEJA INVALIDO
          read_sector((currentDir->block)*(superblock->blockSize), buffer);
          
          

          
          openedFiles[d]->size = 0;/*REVISION*/
          check = 1;
        }
    
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
    currentDir = calloc(1, sizeof(dirDescription));
    strcpy(currentDir->pathName, "/");
    currentDir->block = firstDataBlock;

    //First two records must be current directory and father directory
    struct t2fs_record rootRecord0, rootRecord1;
    rootRecord0->TypeVal = TYPEVAL_DIRETORIO;
    rootRecord0->name = "root";
    rootRecord0->inodeNumber = 0;

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
   
    
    openedFiles = calloc(10, sizeof(fileHandler));
    
    return 0;
}

/*-----------------------------------------------------------------------------
Função:	Procura Inode com indice passado em inumber
Entra:	inumber -> indice do inode a ser retornado.
Saída:	sucesso, retorna o inode
        caso de erro de leitura do setor, retorna inode com inode.blocksFileSize == 0
-----------------------------------------------------------------------------*/
struct t2fs_inode getInode(DWORD inumber){
    int blockOffset = (int) (inumber/inodesPerSector);
    int sectorOffset = (int) (inumber % inodesPerSector);
    struct t2fs_inode inode;
    

    if(read_sector(inodeSector + blockOffset, buffer)){
      printf("read_sector ERROR! (Function: getInode) \n");
      inode.blocksFileSize = 0;
      return inode;
    }
    memcpy(&inode, buffer + sectorOffset, sizeof(struct t2fs_inode));

    return inode;
}

/*-----------------------------------------------------------------------------
Função:	Procura no diretorio corrrente por um registro do tipo REGULAR com o mesmo nome de 
        filename
Entra:	filename -> nome do arquivo a ser procurado.
Saída:	Se a funcao encontrar um registro com mesmo nome que filename, RETORNA 0
        Caso contrario RETORNA 1
-----------------------------------------------------------------------------*/
boolean validRegName(char *filename){
int dirBlocks=0, offsetDirBlock=0, offsetSingleIndBlock=0, t=0, offsetDoubleIndBlock=0, h=0, g=0, j=0, z=0;
DWORD rsector;
DWORD pointer, indpointer;
BYTE sindbuffer[SECTOR_SIZE];
BYTE dindbuffer[SECTOR_SIZE];
DWORD bytesread=0;  //contabiliza o total de bytes que ja foram lidos
struct t2fs_record dirReg;
struct t2fs_inode dirInode;
struct t2fs_inode regaux;
  
  
  
    read_sector((currentDir->block) * (superBlock->blockSize), buffer);
    memcpy(&dirReg, buffer, sizeof(t2fs_record));                       //registro do diretorio corrente

    &dirInode = getInode(dirReg.inodeNumber);


  for(dirBlocks=0; dirBlocks<dirInode.blocksFileSize; dirBlocks++){   //percorre blocos
      if(dirBlocks==0)
          rsector = (dirInode.dataPtr[0] * superBlock->blockSize);
      else if(dirBlocks==1)
          rsector = (dirInode.dataPtr[1] * superBlock->blockSize);
      else if(dirBlocks<pointersSingleInd){               //INDIRECAO SIMPLES

          if(offsetSingleIndBlock<superblock->blockSize){
              read_sector(((dirInode.singleIndPtr)*(superBlock->blockSize)) + offsetSingleIndBlock, sindbuffer);
              offsetSingleIndBlock++;

              if(t<(pointersSingleInd/superBlock->blockSize)){
                  memcpy(&pointer, sindbuffer+(t*sizeof(DWORD)), sizeof(DWORD));
                  rsector = (pointer * superBlock->blockSize);
                  t++;
              }
          }

      }else{                                            //INDIRECAO DUPLA

          if(offsetDoubleIndBlock<superBlock->blockSize){
              read_sector(((dirInode.doubleIndPtr)*(superBlock->blockSize))+offsetDoubleIndBlock, dindbuffer);
              offsetDoubleIndBlock++;

              if(h<(pointersDoubleInd/superBlock->blockSize)){
                  memcpy(&indpointer, dindbuffer + (h*sizeof(DWORD)), sizeof(DWORD));
                  h++;

                  if(j<superblock->blockSize){
                      read_sector(((indpointer)*(superBlock->blockSize)) + j, indbuffer);
                      j++;

                      if(g<(pointersSingleInd/superBlock->blockSize)){
                          memcpy(&pointer, indbuffer+(g*sizeof(DWORD)), sizeof(DWORD));
                          rsector = (pointer * superBlock->blockSize);
                          g++;
                      }
                  }
              }
          }
      }
      for(offsetDirBlock=0; offsetDirBlock<superblock->blockSize; offsetDirBlock++){  //setores dentro do bloco
          read_sector(rsector + offsetDirBlock, buffer);
          do{
              memcpy(&regaux, buffer+(z*sizeof(t2fs_record)), sizeof(t2fs_record));
              bytesread += sizeof(t2fs_record);
              if(strcmp(regaux.name, filename)==0){
                if(regaux.TypeVal == TYPEVAL_REGULAR)
                  return 0;  //filename JA USADO
              }
          }while(bytesread<dirInode.bytesFileSize);
      }
  }

  return 1; //filename PODE SER USADO
}
  
  /*-----------------------------------------------------------------------------
Função:	Procura o arquivo a partir do nome absoluto ou relativo "filename".
		Preenche todo fileHandler (fullPathName, type, currentPointer e inode).
Entra:	filename -> nome do arquivo a ser procurado.
Saída:	Se a operação foi realizada com sucesso, a função retorna o fileHandler do arquivo.
			    type = TYPEVAL_REGULAR;
		    Em caso de erro, será retornado com type = TYPEVAL_INVALIDO.
-----------------------------------------------------------------------------*/
fileHandler findFileByName(char *filename){
  //Quebrar filename em tokens
  //checar se o primeiro caractere é '/'
    //caso seja, TRATAR COMO CAMINHO ABSOLUTO
      //ler primeiro bloco de dados
      //carregar registro do root
    //caso nao, checar se token é '..' entao acessa o registro do diretorio pai
  //checar se token
  
  filehandler file;
  char *token;
  struct t2fs_record regAux;
  struct t2fs_inode inodeAux;
  char auxPathName[MAX_NAME_SIZE];
  
  //              /p1/p2/p3/p4/p5/teste.txt
  //              ./p4/p5/teste.txt
  //              ../p7/p8/p9/teste1.txt
  //              /teste.txt
  
  token = strtok(filename, "/");
  
  if(filename[0]=='/'){   //CAMINHO ABSOLUTO  regAux <= regDirRaiz
    read_sector(firstDataBlock * superBlock.blockSize, buffer);
    memcpy(&regAux, buffer, sizeof(struct t2fs_record));
    file.fullPathName = filename;
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
/*-----------------------------------------------------------------------------
Função:	Procura um registro VALIDO com nome igual a name, no diretorio apontado por dirReg
Entra:	name -> nome do arquivo a ser procurado.
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
  
  if(dirReg.TypeVal==TYPEVAL_DIRETORIO){    
    inodeAux = getInode(dirReg.inodeNumber);


    for(dirBlocks=0; dirBlocks<inodeAux.blocksFileSize; dirBlocks++){   //percorre blocos
        if(dirBlocks==0)
            rsector = (inodeAux.dataPtr[0] * superBlock->blockSize);
        else if(dirBlocks==1)
            rsector = (inodeAux.dataPtr[1] * superBlock->blockSize);
        else if(dirBlocks<pointersSingleInd){               //INDIRECAO SIMPLES

            if(offsetSingleIndBlock<superblock->blockSize){
                read_sector(((inodeAux.singleIndPtr)*(superBlock->blockSize)) + offsetSingleIndBlock, sindbuffer);
                offsetSingleIndBlock++;

                if(t<(pointersSingleInd/superBlock->blockSize)){
                    memcpy(&pointer, sindbuffer+(t*sizeof(DWORD)), sizeof(DWORD));
                    rsector = (pointer * superBlock->blockSize);
                    t++;
                }
            }

        }else{                                            //INDIRECAO DUPLA

            if(offsetDoubleIndBlock<superBlock->blockSize){
                read_sector(((inodeAux.doubleIndPtr)*(superBlock->blockSize))+offsetDoubleIndBlock, dindbuffer);
                offsetDoubleIndBlock++;

                if(h<(pointersDoubleInd/superBlock->blockSize)){
                    memcpy(&indpointer, dindbuffer + (h*sizeof(DWORD)), sizeof(DWORD));
                    h++;

                    if(j<superblock->blockSize){
                        read_sector(((indpointer)*(superBlock->blockSize)) + j, indbuffer);
                        j++;

                        if(g<(pointersSingleInd/superBlock->blockSize)){
                            memcpy(&pointer, indbuffer+(g*sizeof(DWORD)), sizeof(DWORD));
                            rsector = (pointer * superBlock->blockSize);
                            g++;
                        }
                    }
                }
            }
        }
        for(offsetDirBlock=0; offsetDirBlock<superblock->blockSize; offsetDirBlock++){  //setores dentro do bloco
            read_sector(rsector + offsetDirBlock, buffer);
            do{
                memcpy(&regAux, buffer+(z*sizeof(t2fs_record)), sizeof(t2fs_record));
                bytesread += sizeof(t2fs_record);
                if(strcmp(regAux.name, name)==0){
                  if(regAux.TypeVal == TYPEVAL_REGULAR || regAux.TypeVal == TYPEVAL_DIRETORIO)
                    return regAux;
                }
            }while(bytesread<inodeAux.bytesFileSize);
        }
    }
  }
  
regAux.TypeVal = TYPEVAL_INVALIDO;

return regAux;
  
}

/*---------------------------------------------------------------------------------
Função: Procura o inode do arquivo especificado pelo nome "filename" 
Entrada: filename -> nome do arquivo ou diretório
Saída: Se a operação for realizada com sucesso, retorna a estrutura de um inode(t2fs_inode)
       Em caso de erro retorna a estrutura com o primeiro campo(blocksFileSize) com -1;

-----------------------------------------------------------------------------------*/
t2fs_inode findInode(char *filename){
  /* TO DO*/  
}
/*-----------------------------------------------------------------------------------
Função: Percorre os blocos que estão sendo utilizados pelo inode e libera eles
Entrada: inode -> número do inode do arquivo 
          amount -> Se -1 apaga todos os blocos, se outro valor maior que zero apaga 
          a partir do "cPointer"
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int delBlocks(int inode, int cPointer){
  /*  TO-DO */
}
/*-----------------------------------------------------------------------------------
Função: Libera o registro do diretorio do arquivo
Entrada: dirInode -> número do inode do diretório 
          filename -> nome do arquivo a ser invalidado
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int delRecord(t2fs_inode dirInode, fileHandler fileName){
  /*  TO-DO */
}