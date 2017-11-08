#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "vector.h"

enum json_value_type {
	TYPE_NULL,
	TYPE_BOOL,
	TYPE_NUMBER,
	TYPE_OBJECT, // Is a vector with pairwise entries, key, value
	TYPE_ARRAY, // Is a vector, all entries are plain 
	TYPE_STRING,
	TYPE_KEY
};

typedef struct {
	int type;
	union {
		int boolean;
		double number;
		char* string;
		char* key;
		vector array;
		vector object;
	} value;
} json_value;

void json_free_value(json_value * val);

int json_parse(const char* input, json_value* root);

#ifdef _DEBUG
void json_test_all(void);
#endif 