#include "json.h"

static int json_parse_value(const char ** cursor, json_value * parent);

static void skip_whitespace(const char** cursor)
{
	if (**cursor == '\0') return;
	while (iscntrl(**cursor) || isspace(**cursor)) ++(*cursor);
}

static int has_char(const char** cursor, char character)
{
	skip_whitespace(cursor);
	int success = **cursor == character;
	if (success) ++(*cursor);
	return success;
}

static int json_parse_object(const char** cursor, json_value* parent)
{
	// Could refactor this into function json_init_value(type)
	json_value result = { .type = TYPE_OBJECT };
	vector_init(&result.value.object, sizeof(json_value));

	int success = 1;
	while (success && !has_char(cursor, '}')) {
		json_value key = { .type = TYPE_NULL };
		json_value value = { .type = TYPE_NULL };
		if (json_parse_value(cursor, &key));
		success = success && has_char(cursor, ':');
		success = success && json_parse_value(cursor, &value);

		if (success) {
			vector_push_back(&result.value.object, &key);
			vector_push_back(&result.value.object, &value);
		}
		else {
			json_free_value(&key);
			break;
		}
		skip_whitespace(cursor);
		if (has_char(cursor, '}')) break;
		else if (has_char(cursor, ',')) continue;
		else success = 0;
	}

	if (success) {
		*parent = result;
	}
	else {
		json_free_value(&result);
	}

	return success;
	return 1;
}

static int json_parse_array(const char** cursor, json_value* parent)
{
	int success = 1;
	if (**cursor == ']') {
		++(*cursor);
		return success;
	}
	while (success) {
		json_value new_value = { .type = TYPE_NULL };
		success = json_parse_value(cursor, &new_value);
		if (!success) break;
		skip_whitespace(cursor);
		vector_push_back(&parent->value.array, &new_value);
		skip_whitespace(cursor);
		if (has_char(cursor, ']')) break;
		else if (has_char(cursor, ',')) continue;
		else success = 0;
	}
	return success;
}


void json_free_value(json_value* val)
{
	if (!val) return;

	switch (val->type) {
		case TYPE_STRING:
			free(val->value.string);
			val->value.string = NULL;
			break;
		case TYPE_ARRAY:
		case TYPE_OBJECT:
			vector_foreach(&(val->value.array), (void(*)(void*))json_free_value);
			vector_free(&(val->value.array));
			break;
	}

	val->type = TYPE_NULL;
}

int json_is_literal(const char** cursor, const char* literal) {
	size_t cnt = strlen(literal);
	if (strncmp(*cursor, literal, cnt) == 0) {
		*cursor += cnt;
		return 1;
	}
	return 0;
}

static int json_parse_value(const char** cursor, json_value* parent)
{
	// Eat whitespace
	int success = 0;
	skip_whitespace(cursor);
	switch (**cursor) {
		case 0:
			// If parse_value is called with the cursor at the end of the string
			// that's a failure
			success = 0;
			break;
		case '"':
			++*cursor;
			const char* start = *cursor;
			char* end = strchr(*cursor, '"');
			if (end) {
				size_t len = end - start;
				char* new_string = malloc((len + 1) * sizeof(char));
				memcpy(new_string, start, len);
				new_string[len] = '\0';

				assert(len == strlen(new_string));

				parent->type = TYPE_STRING;
				parent->value.string = new_string;

				*cursor = end + 1;
				success = 1;
			}
			break;
		case '{':
			++(*cursor);
			skip_whitespace(cursor);
			success = json_parse_object(cursor, parent);
			break;
		case '[':
			parent->type = TYPE_ARRAY;
			vector_init(&parent->value.array, sizeof(json_value));
			++(*cursor);
			skip_whitespace(cursor);
			success = json_parse_array(cursor, parent);
			if (!success) {
				vector_free(&parent->value.array);
			}
			break;
		case 't': {
			success = json_is_literal(cursor, "true");
			if (success) {
				parent->type = TYPE_BOOL;
				parent->value.boolean = 1;
			}
			break;
		}
		case 'f': {
			success = json_is_literal(cursor, "false");
			if (success) {
				parent->type = TYPE_BOOL;
				parent->value.boolean = 0;
			}
			break;
		}
		case 'n':
			success = json_is_literal(cursor, "null");
			break;
		default: {
			char* end;
			double number = strtod(*cursor, &end);
			if (*cursor != end) {
				parent->type = TYPE_NUMBER;
				parent->value.number = number;
				*cursor = end;
				success = 1;
			}
		}
	}

	return success;
}


