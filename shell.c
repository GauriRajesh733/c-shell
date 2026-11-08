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
void executePipeCommands(pipeVector *pipeCommands, stringVector *prevTokens);
// execute source command 
void executeSourceCommand(char *sourceFile);

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
                        } 

			// execute pipe commands
			if (pipeCommands->size > 0) {
				executePipeCommands(pipeCommands, prevTokens);
			}
			
			// reset pipe commands and current command
			clearPipeVector(pipeCommands);

			// update prev command
			freeStringVector(prevTokens);
			prevTokens = currentCommand;
			currentCommand = createStringVector(0);
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
		executePipeCommands(pipeCommands, prevTokens);
	}
	
	// free pipe commands (includes current command)
	freePipeVector(pipeCommands);
}

bool commandIsBuiltIn(stringVector *tokens) {
	
	if (tokens == NULL || tokens->size == 0) {
		return false;
	}


	return (strcmp(tokens->data[0], "cd") == 0) ||
		(strcmp(tokens->data[0], "source") == 0) ||
		(strcmp(tokens->data[0], "prev") == 0) ||
		(strcmp(tokens->data[0], "help") == 0) ||
		(strcmp(tokens->data[0], "exit") == 0);
}

void executeCommand(stringVector *tokens) {

	if (execvp(tokens->data[0], tokens->data) == -1) {
		fprintf(stderr, "%s: command not found.", tokens->data[0]);
		exit(1);
	}
}

void executeBuiltInCommand(stringVector *tokens, stringVector *prevTokens) {
	char *command = tokens->data[0];

	// cd command
	if (strcmp(command, "cd") == 0 && (tokens->size == 1 || chdir(tokens->data[1]) != 0)) {
		fprintf(stderr, "cd: command failed.\n");
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
	// source command with file
	if (strcmp(command, "source") == 0 && tokens->size > 1) {
		executeSourceCommand(tokens->data[1]);
	}
	// source command without file
	if (strcmp(command, "source") == 0 && tokens->size == 1) {
		fprintf(stderr, "source command: no file provided\n");
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

	// process command again in case of sequencing or piping
	stringVector *prevTokens = createStringVector(0);
        processCommand(tokens, prevTokens);
	freeStringVector(prevTokens);
}

void executeOutputRedirection(stringVector *tokens, char *outputFile) {	
	// no commands
	if (tokens->size == 0) {
		fprintf(stderr, "output redirection: no command provided.");
		return;
	}
	// no output file
	if (strcmp(outputFile, "") == 0) {
		fprintf(stderr, "output redirection: no file provided.");
		return;
	
	}

	
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
        }
}

void executeInputRedirection(stringVector *tokens, char *inputFile) {
	// no commands
        if (tokens->size == 0) {
                fprintf(stderr, "input redirection: no command provided.");
		return;
        }
        // no input file
        if (strcmp(inputFile, "") == 0) {
                fprintf(stderr, "input redirection: no file provided.");
		return;
        }

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
	}
}

