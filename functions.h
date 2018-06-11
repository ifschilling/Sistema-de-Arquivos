/*************************************************************************

	Funções auxiliares usadas pelo T2FS
	
	Versão: 16.2
	
*************************************************************************/

#ifndef __functions_h__
#define __functions_h__

typedef struct files
{
    char fullPathName[MAX_NAME_FILE_NAME_SIZE+1];
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

struct coordinates{
	unsigned int lastAcessedPosition;
	unsigned int lastReadSector;
	int bytesread;
	struct t2fs_record record;
};

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
Função: Percorre os blocos que estão sendo utilizados pelo inode e libera eles
Entrada:  inode -> número do inode do arquivo 
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int delBlocks(DWORD inode);

/*-----------------------------------------------------------------------------------
Função: Percorre os arquivos abertos e 
Entrada:  inode -> número do inode do arquivo 
Saída: Retorna 0 se for realizada com sucesso, senão retorna um valor negativo
-----------------------------------------------------------------------------------*/
int isFileOpen(DWORD inode);


#endif