int json_parse(const char* input, json_value* result)
{
	return json_parse_value(&input, result);
}

char* json_value_to_string(json_value* value)
{
	assert(value->type == TYPE_STRING);
	return (char *)value->value.string;
}

double json_value_to_double(json_value* value)
{
	assert(value->type == TYPE_NUMBER);
	return value->value.number;
}

int json_value_to_bool(json_value* value)
{
	assert(value->type == TYPE_BOOL);
	return value->value.boolean;
}

vector* json_value_to_array(json_value* value)
{
	assert(value->type == TYPE_ARRAY);
	return &value->value.array;
}

vector* json_value_to_object(json_value* value)
{
	assert(value->type == TYPE_OBJECT);
	return &value->value.object;
}

json_value* json_value_at(const json_value* root, size_t index)
{
	assert(root->type == TYPE_ARRAY);
	if (root->value.array.size < index) {
		return vector_get_checked(&root->value.array,index);
	}
	else {
		return NULL;
	}
}

json_value* json_value_with_key(const json_value* object, const char* key)
{
	assert(object->type == TYPE_OBJECT);
	json_value* data = (json_value*)object->value.object.data;
	size_t size = object->value.object.size;
	for (size_t i = 0; i < size; i += 2)
	{
		if (strcmp(data[i].value.string, key) == 0)
		{
			return &data[i + 1];
		}
	}
	return NULL;
}




#ifdef _DEBUG

const char* test_string = " \
{ \"item1\" : [1, 2, 3, 4], \
  \"item2\" : { \"a\" : 1, \"b\" : 2, \"c\" : 3 }, \
  \"item3\" : \"An Item\" \
}";

void json_test_value_string(void)
{
	printf("json_parse_value_string: ");
	// Normal parse, skip whitespace
	const char* string = "     \n\t\"Hello World!\"";
	json_value result = { .type = TYPE_NULL };
	assert(json_parse_value(&string, &result));
	assert(result.type == TYPE_STRING);
	assert(result.value.string != NULL);
	assert(strlen(result.value.string) == 12);
	assert(strcmp("Hello World!", result.value.string) == 0);

	json_free_value(&result);

	// Empty string
	string = "\"\"";
	json_parse_value(&string, &result);
	assert(result.type == TYPE_STRING);
	assert(result.value.string != NULL);
	assert(strlen(result.value.string) == 0);
	json_free_value(&result);

	printf(" OK\n");
}

void json_test_value_number(void)
{
	printf("json_test_value_number: ");
	const char* string = "  23.4";
	json_value result = { .type = TYPE_NULL };
	assert(json_parse_value(&string, &result));
	assert(result.type == TYPE_NUMBER);
	assert(result.value.number == 23.4);

	json_free_value(&result);
	printf(" OK\n");
}

void json_test_value_invalid(void)
{
	printf("json_test_value_invalid: ");
	{
		// not a valid value
		const char* string = "xxx";
		json_value result = { .type = TYPE_NULL };
		assert(!json_parse_value(&string, &result));
		assert(result.type == TYPE_NULL);
		json_free_value(&result);
	}
	{
		// parse_value at end should fail
		const char* string = "";
		json_value result = { .type = TYPE_NULL };
		assert(!json_parse_value(&string, &result));
		assert(result.type == TYPE_NULL);
		json_free_value(&result);
	}
	printf(" OK\n");
}

