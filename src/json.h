
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

int json_parse(const char* input, json_value* root);

void json_free_value(json_value* val);

#ifdef _DEBUG
void json_test_all(void);
#endif 
