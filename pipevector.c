#include "stringvector.h"
#include "pipevector.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

pipeVector* createPipeVector(int capacity) {
	// if given negative capacity return NULL
	if (capacity < 0) {
		fprintf(stderr, "Capacity must be greater than or equal to 0.");
		return NULL;
	}
	
	// initialize pipe vector
	pipeVector *v = malloc(sizeof(pipeVector));

	if (v == NULL) {
		perror("Pipe vector allocation failed.");
		return NULL;
	}

	v->capacity = capacity;
	v->size = 0;
	v->commands = calloc(capacity, sizeof(stringVector *));

	// free pipe vector if data allocation fails
	if (v->commands
		       	== NULL) {
		perror("Data array allocation in pipe vector failed.");
		free(v);
		return NULL;
	}

	// return pointer to pipe vector
	return v;
}

void pushPipeVector(stringVector *tokens, pipeVector *pipeVector) {
	// verify given tokens vector is not NULL
	if (tokens == NULL) {
		fprintf(stderr, "Given tokens vector is NULL.");
		return;
	}

	// given pipe vector is not NULL
	if (pipeVector == NULL) {
		fprintf(stderr, "Given pipe vector is NULL.");
		return;
	}

	// if size + 1 >= capacity, double size of array, copy over elements, and free old data
	if (pipeVector->size + 1 >= pipeVector->capacity) {
		pipeVector->capacity *= 2;

		// check if pipe vector capacity has not been set
		if (pipeVector->capacity == 0) {
			pipeVector->capacity = 1;
			stringVector **newCommands = realloc(pipeVector->commands, pipeVector->capacity * sizeof(stringVector *));

			if (newCommands == NULL) {
				perror("Resized commands array allocation in pipe vector failed.");
				perror("Failed to append commands array to pipe vector.");
				return;
			}

			pipeVector->commands = newCommands;
		}
	}

	// append tokens and increment size
	pipeVector->commands[pipeVector->size] = tokens;
	pipeVector->size += 1;

	// null terminate vector
	pipeVector->commands[pipeVector->size] = NULL;
}

void clearPipeVector(pipeVector *pipeVector) {
	// check given pipe vector is not NULL
	if (pipeVector == NULL) {
		fprintf(stderr, "Given pipe vector is NULL.");
		return;
	}	
	
	for (int i = 0; i < pipeVector->size; i++) {
		if (pipeVector->commands[i] != NULL) {
			freeStringVector(pipeVector->commands[i]);
			pipeVector->commands[i] = NULL;
		}
	}

	pipeVector->size = 0;
	pipeVector->commands[0] = NULL;
}

void freePipeVector(pipeVector *pipeVector) {
	// check if pipe vector is NULL
	if (pipeVector == NULL) {
		fprintf(stderr, "Given pipe vector is NULL.");
		return;
	}

	for (int i = 0; i < pipeVector->size; i++) {
		if (pipeVector->commands[i] != NULL) {
			freeStringVector(pipeVector->commands[i]);
		}
	}

	if (pipeVector->commands != NULL) {
		free(pipeVector->commands);
	}

	free(pipeVector);
}


void printPipeVector(pipeVector *pipeVector) {
	if (pipeVector == NULL) {
		return;
	}

	for (int i = 0; i < pipeVector->size; i++) {
		printf("String Vector #%d\n", i);
		if (pipeVector->commands[i] != NULL) {
			printStringVector(pipeVector->commands[i]);
		}
	}
}

/*
int main() {
	// test pipe vector creation
	pipeVector *v = createPipeVector(0);
	printf("Pipe vector capacity: %d\n", v->capacity);
	printf("Pipe vector size: %d\n", v->size);
	// test pipe vector resizing
	stringVector *s1 = createStringVector(1);
	pushStringVector("hi", s1);
	pushStringVector("there", s1);
	pushPipeVector(s1, v);
	printf("Pipe vector capacity: %d\n", v->capacity);
        printf("Pipe vector size: %d\n", v->size);
	printPipeVector(v);
	// test pushing elements 
	stringVector *s2 = createStringVector(2);
	pushStringVector("how", s2);
	pushStringVector("are", s2);
	pushStringVector("you?", s2);
	pushPipeVector(s2, v);
	printf("Pipe vector capacity: %d\n", v->capacity);
        printf("Pipe vector size: %d\n", v->size);
	printPipeVector(v);
	// test clearing pipe vector
	clearPipeVector(v);
	printf("Pipe vector capacity: %d\n", v->capacity);
        printf("Pipe vector size: %d\n", v->size);

	return 0;
}*/
