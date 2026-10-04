#include "stringvector.h"
#include <stdbool.h>

// DFA states for processing shell input
enum TOKEN_STATE {
	SPACE, // currently outside of token
	WORD, // processing non-special characters
	QUOTE, // processing quote
};

// return string vector of tokens given input
stringVector* tokenize(const char *input);
	
