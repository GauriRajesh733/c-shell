#ifndef STRING_VECTOR_H
#define STRING_VECTOR_H


// data structure to keep track of string vector size
typedef struct {
	char** data;
	int capacity;
	int size;
} stringVector;

// create string vector given total capacity
stringVector* createStringVector(int capacity);

// append given string element to given string vector
void pushStringVector(const char* str, stringVector *v);

// free allocated memory for given string vector
void freeStringVector(stringVector *v);

// print items in given string vector
void printStringVector(stringVector *v);

#endif
