#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

char* promptUser();

void doStatus(int status);

void catchSIGINT(int status);

void catchSIGTSTP(int status);

void checkChildren(int *status);

int bgtoggle = 0;

int main() {
	char* input = NULL;
	
	char* args[512];
	char* inputf = NULL;
	char* outputf = NULL;
	char* token = NULL;
	char* s = " \n";
	
	int counter = 0;
	int status = 0;
	int file1, file2;
	int pid;	
	int fg;
	char* thispid = NULL;

	struct sigaction sigstop;
	struct sigaction siginter;
	

	//catches ctrl-c
	sigstop.sa_handler = catchSIGTSTP;
	sigstop.sa_flags = 0;
	sigfillset(&(sigstop.sa_mask));
	if(sigaction(SIGTSTP, &sigstop, NULL) == -1) {
		printf("ERR: SIGSTP HANDLER\n");
		fflush(stdout);
	}	
	//catches ctrl-z
	siginter.sa_handler = catchSIGINT;
	sigfillset(&(siginter.sa_mask));
	if(sigaction(SIGINT, &siginter, NULL) == -1) {
		printf("ERR: SIGINT HANDLER\n");
		fflush(stdout);
	}

	while(1) {
		fg = 1;
		counter = 0;
		input = promptUser();
		token = strtok(input, s);
			
		//parse user input for args and destinations
		while(token != NULL) {
			//input
			if(strcmp(token, "<") == 0) {
				token = strtok(NULL, s);
				inputf = strdup(token);
			//output
			} else if(strcmp(token, ">") == 0) {
				token = strtok(NULL, s);
				outputf = strdup(token);
			//background toggle
			} else if(strcmp(token, "&") == 0 && bgtoggle == 0) {	
				fg = 0;
				break;
			//args
			} else {
				args[counter++] = strdup(token);
			}
			token = strtok(NULL, s);
		}
		
		//get PID if user inputs $$
		int ii;			
		for(ii = 0; ii < counter; ii ++) {
			if(strcmp(args[ii], "$$") == 0) {
				sprintf(args[ii], "%d", getpid());
			}
		}

		//handle no input
		if(args[0] == NULL) {
			printf("ERR: NO	ARGS\n");
			fflush(stdout);
		}
		//handle cd 
		else if(strcmp(args[0], "cd") == 0) {
			if(args[1] == NULL) {
				chdir(getenv("HOME"));
			} else {
				chdir(args[1]);		
			}
		}
		//handle status
		else if(strcmp(args[0], "status") == 0) {
			doStatus(status);
		}
		//handles exit
		else if(strcmp(args[0], "exit") == 0) {
			exit(0);
		}
		//handles everything else
		else {
			pid = fork();
			//fork returns 0 for child, positive number for parent
			if(pid ==  0) { //this code is only executed by the child process
				if(fg) {
					siginter.sa_handler = SIG_DFL;
					if(sigaction(SIGINT, &siginter, NULL) == -1) {
						printf("ERR: FG PROCESS SIGINT\n");
						fflush(stdout);
						_Exit(1);
					}
				}

				if(inputf != NULL) {
					//check if input file is readable
					file1 = open(inputf, O_RDONLY, 0666);		
					if(file1 == -1) {
						printf("ERR: INPUT '%s' UNREADABLE\n", inputf);
						fflush(stdout);
						_Exit(1);
					}
					//set stdin to input file
					if(dup2(file1, 0) == -1) {
						printf("ERR: DUP2 FAILED ON %s\n", inputf);
						fflush(stdout);
						_Exit(1);
					}
					close(file1);
				} else if(!fg) {
					//if it's a background task with no input, redirect stdin to /dev/null
					file1 = open("/dev/null", O_RDONLY);
					dup2(file1, 0);
				}
				if(outputf != NULL) {
					//check if output file is readable
					file2 = open(outputf, O_WRONLY|O_CREAT|O_TRUNC, 0740);		
					if(file2 == -1) {
						printf("ERR: OUTPUT '%s' UNREADABLE\n", outputf);
						fflush(stdout);
						_Exit(1);
					}
					//set stdout to output file
					if(dup2(file2, 1) == -1) {
						printf("ERR: DUP2 FAILED ON %s\n", outputf);
						fflush(stdout);
						_Exit(1);
					}
					close(file2);
				} else if(!fg) {
					//if it's a background task with no output, redirect stdout to /dev/null
					file2 = open("/dev/null", O_RDONLY);
					dup2(file2, 1);
				}
				//check if command is valid
				if(execvp(args[0], args) == -1) {
					printf("ERR: INVALID COMMAND '%s'\n", args[0]);
					fflush(stdout);
					_Exit(1);
				}
			} else if (pid > 0) { //this code is only executed by the parent process
				if(fg) {
					//If it's a foreground task, wait for the child to finish
					waitpid(pid, &status, 0);
				} else {
					//If it's a background task, print the PID
					printf("Background PID: %d\n", pid);
					fflush(stdout);
				}
			} else { //this code only runs if fork() fails
				status = 1;
			}
		}
		//clear args out
		for(counter = 0; args[counter] != NULL; counter ++) {
			args[counter] = NULL;
		}
		//reset inputs and outputs
		inputf = NULL;
		outputf = NULL;
		//check if any background tasks have completed 
		checkChildren(&status);
		

	}
	return 0;
}


char* promptUser() {
	char* input = malloc(2049);
	printf(": ");
	fflush(stdout);
	if(fgets(input, 2049, stdin) == NULL) {
		return NULL;
	}
	//handles comment lines and no input
	if(input[0] == '#' || strcmp(input, "\n") == 0) {
		input = promptUser();
	}
	return input;
}

void doStatus(int status) {
	if(WIFEXITED(status)) { //if the task exited normally
		printf("exit status %d\n", status);
	} else { //if the task did not exit normally (terminated)
		printf("terminated by signal %d\n", status);
	}
	fflush(stdout);
}

void catchSIGINT(int status) {
	char* msg = "\nCaught SIGINT\n";
	write(STDOUT_FILENO, msg, 15);
}

void catchSIGTSTP(int status) {
	if(bgtoggle == 0) {
		char* msg = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, msg, 51);
		bgtoggle = 1;	
	} else {
		char* msg = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, msg, 31);
		bgtoggle = 0;
	}
}

void checkChildren(int *status) {
	//check for any processes that have completed
	int pid = waitpid(-1, status, WNOHANG);
	while(pid > 0) {
		printf("Background process %d complete: ", pid);
		doStatus(*status);
		pid = waitpid(-1, status, WNOHANG);
	}
}