void json_test_value_array(void)
{
	printf("json_test_value_array: ");
	{
		// Empty Array
		const char* string = "[]";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.array.data == NULL);
		assert(json_parse_value(&string, &result));
		assert(result.type = TYPE_ARRAY);
		assert(result.value.array.data != NULL);
		assert(result.value.array.size == 0);
		json_free_value(&result);
	}

	{
		// One Element
		const char* string = "[\"Hello World\"]";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.array.data == NULL);
		assert(json_parse_value(&string, &result));
		assert(result.type = TYPE_ARRAY);
		assert(result.value.array.data != NULL);
		assert(result.value.array.size == 1);

		json_value* string_value = (json_value *)result.value.array.data;
		assert(string_value->type == TYPE_STRING);
		assert(strcmp("Hello World", string_value->value.string) == 0);;

		json_free_value(&result);
	}

	{
		// Mutliple Elements
		const char* string = "[0, 1, 2, 3]";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.array.data == NULL);
		assert(json_parse_value(&string, &result));
		assert(result.type = TYPE_ARRAY);
		assert(result.value.array.data != NULL);
		assert(result.value.array.size == 4);

		json_free_value(&result);
	}

	{
		// Failure
		const char* string = "[0, 2,,]";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.array.data == NULL);
		assert(result.type == TYPE_NULL);
		assert(result.value.array.data == NULL);

		json_free_value(&result);
	}
	{
		// Failure
		const char* string = "[0, 2, 0";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.array.data == NULL);
		assert(result.type == TYPE_NULL);
		assert(result.value.array.data == NULL);

		json_free_value(&result);
	}


	printf(" OK\n");
}

void json_test_value_object(void)
{
	printf("json_test_value_object: ");
	{
		// Empty Object
		const char* string = "{}";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.object.data == NULL);
		assert(json_parse_value(&string, &result));
		assert(result.type = TYPE_OBJECT);
		assert(result.value.array.data != NULL);
		assert(result.value.array.size == 0);
		json_free_value(&result);
	}

	{
		// One Pair
		const char* string = "{ \"a\"  :   1  }";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.object.data == NULL);
		assert(json_parse_value(&string, &result));
		assert(result.type = TYPE_OBJECT);
		assert(result.value.array.data != NULL);
		assert(result.value.array.size == 2);

		json_value* members = (json_value *)result.value.object.data;

		assert(strcmp(json_value_to_string(members), "a") == 0);
		++members;
		assert(json_value_to_double(members) == 1.0);
		json_free_value(&result);
	}

	{
		// Multiple Pairs
		const char* string = "{ \"a\": 1, \"b\" : 2, \"c\" : 3 }";
		json_value result = { .type = TYPE_NULL };
		assert(result.value.object.data == NULL);
		assert(json_parse_value(&string, &result));
		assert(result.type = TYPE_OBJECT);
		assert(result.value.array.data != NULL);
		assert(result.value.array.size == 6);

		json_value* members = (json_value *)result.value.object.data;

		assert(strcmp(json_value_to_string(&members[4]), "c") == 0);
		assert(json_value_to_double(&members[5]) == 3.0);
		json_free_value(&result);
	}

	printf(" OK\n");
}

void json_test_value_literal(void) {
	printf("json_test_values_literal: ");
	{
		const char* string = "true";
		json_value result = { .type = TYPE_NULL };
		assert(json_parse_value(&string, &result));
		assert(result.type == TYPE_BOOL);
		assert(result.value.boolean);
		json_free_value(&result);
	}

	{
		const char* string = "false";
		json_value result = { .type = TYPE_NULL };
		assert(json_parse_value(&string, &result));
		assert(result.type == TYPE_BOOL);
		assert(!result.value.boolean);
		json_free_value(&result);
	}

	{
		const char* string = "null";
		json_value result = { .type = TYPE_NULL };
		assert(json_parse_value(&string, &result));
		assert(result.type == TYPE_NULL);
		json_free_value(&result);
	}
	printf(" OK\n");
}

void json_test_coarse(void)
{
	printf("json_test_coarse: ");

	json_value root;
	assert(json_parse(test_string, &root));

	json_value* val = json_value_with_key(&root, "item1");

	assert(root.type == TYPE_OBJECT);
	assert(root.value.object.size == 6);

	assert(val != NULL);
	assert(json_value_to_array(val) != NULL);
	assert(json_value_to_array(val)->size == 4);

	val = json_value_with_key(&root, "item3");
	assert(val != NULL);
	assert(json_value_to_string(val) != NULL);
	assert(strcmp(json_value_to_string(val), "An Item") == 0);

	json_free_value(&root);
	printf(" OK\n");
}


void json_test_all(void)
{
	json_test_value_invalid();
	json_test_value_string();
	json_test_value_number();
	json_test_value_array();
	json_test_value_object();
	json_test_value_literal();
	json_test_coarse();
}



#endif 




