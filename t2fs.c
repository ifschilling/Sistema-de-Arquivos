/* Arthur Böckmann Grossi (275607), Cassiano Translatti Furlani(278038) e Ian Fischer Schilling(275603)*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/bitmap2.h"
#include "../include/apidisk.h"
/* talvez precise mais includes */

/* Variaveis Globais */
BYTE Buffer[SECTOR_SIZE];




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