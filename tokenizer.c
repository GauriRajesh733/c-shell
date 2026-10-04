#include "tokenizer.h"
#include "stringvector.h"
#include <stdio.h>

// return true if given char is special shell char
static bool isSpecialChar(char c) {
        return c == '(' || c == ')' || c == '<' || c == '>' ||
                c == ';' || c == '|';
}

// return true if given char is whitespace
static bool isWhiteSpace(char c) {
        return c == '\t' || c == ' ';
}

stringVector* tokenize(const char *input) {
	stringVector *tokens = createStringVector(0);
	enum TOKEN_STATE state = SPACE;
	char buffer[1024];
	int bufferIdx = 0;
	int singleQuote = 1; // 0 if doubleQuote

	for (int i = 0; input[i] != '\0'; i++) {
		char c = input[i];

		switch(state) {
			case SPACE:
				// add special character
				if (isSpecialChar(c)) {
					buffer[bufferIdx++] = c;
					buffer[bufferIdx] = '\0';
					pushStringVector(buffer, tokens);
					bufferIdx = 0;
				}
				// begin processing quote
				else if (c == '"') {
					state = QUOTE;
					singleQuote = 0;
				}
				else if (c == '\'') {
					state = QUOTE;
					singleQuote = 1;
				}
				// begin processing word
				else if (!isWhiteSpace(c)) {
					buffer[bufferIdx++] = c;
					state = WORD;
				}

				break;
			case WORD:
				// finish processing word and special character
				if (isSpecialChar(c)) {
                                	buffer[bufferIdx] = '\0';
					pushStringVector(buffer, tokens);
					bufferIdx = 0;

					buffer[bufferIdx++] = c;
                                        buffer[bufferIdx] = '\0';
					pushStringVector(buffer, tokens);
					state = SPACE;
					
					// reset buffer
					bufferIdx = 0;
				}
				// UNIX shell standard: process as continuous string
                                else if (c == '"') {                                                                
					state = QUOTE;                                                              
					singleQuote = 0;                                                    
				}                                                                           
				else if (c == '\'') {                                                                
					state = QUOTE;                                                              
					singleQuote = 1;                                                    
				}  
				// continue processing word
                                else if (!isWhiteSpace(c)) {
					buffer[bufferIdx++] = c;
                                }
				// finish processing word
				else {
					buffer[bufferIdx] = '\0';
                                        pushStringVector(buffer, tokens);
                                        state = SPACE;

                                        // reset buffer
                                        bufferIdx = 0;
				}

				break;
			case QUOTE:
				// finish processing quote
                                if (c == '"' && singleQuote == 0) {
					buffer[bufferIdx] = '\0';
                                        pushStringVector(buffer, tokens);
                                        state = SPACE;

                                        // reset buffer
                                        bufferIdx = 0;
                                }
				if (c == '\'' && singleQuote == 1) {
					buffer[bufferIdx] = '\0';
					pushStringVector(buffer, tokens);
					state = SPACE;

					// reset buffer
					bufferIdx = 0;
				}
				// continue processing quotes
				else {
					buffer[bufferIdx++] = c;
                                }
				break;
		
		}


	}
	
	// add remaining token if there
	if (bufferIdx != 0) {
		buffer[bufferIdx] = '\0';
		pushStringVector(buffer, tokens);
	}

	return tokens;
}

/*
int main() {
	// example inputs
	char *input1 = "echo one";
	char *input2 = "echo one; echo two";
	char *input3 = "echo -e \"1\\n2\\n3\\n4\\n5\\\" > numbers.txt; cat numbers.txt";
	char *input4 = "sort -nr < numbers.txt";
	char *input5 = "shuf -i 1-10 | sort -n | tail -5";
	char *input6 = "    echo one";
	char *input7 = "  rand\"om\"";
	char *input8 = "echo 'this < is > a demo \"This is a sentence\" ; \"some ( special > chars\"' | ./tokenize";

	// tokenize inputs
	stringVector *vector1 = tokenize(input1);
	stringVector *vector2 = tokenize(input2);
	stringVector *vector3 = tokenize(input3);
	stringVector *vector4 = tokenize(input4);
	stringVector *vector5 = tokenize(input5);
	stringVector *vector6 = tokenize(input6);
	stringVector *vector7 = tokenize(input7);
	stringVector *vector8 = tokenize(input8);

	// print tokens
	printf("%s\n", input1);
	printStringVector(vector1);

	printf("%s\n", input2);
        printStringVector(vector2);

	printf("%s\n", input3);
        printStringVector(vector3);

	printf("%s\n", input4);
        printStringVector(vector4);

	printf("%s\n", input5);
        printStringVector(vector5);

	printf("%s\n", input6);
        printStringVector(vector6);

	printf("%s\n", input7);
        printStringVector(vector7);

	printf("%s\n", input8);
        printStringVector(vector8);

	// free string vectors

	return 0;
}*/


