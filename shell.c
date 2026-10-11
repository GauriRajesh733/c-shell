#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stringvector.h"
#include "tokenizer.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


// process command given user input
void processCommand(stringVector *tokens, stringVector *prevTokens);
// execute non built in command given tokens
void executeCommand(stringVector *tokens);
// check if given command is built in
bool commandIsBuiltIn(stringVector *tokens);
// execute built in command
void executeBuiltInCommand(stringVector *tokens, stringVector *prevTokens);
// execute prev command
void executePrevCommand(stringVector *tokens);
// execute output redirection
void executeOutputRedirection(stringVector *tokens, char *outputFile);
// execute input redirection
void executeInputRedirection(stringVector *tokens, char *inputFile);


int main() {
	ssize_t read;
	size_t len = 0;
	char *line = NULL;
	stringVector *tokens = createStringVector(0);
	stringVector *prevTokens = createStringVector(0);
	
	while (1) {
	        printf("\n\x1b[32mgauri-shell$\x1b[32m ");
		
		// get user input
		read = getline(&line, &len, stdin);
		line[read - 1] = '\0';	
		
		// exit (ctrl-d)
		if (read == -1) {
			break;
		}

		// tokenize input
		clearStringVector(tokens);
		tokenize(line, tokens);

		// exit
		if (tokens->size == 1 && strcmp(tokens->data[0], "exit") == 0) {
			break;
		}
		
		// process and execute command
		processCommand(tokens, prevTokens);
		
	       	// set previous tokens
		clearStringVector(prevTokens);
		tokenize(line, prevTokens);
	}
	
	// free line buffer
	free(line);

	// free tokens
	freeStringVector(tokens);
	freeStringVector(prevTokens);

	return 0;
}

void processCommand(stringVector *tokens, stringVector *prevTokens) {
	
	// process command(s)
	stringVector *currentCommand = createStringVector(0);

	for (int i = 0; i < tokens->size; i++) {
		
		char *token = tokens->data[i];
		
		// output redirection
		if (strcmp(token, ">") == 0 && i < tokens->size - 1) {
			char *outputFile = strdup(tokens->data[i + 1]);
                	executeOutputRedirection(currentCommand, outputFile);
                	free(outputFile);
			clearStringVector(currentCommand);

			// increment index so file skipped
			i++;
		}
		// input redirection
		else if (strcmp(token, "<") == 0 && i < tokens->size - 1) {
			char *inputFile = strdup(tokens->data[i + 1]);
                        executeInputRedirection(currentCommand, inputFile);
                        free(inputFile);
                        clearStringVector(currentCommand);

			// increment index so file skipped
			i++;
		}
		// add to current command
		else if (strcmp(token, ";") != 0) {
                        pushStringVector(token, currentCommand);
                }
		// skip empty command
		else if (currentCommand->size == 0) {
			continue;
		}
		// execute built in command
		else if (commandIsBuiltIn(currentCommand)) {
			executeBuiltInCommand(currentCommand, prevTokens);
			clearStringVector(currentCommand);
		}
		// execute non-empty command and reset
		else if (currentCommand->size != 0) {
			executeCommand(currentCommand);
			clearStringVector(currentCommand);	
		}
	}

	// execute any remaining commands
	if (currentCommand->size != 0 && commandIsBuiltIn(currentCommand)) {
		executeBuiltInCommand(currentCommand, prevTokens);
	}
	else if (currentCommand->size != 0) {
		executeCommand(currentCommand);
	}

	// free current command tokens
	freeStringVector(currentCommand);
}

bool commandIsBuiltIn(stringVector *tokens) {
	return (strcmp(tokens->data[0], "cd") == 0) ||
		(strcmp(tokens->data[0], "source") == 0) ||
		(strcmp(tokens->data[0], "prev") == 0) ||
		(strcmp(tokens->data[0], "help") == 0) ||
		(strcmp(tokens->data[0], "exit") == 0);
}

void executeCommand(stringVector *tokens) {
	pid_t pid = fork();

        // fork failed
        if (pid < 0) {
                perror("Fork failed to process command.");
        }
        // child process
        else if (pid == 0) {
                if (execvp(tokens->data[0], tokens->data) == -1) {
                        fprintf(stderr, "%s: command not found.", tokens->data[0]);
                 	// free tokens
			freeStringVector(tokens);
		 	exit(EXIT_FAILURE);
                }
        }
        // parent process
        else {
                wait(NULL);
        }

}

