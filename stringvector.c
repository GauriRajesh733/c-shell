#include "stringvector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

stringVector* createStringVector(int capacity) {
	// if given non-positive capacity return NULL
	if (capacity < 0) {
		fprintf(stderr, "Capacity must be greater than or equal to 0.");
		return NULL;
	}

       	// otherwise return pointer if successful allocation or NULL if not	
	stringVector *v = malloc(sizeof(stringVector));
	
	if (v == NULL) {
		perror("String vector allocation failed.");
		return NULL;
	}

	// initialize string vector
	v->capacity = capacity;
	v->size = 0;
	v->data = calloc(capacity, sizeof(char *));

	// free vector if data allocation fails
	if (v->data == NULL) {
		perror("Data array allocation in string vector failed.");
		free(v);
		return NULL;
	}
#include <stdio.h>
#include <stdlib.h>
	return v;
}


void pushStringVector(const char *str, stringVector *v) {
	// verify given string vector is not NULL
	if (v == NULL) {
		fprintf(stderr, "Given string vector is NULL");
		return;
	}

	// if size + 1 >= capacity, double size of array, copy over elements, and free old data
	if (v->size >= v->capacity) {
			v->capacity *= 2;
			
			// check if vector capacity has not been set
			if (v->capacity == 0) {
				v->capacity = 1;
			}

			char **newData = realloc(v->data, v->capacity * sizeof(char *));
		
		if (newData == NULL) {
                	perror("Resized data array allocation in string vector failed.");
                	perror("Failed to append string element to string vector.");
			return;
		}

		v->data = newData;
	}

	// append string element and increment size
	v->data[v->size] = strdup(str);
	v->size += 1;

}	

void freeStringVector(stringVector *v) {
	// verify given string vector is not NULL
        if (v == NULL) {
                fprintf(stderr, "Given string vector is NULL");
                return;
        }

	for (int i = 0; i < v->size; i++) {
		if (v->data[i] != NULL) {
			free(v->data[i]);
		}
	}

	if (v->data != NULL) {
		free(v->data);
	}

	free(v);
}

/*
int main() {
	stringVector *v = createStringVector(10);
	
	for (int i = 0; i < 20; i++) {
		pushStringVector("test", v);
		printf("current string vector capacity: %d element i: %s\n", v->capacity, v->data[i]);
	}

	freeStringVector(v);

	stringVector *v2 = createStringVector(0);

        for (int i = 0; i < 10; i++) {
                pushStringVector("test", v2);
                printf("current string vector capacity: %d element i: %s\n", v2->capacity, v2->data[i]);
        }

        freeStringVector(v2);
}*/
