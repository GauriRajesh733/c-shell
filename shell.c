#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stringvector.h"
#include "tokenizer.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "pipevector.h"


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
// execute pipe commands
void executePipeCommands(pipeVector *pipeCommands);

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
	
	// list of all commands
	pipeVector *pipeCommands = createPipeVector(0);

	// list of tokens for current command
	stringVector *currentCommand = createStringVector(0);

	for (int i = 0; i < tokens->size; i++) {
		char *token = tokens->data[i];
		
		// end of pipeline
		if (strcmp(token, ";") == 0) {
			// add current command to pipe and reset current command
                        if (currentCommand->size > 0) {
                                pushPipeVector(currentCommand, pipeCommands);
				currentCommand = createStringVector(0);
                        } 

			// execute pipe commands
			if (pipeCommands->size > 0) {
				executePipeCommands(pipeCommands);
			}
			
			// reset pipe commands and current command
			clearPipeVector(pipeCommands);
		}
		// end of one side of pipe
		else if (strcmp(token, "|") == 0) {
			// add current command to pipe and reset current command
                        if (currentCommand->size > 0) {
                                pushPipeVector(currentCommand, pipeCommands);
				currentCommand = createStringVector(0);
                        }
		}
		// normal token, redirect, or file name
		else {
			// add to current command
			pushStringVector(token, currentCommand);
		}
	}

	// add current command to pipe commands
	if (currentCommand->size > 0) {
		pushPipeVector(currentCommand, pipeCommands);
	}

	// execute pipe commands
	if (pipeCommands->size > 0) {
		executePipeCommands(pipeCommands);
	}
	
	// free pipe commands (includes current command)
	freePipeVector(pipeCommands);
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

void executePipeCommands(pipeVector *pipeCommands) {
	// no pipe command vector
	if (pipeCommands == NULL) {
		fprintf(stderr, "No pipe commands provided to execute.");
		return;
	}
	// no pipe commands to execute
	if (pipeCommands->size == 0) {
		return;
	}
	
	// execute series of pipe commands
	
	// keep track of read and write ends of pipe
	int fd[2];

	// keep track of previous file descriptor to read output from
	int prevFd = -1;

	// keep track of child processes
	pid_t processes[pipeCommands->size];

	// iterate through tokens one command at a time in pipeCommands
	for (int i = 0; i < pipeCommands->size; i++) {
		if (pipeCommands->commands[i] == NULL) {
			continue;
		}
		
		// pipe if not last command
		if (i != pipeCommands->size - 1) {
			if (pipe(fd) != 0) {
				perror("pipe command failed.");
				freePipeVector(pipeCommands);
				exit(EXIT_FAILURE);
			}
		}
		
		stringVector *tokens = pipeCommands->commands[i];
		int read_fd = fd[0];
		int write_fd = fd[1];
		int pid = fork();
		processes[i] = pid;

		if (pid < 0) {
			perror("forking in pipe command failed.");
			freePipeVector(pipeCommands);
			exit(EXIT_FAILURE);
		}

		if (pid == 0) {
			// redirect output of previous process to input of this process
			if (prevFd != -1) {
				if (dup2(prevFd, STDIN_FILENO) < 0) {
					perror("pipe command failed.");
					freePipeVector(pipeCommands);
					exit(EXIT_FAILURE);
				}

				// close old fd
				close(prevFd);

			}

			// redirect output of this process
			if (i != pipeCommands->size - 1) {
				// close read pipe
				close(read_fd);

				// redirect standard output
				if (dup2(write_fd, STDOUT_FILENO) < 0) {
					perror("pipe command failed.");
					freePipeVector(pipeCommands);
					exit(EXIT_FAILURE);
				}
				
				// close old fd	
				close(write_fd)
			}

			// iterate through tokens in command
			for (int i = 0; i < tokens->size; i++) {
				char *token = tokens->data[i];
				
				// output redirection
				if (strcmp(token, ">") == 0 && i < tokens->size - 1) {
					char *outputFile = strdup(tokens->data[i + 1]);
					// TODO: executeOutputRedirection(currentCommand, outputFile);
					free(outputFile);
					
					// increment index so file skipped
					i++;
				}
				// input redirection
				else if (strcmp(token, "<") == 0 && i < tokens->size - 1) {
					char *inputFile = strdup(tokens->data[i + 1]);
                        		// TODO: executeInputRedirection(currentCommand, inputFile);
                        		free(inputFile);

					// increment index so file skipped
					i++;
				}
				// execute built in command
				else if (commandIsBuiltIn(currentCommand)) {
					executeBuiltInCommand(currentCommand, prevTokens);
					clearStringVector(currentCommand);
				}
				// execute non-empty command and reset
				else if (currentCommand->size != 0) {
					// TODO: executeCommand(currentCommand);
					clearStringVector(currentCommand);	
				}
			}

		}

	}
		

	// to do: should helper methods to execute command or redirection free vectors? prob not!
}	
