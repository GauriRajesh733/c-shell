#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stringvector.h"
#include "tokenizer.h"
#include <unistd.h>
#include <sys/wait.h>


// process command given user input
void processCommand(char *line);


int main() {
	ssize_t read;
	size_t len = 0;
	char *line = NULL;
	
	while (1) {
	        printf("\n\x1b[32mgauri-shell$\x1b[32m ");

		// get user input
		read = getline(&line, &len, stdin);
		line[read - 1] = '\0';	
		
		// exit loop
		if (read == -1 || strcmp(line, "exit") == 0) {
			printf("\n");
			break;
		}

		// process command
		processCommand(line);
		
	       	// execute command

		// free tokens	
	}
	
	// free line buffer
	free(line);


	return 0;
}

void processCommand(char *line) {
	// TODO: split if sequencing etc.
	
	stringVector *tokens = tokenize(line);
	pid_t pid = fork();

	// fork failed
	if (pid < 0) {
		perror("Fork failed to process command.");
	}
	// child process
	else if (pid == 0) {
		if (execvp(tokens->data[0], tokens->data) == -1) {
			fprintf(stderr, "%s: command not found.", tokens->data[0]);
			exit(EXIT_FAILURE);
		}
	}
	// parent process
	else {
		int status;
		waitpid(pid, &status, 0);
	}
}
