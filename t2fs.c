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
	/* TO-DO */
}

int delete2 (char *filename){
	/* TO-DO */
}

FILE2 open2 (char *filename){
	/* TO-DO */
}

int close2 (FILE2 handle){
	/* TO-DO */
}

int read2 (FILE2 handle, char *buffer, int size){
	/* TO-DO */
}

int write2 (FILE2 handle, char *buffer, int size){
	/* TO-DO */
}

int truncate2 (FILE2 handle){
	/* TO-DO */
}

int seek2 (FILE2 handle, DWORD offset){
	/* TO-DO */
}

int mkdir2 (char *pathname){
	/* TO-DO */
}

int rmdir2 (char *pathname){
	/* TO-DO */
}

int getcwd2 (char *pathname, int size){
	/* TO-DO */
}

DIR2 opendir2 (char *pathname){
	/* TO-DO */
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){
	/* TO-DO */
}

int closedir2 (DIR2 handle){
	/* TO-DO */
}