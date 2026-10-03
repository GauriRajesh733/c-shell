#include "stringvector.h"
#include <stdbool.h>

// DFA states for processing shell input
enum TOKEN_STATE {
	SPACE, // currently outside of token
	WORD, // processing non-special characters
	QUOTE // processing quote
};

// return true if given char is special shell char
bool isSpecialChar(char c) {
	return c == '(' || c == ')' || c == '<' || c == '>' ||
		c == ';' || c == '|'; 
}

// return true if given char is whitespace
bool isWhiteSpace(char c) {
	return c == '\t' || c == ' ';
}

// return string vector of tokens given input
stringVector* tokenize(const char *input);
	
