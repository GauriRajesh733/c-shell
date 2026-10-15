#include "stringvector.h"


// pipe vector stores arrays of tokens for pipe commands
typedef struct {
	int size;
	int capacity;
	stringVector **commands;
} pipeVector;

// create pipe vector with given size
pipeVector* createPipeVector(int capacity);

// append given command tokens to given pipe vector
void pushPipeVector(stringVector *tokens, pipeVector *pipeVector);

// clear given pipe vector
void clearPipeVector(pipeVector *pipeVector);

// free data in given pipe vector
void freePipeVector(pipeVector *pipeVector);

// print items in given pipe vector
void printPipeVector(pipeVector *pipeVector);