void executeBuiltInCommand(stringVector *tokens, stringVector *prevTokens) {
	char *command = tokens->data[0];

	// cd command
	if (strcmp(command, "cd") == 0 && (tokens->size == 1 || chdir(tokens->data[1]) != 0)) {
		fprintf(stderr, "cd: command failed.");
	}
	// prev command
	if (strcmp(command, "prev") == 0) {
		executePrevCommand(prevTokens);
	}
	// help command
	if (strcmp(command, "help") == 0) {
		printf("\n\nshell commands:\n\n");
		printf("cd: change directory to path specified as argument.\n\n");
		printf("source: execute script given filename as argument and each line of file as command.\n\n");
		printf("prev: prints previous command line and executes it again without becoming the new command line.\n\n");
		printf("help: explains all commands available in shell.\n\n");
	}
}

void executePrevCommand(stringVector *tokens) {
	// no previous command to execute
	if (tokens->size == 0) {
		return;
	}

	// print command and then output
	for (int i = 0; i < tokens->size; i++) {
		printf("%s ", tokens->data[i]);
	}
	printf("\n");

	// execute built in command
	if (commandIsBuiltIn(tokens)) {
		stringVector *prevTokens = createStringVector(0);
		executeBuiltInCommand(tokens, prevTokens);
		freeStringVector(prevTokens);
	}
	// execute non built in command
	else {
		executeCommand(tokens);
	}
}

void executeOutputRedirection(stringVector *tokens, char *outputFile) {	
	// no commands
	if (tokens->size == 0) {
		fprintf(stderr, "output redirection: no command provided.");
	}
	// no output file
	if (strcmp(outputFile, "") == 0) {
		fprintf(stderr, "output redirection: no file provided.");
	
	}


	pid_t pid = fork();


	// fork failed
	if (pid < 0) {
		perror("output redirection: fork failed.");
		freeStringVector(tokens);
		free(outputFile);
		exit(1);
	}
	// child process
	else if (pid == 0) {
		int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

		// file open failed
		if (fd < 0) {
			perror("output redirection: file open failed.\n");
			freeStringVector(tokens);
			free(outputFile);
			exit(1);
		}

		if (dup2(fd, STDOUT_FILENO) < 0) {
			perror("output redirection: dup2 failed.\n");
			freeStringVector(tokens);
			free(outputFile);
			exit(1);
		}

		// close original fd since stdout points to file now
		close(fd);

		// execute command
		if (execvp(tokens->data[0], tokens->data) == -1) {          
			fprintf(stderr, "%s: output redirection command not found.", tokens->data[0]);
			// free tokens
			freeStringVector(tokens);
			free(outputFile);			
			exit(EXIT_FAILURE);                                                 
		}

	}
	// parent process
	else {
		wait(NULL);
	}

}

void executeInputRedirection(stringVector *tokens, char *inputFile) {
	// no commands
        if (tokens->size == 0) {
                fprintf(stderr, "input redirection: no command provided.");
        }
        // no input file
        if (strcmp(inputFile, "") == 0) {
                fprintf(stderr, "input redirection: no file provided.");

        }


        pid_t pid = fork();


        // fork failed
        if (pid < 0) {
                perror("input redirection: fork failed.");
                freeStringVector(tokens);
                free(inputFile);
                exit(1);
        }
        // child process
        else if (pid == 0) {
                int fd = open(inputFile, O_RDONLY);

                // file open failed
                if (fd < 0) {
                        perror("input redirection: file open failed.\n");
                        freeStringVector(tokens);
                        free(inputFile);
                        exit(1);
                }

                if (dup2(fd, STDIN_FILENO) < 0) {
                        perror("input redirection: dup2 failed.\n");
                        freeStringVector(tokens);
                        free(inputFile);
                        exit(1);
                }

                // close original fd since stdin points to file now
                close(fd);

                // execute command
                if (execvp(tokens->data[0], tokens->data) == -1) {
                        fprintf(stderr, "%s: input redirection command not found.", tokens->data[0]);
                        // free tokens
                        freeStringVector(tokens);
                        free(inputFile);
                        exit(EXIT_FAILURE);
                }

        }
        // parent process
        else {
                wait(NULL);
        }

}


