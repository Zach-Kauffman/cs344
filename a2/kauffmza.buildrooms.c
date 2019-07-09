#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

//I use two global ints so that I can actually modify the connections outside the function
int ga = 0;
int gb = 0;
//Room struct so that each room knows its name, type and connections
struct Room {
	char name[8];
	char type[11];
	char connections[6][8];
	int cnum;
};

//edits the values of ga and gb to the address in the rooms[] array for the rooms to be connected
void connect(struct Room rooms[7]) {
	struct Room a;
	struct Room b;
	//code from 2.2
	while (1) {
		ga = rand() % 7;
		if (rooms[ga].cnum < 6) {
			break;
		}
	}
	a = rooms[ga];
	do {
		gb = rand() % 7;
		b = rooms[gb];
	} while(areNotConnected(a,b) == 0 || strcmp(a.name, b.name) == 0 || b.cnum == 6);

	return;
}

//checks if two rooms are connected by comparing string values
int areNotConnected(struct Room a, struct Room b) {
	int ii = 0;
	for(ii = 0; ii < 6; ii ++) {
		if (strcmp(a.connections[ii], b.name) == 0) {
			return 0;
		}
	}
	return 1;
}

int isGraphFull(struct Room rooms[7]) {
	int ii = 0;
	int go = 0;
	for(ii = 0; ii < 7; ii ++) {
		if (rooms[ii].cnum > 2) {
			go ++;
		}
	}
	//if there are 7 rooms with at least 3 connections return 1
	if (go == 7) {
		return 1;
	}
	return 0;
}

void fillDir(struct Room rooms[7], char* dir) {
	chdir(dir);
	int ii = 0;
	int jj = 0;
	//iterate through all rooms and create a file for each of them
	for(ii = 0; ii < 7; ii ++) {
		FILE* roomFile = fopen(rooms[ii].name, "w");	
		fprintf(roomFile, "ROOM NAME: %s\n", rooms[ii].name);
		//iterate through all connections to fill the file
		for(jj = 0; jj < rooms[ii].cnum; jj ++) {
			fprintf(roomFile, "CONNECTION %d: %s\n", jj + 1, rooms[ii].connections[jj]);
		}
	
		fprintf(roomFile, "ROOM TYPE: %s\n", rooms[ii].type);
		fclose(roomFile);
	}
	
}

int main() {
	//make sure I have a different set of random numbers every time
	srand(time(NULL));
	//The first thing I do is create the name of the directory
	int pid = getpid(); //I get the process ID as an integer
	char dir[25];
	sprintf(dir, "kauffmza.rooms.%d", pid);
	mkdir(dir, 0755);
	
	//Next I create all possible room names
	char* names[10] = {"swag\0","floss\0","moldy\0","marge\0","Large\0","saucy\0","dollas\0","ABC\0","XYZ\0","zachary\0"};
	char* types[3] = {"START_ROOM\0", "END_ROOM\0", "MID_ROOM\0"};

	//Array for storing Room structs
	struct Room rooms[7];
	int takenSpots[7] = {88,88,88,88,88,88,88};
	int ii = 0;
	int jj = 0;
	int num = 0;
	int unique = 1;	
	//Fill the array with rooms of random names
	while(jj < 7) {
		num = rand() % 10;
		unique = 1;
		for(ii = 0; ii < 8; ii ++) {
			if (num == takenSpots[ii]) {
				//ensures there are no duplicate names
				unique = 0;
				break;
			}
		}

		if(unique == 1) {
			takenSpots[jj] = num;
			strcpy(rooms[jj].name, names[num]);
			rooms[jj].cnum = 0;
	
			if (jj < 2) {
				//First room is always the start, second is end
				strcpy(rooms[jj].type, types[jj]);
				jj ++;
			} else {
				//everything after is mid
				strcpy(rooms[jj].type, types[2]);
				jj ++;
			}
		}
	}
	//Connect rooms to each other until all rooms have at least 3 connections
	while(isGraphFull(rooms) == 0) {
		connect(rooms);

		strcpy(rooms[ga].connections[rooms[ga].cnum], rooms[gb].name);
		strcpy(rooms[gb].connections[rooms[gb].cnum], rooms[ga].name);

		rooms[ga].cnum ++;
		rooms[gb].cnum ++;
	}
	fillDir(rooms, dir);
return 0;
}

