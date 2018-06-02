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
	int currentPointer; // bytes
	int handle;
	int size; // bytes
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


/* AUXILIAR FUNCTIONS' DECLARATION */
int setup();
struct *t2fs_inode findInode(DWORD inumber);
fileHandler findFileByName(char *filename);


/* MAIN FUNCTIONS */

/*-----------------------------------------------------------------------------
Função: Usada para identificar os desenvolvedores do T2FS.
	Essa função copia um string de identificação para o ponteiro indicado por "name".
	Essa cópia não pode exceder o tamanho do buffer, informado pelo parâmetro "size".
	O string deve ser formado apenas por caracteres ASCII (Valores entre 0x20 e 0x7A) e terminado por ‘\0’.
	O string deve conter o nome e número do cartão dos participantes do grupo.

Entra:	name -> buffer onde colocar o string de identificação.
	size -> tamanho do buffer "name" (número máximo de bytes a serem copiados).

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
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

Entra:	filename -> nome do arquivo a ser criado.

Saída:	Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo).
	Em caso de erro, deve ser retornado um valor negativo.
-----------------------------------------------------------------------------*/
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
// (BLOCOS DO ARQUIVO * TAMANHO DO SETOR) / SETORES P BLOCO


/*-----------------------------------------------------------------------------
Função:	Apagar um arquivo do disco.
	O nome do arquivo a ser apagado é aquele informado pelo parâmetro "filename".

Entra:	filename -> nome do arquivo a ser apagado.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int delete2 (char *filename){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Abre um arquivo existente no disco.
	O nome desse novo arquivo é aquele informado pelo parâmetro "filename".
	Ao abrir um arquivo, o contador de posição do arquivo (current pointer) deve ser colocado na posição zero.
	A função deve retornar o identificador (handle) do arquivo.
	Esse handle será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do arquivo criado.
	Todos os arquivos abertos por esta chamada são abertos em leitura e em escrita.
	O ponto em que a leitura, ou escrita, será realizada é fornecido pelo valor current_pointer (ver função seek2).

Entra:	filename -> nome do arquivo a ser apagado.

Saída:	Se a operação foi realizada com sucesso, a função retorna o handle do arquivo (número positivo)
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
	
	file.handle = i;
	openedFiles[i] = file;
	return i;
}


/*-----------------------------------------------------------------------------
Função:	Fecha o arquivo identificado pelo parâmetro "handle".

Entra:	handle -> identificador do arquivo a ser fechado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
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
	file->currentPointer = 0;
	file->size = 0;
	return 0;
}


/*-----------------------------------------------------------------------------
Função:	Realiza a leitura de "size" bytes do arquivo identificado por "handle".
	Os bytes lidos são colocados na área apontada por "buffer".
	Após a leitura, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último lido.

Entra:	handle -> identificador do arquivo a ser lido
	buffer -> buffer onde colocar os bytes lidos do arquivo
	size -> número de bytes a serem lidos

Saída:	Se a operação foi realizada com sucesso, a função retorna o número de bytes lidos.
	Se o valor retornado for menor do que "size", então o contador de posição atingiu o final do arquivo.
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Realiza a escrita de "size" bytes no arquivo identificado por "handle".
	Os bytes a serem escritos estão na área apontada por "buffer".
	Após a escrita, o contador de posição (current pointer) deve ser ajustado para o byte seguinte ao último escrito.

Entra:	handle -> identificador do arquivo a ser escrito
	buffer -> buffer de onde pegar os bytes a serem escritos no arquivo
	size -> número de bytes a serem escritos

Saída:	Se a operação foi realizada com sucesso, a função retorna o número de bytes efetivamente escritos.
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Função usada para truncar um arquivo.
	Remove do arquivo todos os bytes a partir da posição atual do contador de posição (CP)
	Todos os bytes a partir da posição CP (inclusive) serão removidos do arquivo.
	Após a operação, o arquivo deverá contar com CP bytes e o ponteiro estará no final do arquivo

Entra:	handle -> identificador do arquivo a ser truncado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int truncate2 (FILE2 handle){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Reposiciona o contador de posições (current pointer) do arquivo identificado por "handle".
	A nova posição é determinada pelo parâmetro "offset".
	O parâmetro "offset" corresponde ao deslocamento, em bytes, contados a partir do início do arquivo.
	Se o valor de "offset" for "-1", o current_pointer deverá ser posicionado no byte seguinte ao final do arquivo,
		Isso é útil para permitir que novos dados sejam adicionados no final de um arquivo já existente.

Entra:	handle -> identificador do arquivo a ser escrito
	offset -> deslocamento, em bytes, onde posicionar o "current pointer".

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int seek2 (FILE2 handle, DWORD offset){
	fileHandler *file;
	file = &(openedFiles[handle]);

	if (offset > file->size) {
		printf("ERROR: offset greater than filesize!\n");
		return 1;
	}

	if(offset == -1){
		file->currentPointer = file->size;
	}
	else {
		file->currentPointer = offset;
	}

	return 0;
}


/*-----------------------------------------------------------------------------
Função:	Criar um novo diretório.
	O caminho desse novo diretório é aquele informado pelo parâmetro "pathname".
		O caminho pode ser ser absoluto ou relativo.
	São considerados erros de criação quaisquer situações em que o diretório não possa ser criado.
		Isso inclui a existência de um arquivo ou diretório com o mesmo "pathname".

Entra:	pathname -> caminho do diretório a ser criado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int mkdir2 (char *pathname){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Apagar um subdiretório do disco.
	O caminho do diretório a ser apagado é aquele informado pelo parâmetro "pathname".
	São considerados erros quaisquer situações que impeçam a operação.
		Isso inclui:
			(a) o diretório a ser removido não está vazio;
			(b) "pathname" não existente;
			(c) algum dos componentes do "pathname" não existe (caminho inválido);
			(d) o "pathname" indicado não é um arquivo;

Entra:	pathname -> caminho do diretório a ser criado

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int rmdir2 (char *pathname){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Altera o diretório atual de trabalho (working directory).
		O caminho desse diretório é informado no parâmetro "pathname".
		São considerados erros:
			(a) qualquer situação que impeça a realização da operação
			(b) não existência do "pathname" informado.

Entra:	pathname -> caminho do novo diretório de trabalho.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
		Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int chdir2 (char *pathname){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Informa o diretório atual de trabalho.
		O "pathname" do diretório de trabalho deve ser copiado para o buffer indicado por "pathname".
			Essa cópia não pode exceder o tamanho do buffer, informado pelo parâmetro "size".
		São considerados erros:
			(a) quaisquer situações que impeçam a realização da operação
			(b) espaço insuficiente no buffer "pathname", cujo tamanho está informado por "size".

Entra:	pathname -> buffer para onde copiar o pathname do diretório de trabalho
		size -> tamanho do buffer pathname

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
		Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getcwd2 (char *pathname, int size){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Abre um diretório existente no disco.
	O caminho desse diretório é aquele informado pelo parâmetro "pathname".
	Se a operação foi realizada com sucesso, a função:
		(a) deve retornar o identificador (handle) do diretório
		(b) deve posicionar o ponteiro de entradas (current entry) na primeira posição válida do diretório "pathname".
	O handle retornado será usado em chamadas posteriores do sistema de arquivo para fins de manipulação do diretório.

Entra:	pathname -> caminho do diretório a ser aberto

Saída:	Se a operação foi realizada com sucesso, a função retorna o identificador do diretório (handle).
	Em caso de erro, será retornado um valor negativo.
-----------------------------------------------------------------------------*/
DIR2 opendir2 (char *pathname){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Realiza a leitura das entradas do diretório identificado por "handle".
	A cada chamada da função é lida a entrada seguinte do diretório representado pelo identificador "handle".
	Algumas das informações dessas entradas devem ser colocadas no parâmetro "dentry".
	Após realizada a leitura de uma entrada, o ponteiro de entradas (current entry) deve ser ajustado para a próxima entrada válida, seguinte à última lida.
	São considerados erros:
		(a) qualquer situação que impeça a realização da operação
		(b) término das entradas válidas do diretório identificado por "handle".

Entra:	handle -> identificador do diretório cujas entradas deseja-se ler.
	dentry -> estrutura de dados onde a função coloca as informações da entrada lida.

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero ( e "dentry" não será válido)
-----------------------------------------------------------------------------*/
int readdir2 (DIR2 handle, DIRENT2 *dentry){
	/* TO-DO */
}


/*-----------------------------------------------------------------------------
Função:	Fecha o diretório identificado pelo parâmetro "handle".

Entra:	handle -> identificador do diretório que se deseja fechar (encerrar a operação).

Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
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

/*-----------------------------------------------------------------------------
Função:	Procura o arquivo a partir do nome absoluto ou relativo "filename".
		Preenche "fullPathName", "type" e "size".

Entra:	filename -> nome do arquivo a ser procurado.

Saída:	Se a operação foi realizada com sucesso, a função retorna o fileHandler do arquivo.
			type = TYPEVAL_REGULAR;
		Em caso de erro, será retornado com type = TYPEVAL_INVALIDO.
-----------------------------------------------------------------------------*/
fileHandler findFileByName(char *filename){
	/* TO-DO */
}
