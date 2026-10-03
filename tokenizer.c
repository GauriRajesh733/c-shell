#include "tokenizer.h"
#include "stringvector.h"


stringVector* tokenize(const char *input) {
	stringVector *tokens = createStringVector(0);
	enum TOKEN_STATE state = SPACE;
	char buffer[1024];
	int bufferIdx = 0;

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
                                if (c == '"') {
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

int main() {

	return 0;
}