void executePipeCommands(pipeVector *pipeCommands, stringVector *prevTokens) {
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
	pid_t *processes = calloc(pipeCommands->size, sizeof(pid_t));

	// iterate through tokens one command at a time in pipeCommands
	for (int i = 0; i < pipeCommands->size; i++) {
		
		if (pipeCommands->commands[i] == NULL) {
			continue;
		}

		stringVector *tokens = pipeCommands->commands[i];
		int builtInCommand = 0;

		// intercept built in command
		if (tokens->size > 0 && commandIsBuiltIn(tokens)) {
			executeBuiltInCommand(tokens, prevTokens);
			builtInCommand = 1;
		}

		int pid = 0;
		
		int readFd = -1;
		int writeFd = -1;

		// pipe if not last command and not built in command
                if (builtInCommand != 1 && i != pipeCommands->size - 1) {
                        if (pipe(fd) != 0) {
                                perror("pipe command failed.");
                                freePipeVector(pipeCommands);
                                exit(EXIT_FAILURE);
                        }

                        // update read and write ends of pipe
                        readFd = fd[0];
                        writeFd = fd[1];
                }
	
		// fork only if not built in command
		if (builtInCommand != 1) {
			pid = fork();
			processes[i] = pid;
		}

		if (pid < 0) {
			freePipeVector(pipeCommands);
			exit(EXIT_FAILURE);
		}

		if (pid == 0 && builtInCommand == 0) {
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
				close(readFd);

				// redirect standard output
				if (dup2(writeFd, STDOUT_FILENO) < 0) {
					perror("pipe command failed.");
					freePipeVector(pipeCommands);
					exit(EXIT_FAILURE);
				}
				
				// close old fd	
				close(writeFd);
			}

			// assemble current command tokens
			stringVector *currentCommand = createStringVector(0);
			
			// iterate through tokens in command
			for (int j = 0; j < tokens->size; j++) {
				char *token = tokens->data[j];

				// output redirection
				if (strcmp(token, ">") == 0 && j == tokens->size - 2) {
					char *outputFile = strdup(tokens->data[j + 1]);
					executeOutputRedirection(currentCommand, outputFile);
					free(outputFile);
					
					// clear command so not ran again
					clearStringVector(currentCommand);

					// exit
					break;
				}
				// input redirection
				else if (strcmp(token, "<") == 0 && j == tokens->size - 2) {
					char *inputFile = strdup(tokens->data[j + 1]);
                        		executeInputRedirection(currentCommand, inputFile);
                        		free(inputFile);

					// clear command so not ran again
                                        clearStringVector(currentCommand);

					// exit
					break;
				}
				// output redirection but no file
				else if (strcmp(token, ">") == 0) {
					fprintf(stderr, "no file provided for output redirection command.\n");
					clearStringVector(currentCommand);
					break;
					
				}
				// input redirection but no file
				else if (strcmp(token, "<") == 0) {
					fprintf(stderr, "no file provided for input redirection command.\n");
					clearStringVector(currentCommand);
					break;
				}

				// add token to current command
				pushStringVector(token, currentCommand);

				// execute built in command
				if (commandIsBuiltIn(currentCommand)) {
					executeBuiltInCommand(currentCommand, prevTokens);
					clearStringVector(currentCommand);
					// exit successfully
					exit(0);
				}
			}
			
			// execute non-empty command
			if (currentCommand->size != 0) {
				executeCommand(currentCommand);
			}


			// free memory if execvp error occured and process not wiped out

			// free current command tokens
			freeStringVector(currentCommand);

			// free child copy of piping commands
			freePipeVector(pipeCommands);

			// free child copy of prevTokens
			freeStringVector(prevTokens);

			// exit child process so next pipe command not processed
			exit(1);
		}

		// if not last command update prev file descriptor and write end of pipe
		if (i != pipeCommands->size - 1) {
			// update fd to read fd of this child process
                	prevFd = readFd;

			// close write end of pipe
			close(writeFd);
		}

	}

	// wait for all child processes to finish
        for (int k = 0; k < pipeCommands->size; k++) {
		// only wait if child process actually created
		if (processes[k] != 0) {
			waitpid(processes[k], NULL, 0);
		}
	}
}

void executeSourceCommand(char *sourceFile) {

        // store tokens of all commands in file
        stringVector* tokens = createStringVector(0);

        // open file
        int fd = open(sourceFile, O_RDONLY);

        // file open failed
        if (fd < 0) {
                perror("source command: file open failed.\n");
                exit(1);
        }

        // convert file descriptor into FILE * stream
        FILE *stream = fdopen(fd, "r");

        if (stream == NULL) {
                perror("source command: file stream failed.\n");
                exit(1);
        }

        // read file line by line
        char *line = NULL;
        size_t len = 0;
        ssize_t nread;

        if ((nread = getline(&line, &len, stream)) != -1) {
                // tokenize current line in file
                tokenize(line, tokens);

                // add ; to separate commands
                pushStringVector(";", tokens);
        }

        // getline allocates memory on heap so free line
        free(line);
        fclose(stream);

        // process and execute commands
        stringVector *prevTokens = createStringVector(0);
        processCommand(tokens, prevTokens);

        // free file tokens
        freeStringVector(prevTokens);
        freeStringVector(tokens);
}
