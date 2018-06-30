#include <stdio.h>
#include "../include/t2fs.h"


int main(){
	int hand = create2("oi");
	char buf1[256]={"TESTE"}, buf2[256];
	write2(hand, buf1, 20);
	read2(hand, buf2, 20);
	
	printf("%s\n", buf2);
	return 0;
}
