#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

char* dumpText(char* fname, int len);

int sendText(char*, int, int);

int main(int argc, char *argv[])
{
	//build in variables
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
	
	//my variables
	int enc, key, encLen, keyLen, c;
	FILE *enc2;
	char* finalText;
	char* plainText;
	char* keyText;
	char* data;
    
	if (argc != 4) { fprintf(stderr,"ERR: INVALID NUMBER OF ARGUMENTS\n"); exit(0); } // Check usage & args
	
	//check length
	enc = open(argv[1], O_RDONLY);
	encLen = lseek(enc, 0, SEEK_END);
	key = open(argv[2], O_RDONLY);
	keyLen = lseek(key, 0, SEEK_END);
	
	plainText = malloc(encLen);
	keyText = malloc(keyLen);
	
	if(keyLen < encLen) {
			fprintf(stderr, "ERR: KEY SHORTER THAN TEXT\n");
			exit(1);
	}

	//checks for non alpha/space characters
	//from stackoverflow: https://stackoverflow.com/questions/18109458/read-from-a-text-file-and-parse-lines-into-words-in-c
	enc2 = fopen(argv[1], "r");
	while((c=fgetc(enc2)) != EOF) {
		if(isalpha(c) || isspace(c)) {
			
		} else {
			fprintf(stderr, "ERR: %c IS BAD CHAR IN %s\n", c,argv[1]);
			exit(1);
		}
	}
	fclose(enc2);

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); 								// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; 						// Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); 				// Store the port number
	serverHostInfo = gethostbyname("localhost"); 				// Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");
	
	//build a data string
	//begins with 'E$' to verify encryption
	//Puts '$' after plaintext and key
	//Puts '!' at the end
	data = malloc(encLen + keyLen + 20);
	strcpy(data, "E$");
	plainText = dumpText(argv[1], encLen);
	strcat(data, plainText);	
	strcat(data, "$");	
	keyText = dumpText(argv[2], keyLen);
	strcat(data, keyText);
	strcat(data, "$ ! ");
	
	//use the sendText function to send chunks of bytes at a time until the whole string sends
	sendText(data, strlen(data), socketFD);
	
	//clear buffer
	memset(buffer, '\0', 256);

	//server returns '&' if connection is rejected, otherwise returns encrypted string
	//if it's a good connection, server appends '$' to the end of the string
	finalText = malloc(encLen);
	strcpy(finalText, "");
	while(1){
		c = recv(socketFD, buffer, 255, 0);
		if(c > 0) {
			strcat(finalText, buffer);
			if(finalText[0] == '&') { //check for good connection
				fprintf(stderr, "ERR: ENC CLIENT AUTH ERR\n");
				exit(2);
			}
			if(buffer[c-1] == '$') { //check for end of string
				break;
			}
			memset(buffer, '\0', 256);
		}
	}
	//strip the $ off the end
	char* token = strtok(finalText, "$");
	//get encrypted text and print
	fprintf(stdout, "%s", token);
	close(socketFD); // Close the socket
	return 0;
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

//gets contents of a file
char* dumpText(char* fname, int len) {
	FILE *f;
	char* text = malloc(len);
	f = fopen(fname, "r");
	fgets(text, len, f);
	fclose(f);
	return text;
}
