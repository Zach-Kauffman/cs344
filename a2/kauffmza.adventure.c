#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pthread.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//sorta the same struct
struct Room {
	char name[10];
	int type;
	char connections[6][12]; 
	int cnum;
};

//void* getTime(void* argument) {
//	FILE* timeFile;
//	timeFile = fopen("currenttime.txt", "w");
//	time_t t = time(NULL);
//	struct tm* currentTime;
//	currentTime = localtime(&t);
//	char text[100];
//	strftime(text, 100, "%X, %A, %B %d, %Y", currentTime);
//	fprintf(timeFile, "%s", text);
//	fclose(timeFile);
//	return 0;
//}

char* getDir(char* dirname) {
	
	char* thatgooddir = malloc(40);
	
	time_t age = 0;
	DIR* currentDir = opendir(".");
	struct dirent *dir;
	struct stat fileStat;
	while(dir = readdir(currentDir)) {
		//if the current directory starts with 'kauffmza.rooms'
		if(strstr(dir->d_name, dirname)) {
			memset(&fileStat, 0, sizeof(fileStat));
			//find the most recent dir
			if (stat(dir->d_name, &fileStat)) {
				//makes sure the file isn't bad
			} else if (fileStat.st_mtime > age) {
				age = fileStat.st_mtime;
				strcpy(thatgooddir, dir->d_name);
			}		
		}
	}	
	closedir(currentDir);
	return thatgooddir;
}

void dispRoom(struct Room* room) {
	printf("\nCURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS: ", room->name);
	int ii = 0;
	for(ii = 0; ii < room->cnum - 1; ii ++) {
		printf("%s, ", room->connections[ii]);
	} 
	printf("%s.\nWHERE TO? >", room->connections[ii]);
}

int getInput(struct Room* room) {
	char input[10]; 
	fgets(input, 10, stdin);
	//fgets puts a /n at the end of the input so I remove it
	input[strlen(input) - 1] = 0;
	int ii;
	if(strcmp(input, "time") == 0) {	
		return -2;
	}
	//return the address of the correct connection
	for(ii = 0; ii < room->cnum; ii ++) {
		if(strcmp(input, room->connections[ii]) == 0) {
			return ii;
		}
	}
	return -1;
}

void game(struct Room* rooms[7]) {
	//find the start and end
	int start, end, ii;
	for(ii = 0; ii < 7; ii ++) {
		if(rooms[ii]->type == 1) {
			start = ii;
		} else if (rooms[ii]->type == 3) {
			end = ii;
		}
	}
	int currentRoom = start;
	int hold;
	int steps = 0;
	char name[10];
	int path[50];
	
//	pthread_t timeThread;
//	int threadStatus;
//	//lock the thread before the game starts
//	pthread_mutex_lock(&mutex);
//	threadStatus = pthread_create(&timeThread, NULL, getTime, NULL);
//	assert(threadStatus == 0);

	while(currentRoom != end) {
		dispRoom(rooms[currentRoom]);
		hold = getInput(rooms[currentRoom]);
		if(hold == -1) {
			printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. PLEASE TRY AGAIN.\n\n");
		}// else if (hold == -2) {
		//	pthread_mutex_unlock(&mutex);
		//	threadStatus = pthread_join(timeThread, NULL);
		//	assert(threadStatus == 0);
		//	//print the time
		//	FILE* timeFile;
		//	timeFile = fopen("currenttime.txt", "r");
		//	char c[256];
		//	printf("\n");
		//	while (fgets(c, sizeof(c), timeFile)) {
		//		printf("%s", c);
		//	}	
		//	printf("\n");
		//	fclose(timeFile);
		//	pthread_mutex_lock(&mutex);
		//	threadStatus = pthread_create(&timeThread, NULL, getTime, NULL);
		//}
		else if (hold == -2) {
			printf("Let the record show that I tried my best to add time keeping.\nCheck all the commented code in kauffmza.adventure.c!\n");
		}
		else {
			for(ii = 0; ii < 7; ii ++) {
				if(strcmp(rooms[ii]->name, rooms[currentRoom]->connections[hold]) == 0) {
					currentRoom = ii;
					break;
				}
			}
		}
		path[steps] = currentRoom;
		steps ++;
	}
	printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
	for(ii = 0; ii < steps; ii ++) {
		printf("%s\n", rooms[path[ii]]->name);
	}
}

int main() {
	struct Room* rooms[7];
	char* dir = getDir("kauffmza.rooms");
	
	chdir(dir);	
	DIR* currentDir = opendir(".");
	struct dirent *dir2;
	int ii = -2;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	int jj;
	char* temp = malloc(40);
	//reads each file
	while(dir2 = readdir(currentDir)) {
		
		FILE *currentFile;
		currentFile = fopen(dir2->d_name, "r");	
		//reads each line of the file
		while((read = getline(&line, &len, currentFile)) != -1) {
			if(strstr(line, "NAME")) {
				//malloc the room here since it gets called one time per room
				rooms[ii] = (struct Room*)malloc(sizeof(struct Room));
				rooms[ii]->cnum = 0;
				//string manip to only get the name
				temp = strtok(line, ":");
				temp = strtok(NULL, ":") + 1;
				temp[strlen(temp) -1] = 0;
				strcpy(rooms[ii]->name, temp);
				
			} else if(strstr(line, "CONNECTION")) {
				temp = strtok(line, ":");
				temp = strtok(NULL, ":") + 1;
				temp[strlen(temp) -1] = 0;
				strcpy(rooms[ii]->connections[rooms[ii]->cnum], temp);
				rooms[ii]->cnum ++;
			} else if(strstr(line, "TYPE")) {
				//I don't need to have the string type, I can just store an int and check that later
				if(strstr(line, "START")) {
					rooms[ii]->type = 1;
				} else if (strstr(line, "END")) {
					rooms[ii]->type = 3;
				} else if (strstr(line, "MID")) {
					rooms[ii]->type = 2;
				}
			} 			
		}
		fclose(currentFile);
		ii++;
	}
	closedir(currentDir);
	
	game(rooms);
	
	return 0;
}
