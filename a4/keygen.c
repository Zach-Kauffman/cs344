#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	srand(time NULL);
	
	char* chars = "QWERTYUIOPASDFGHJKLZXCVBNM ";
	int len = 0;
	char* buffer;
		
	if(argc < 2) {
			fprintf(stderr, "ERR: BAD INPUT\n");
			exit(1);
	}
	
	len = strtoimax(argv[1], &buffer, 10); //convert input string to base 10 int
	
	int i;
	int j;
	for(i = 0; i < len; i ++) {
		j = chars[rand()%27]; //pick a random char from eligiable characters
		fprintf(stdout,"%c", j); //print one char at a time to stdout
	}	
	
	printf("\n");
	
	return 0;
}