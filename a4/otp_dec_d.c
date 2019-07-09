#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

char* decrypt(char*, char*);

char* getText(char*, int);

int sendText(char*, int, int);

int main(int argc, char *argv[]) {
	
	//server.c variables
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	
	//my variables
	int pid;
	char* plaintext = malloc(70000);
	char* key = malloc(70000);
	char* bigBuffer = malloc(140000);
	char* token;
	int total = 0;
	int ii;
	
	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
	portNumber = atoi(argv[1]); 									// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; 							// Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); 					// Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; 					// Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 				// Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); 										// Flip the socket on - it can now receive up to 5 connections

	while(1) {
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		pid = fork(); //fork after successfully connecting to client
		
		if(pid == 0) { //child process
			
			// Get the message from the client and display it
			memset(buffer, '\0', 256);
			while(1){
				ii = recv(establishedConnectionFD, buffer, 20, 0);
				if(ii > 0){
					strcat(bigBuffer, buffer);
					//printf("\nReceived %d bytes: %s\nLast character: %c\n", ii, buffer, buffer[ii-2]);
					if(buffer[ii-2] == '!') {
						break;
					}
					memset(buffer, '\0', 256);
				}
				if(ii == -1) {
					close(establishedConnectionFD); // Close the existing socket which is connected to the client
					fprintf(stderr, "ERR: BAD RECEIVE\n");
					_Exit(1);
				}
			}
			
			if(bigBuffer[0] != 'D') {
				fprintf(stderr, "ERR: CLIENT NOT OTP_DEC\n");
				send(establishedConnectionFD, "&", 1, 0);
			} else {
				//use strtok to seperate data by $'s into the text and key
				token = strtok(bigBuffer, "$");
				token = strtok(NULL, "$");
				strcpy(plaintext, token);
				token = strtok(NULL, "$");
				strcpy(key, token);

				char* final = decrypt(plaintext, key);
				//send decrypted string back in chunks
				sendText(final, strlen(final), establishedConnectionFD);
			}
			close(establishedConnectionFD); // Close the existing socket which is connected to the client
			_Exit(0); //kill child process
		} else {
			
		}
	}
	close(listenSocketFD); // Close the listening socket
	return 0; 
}

char* decrypt(char* toDec, char* key) {
	int ii;
	int dec;
	char* buffer = malloc(strlen(toDec)); //string to return is same length as string to be decrypted
	for(ii = 0; ii < strlen(toDec); ii ++) {
		//handle spaces in key or text
		if(toDec[ii] == 32) {
			toDec[ii] = 91;
		}
		if(key[ii] == 32) {
			key[ii] = 91;
		}
		//handle endlines
		if(toDec[ii] != '\n') {
			dec = (((toDec[ii] - key[ii]) + 27) % 27); //subtract key from string, add 65 to get correct ascii codes
			//if the char gets converted to what should be a space, turn it back into a space
			if(dec == 26) {
				dec = 32;
			} else {
				dec += 65;
			}	
		}
		buffer[ii] = dec;
	}
	buffer[ii] = '\n';
	buffer[ii+1] = '$';
	//printf("%s\n", buffer);
	return buffer;
}

int sendText(char* fname, int len, int socketFD) {
	//taken from stackoverflow https://stackoverflow.com/questions/14184181/send-function-in-c-how-to-handle-if-not-all-bytes-are-sent
	ssize_t n;
	const char *p = fname;
    while (len > 0) {
        n = send(socketFD, p, len, 0);
        if (n <= 0)
            return -1;
        p += n;
        len -= n;
    }
    return 0;
}
