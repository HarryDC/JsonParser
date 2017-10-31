#pragma once

typedef struct {
    size_t capacity;
    size_t data_size;
    size_t size;
    char* data;
} vector;

void vector_init(vector* v, size_t data_size);

void vector_free(vector* v);

void* vector_get(vector* v, size_t index);

void* vector_get_checked(vector* v, size_t index);

void vector_reserve(vector* v, size_t new_capacity);

void vector_push_back(vector* v, void* data);

void vector_foreach(vector* v, void(*fp)(void*));

void vector_foreach_data(vector* v, int (*fp)(void*, void*), void* data);

#ifdef _DEBUG
void vector_test_all();
#endif