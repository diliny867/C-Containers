#pragma once

#include <stdlib.h>


size_t dynamicarray_capacity(void* arr);
size_t dynamicarray_size(void* arr);
size_t dynamicarray_elem_size(void* arr);

void* dynamicarray_create(size_t elem_size);
void dynamicarray_destroy(void* arr);

void* _dynamicarray_push(void* arr, void* elem);
void dynamicarray_pop(void* arr);

void* _dynamicarray_insert_range(void* arr, size_t index, size_t count, void* range);
void* _dynamicarray_insert(void* arr, size_t index, void* elem);
void dynamicarray_erase_range(void* arr, size_t index, size_t count);
void dynamicarray_erase(void* arr, size_t index);

void dynamicarray_clear(void* arr);

#define rtolvalue(val) ((struct { typeof(val) _; }){val})

#define dynamicarray_push(arr, elem) \
	do{ \
		void** tmp_arr = (void**)&arr; \
		*tmp_arr = _dynamicarray_push((arr), (void*)&(elem)); \
	} while(0)
#define dynamicarray_push_rvalue(arr, elem) \
	dynamicarray_push(arr, rtolvalue(elem))

#define dynamicarray_insert(arr, index, elem) \
	do{ \
		void** tmp_arr = (void**)&arr; \
		*tmp_arr = _dynamicarray_insert((arr), (index), (void*)&(elem)); \
	} while(0)
#define dynamicarray_insert_range(arr, index, count, range) \
	do{ \
		void** tmp_arr = (void**)&arr; \
		*tmp_arr = _dynamicarray_insert_range((arr), (index), (count), (void*)(range)); \
	} while(0)
#define dynamicarray_insert_rvalue(arr, index, elem) \
	dynamicarray_insert(arr, index, rtolvalue(elem))


#ifdef DYNAMICARRAY_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>


#define _DA_CAPACITY_AT 0
#define _DA_SIZE_AT 1
#define _DA_ELEM_SIZE_AT 2
#define _DA_HEADER_SIZE_T_COUNT 3

#define _DA_DEFAULT_CAPACITY 8


static void* _dynamicarray_create(size_t initial_capacity, size_t elem_size){
	size_t* tmp = (size_t*)malloc(sizeof(size_t) * _DA_HEADER_SIZE_T_COUNT + initial_capacity * elem_size);
	tmp[_DA_CAPACITY_AT] = initial_capacity;
	tmp[_DA_SIZE_AT] = 0;
	tmp[_DA_ELEM_SIZE_AT] = elem_size;
	return tmp + _DA_HEADER_SIZE_T_COUNT;
}

static size_t* _dynamicarray_header(void* arr) {
	return ((size_t*)arr) - _DA_HEADER_SIZE_T_COUNT;
}
size_t dynamicarray_capacity(void* arr) {
	return _dynamicarray_header(arr)[_DA_CAPACITY_AT];
}
size_t dynamicarray_size(void* arr) {
	return _dynamicarray_header(arr)[_DA_SIZE_AT];
}
size_t dynamicarray_elem_size(void* arr) {
	return _dynamicarray_header(arr)[_DA_ELEM_SIZE_AT];
}

void dynamicarray_destroy(void* arr){
	free(_dynamicarray_header(arr));
}

void* dynamicarray_create(size_t elem_size){
	return _dynamicarray_create(_DA_DEFAULT_CAPACITY, elem_size);
}

static void* _dynamicarray_new_copy_double_capacity(void* arr) {
	void* tmp = _dynamicarray_create(dynamicarray_capacity(arr) * 2, dynamicarray_elem_size(arr));
	_dynamicarray_header(tmp)[_DA_SIZE_AT] = dynamicarray_size(arr);
	memcpy(tmp, arr, dynamicarray_size(arr) * dynamicarray_elem_size(arr));
	dynamicarray_destroy(arr);
	return tmp;
}

void* _dynamicarray_push(void* arr, void* elem) {
	if(dynamicarray_capacity(arr) <= dynamicarray_size(arr)) {
		arr = _dynamicarray_new_copy_double_capacity(arr);
	}
	memcpy((char*)arr + dynamicarray_size(arr) * dynamicarray_elem_size(arr), elem, dynamicarray_elem_size(arr));
	_dynamicarray_header(arr)[_DA_SIZE_AT]++;
	return arr;
}

void* _dynamicarray_insert_range(void* arr, size_t index, size_t count, void* range) {
	while(dynamicarray_capacity(arr) <= dynamicarray_size(arr) + count - 1) { // it do be like that sometimes
		arr = _dynamicarray_new_copy_double_capacity(arr);
	}
	size_t elem_size = dynamicarray_elem_size(arr);
	memmove((char*)arr + (index + count) * elem_size, ((char*)arr) + index * elem_size, (dynamicarray_size(arr) - index) * elem_size);
	memcpy((char*)arr + index * elem_size, range, count * elem_size);
	_dynamicarray_header(arr)[_DA_SIZE_AT]++;
	return arr;
}
void* _dynamicarray_insert(void* arr, size_t index, void* elem) {
	return _dynamicarray_insert_range(arr, index, 1, elem);
}

void dynamicarray_pop(void* arr) {
	_dynamicarray_header(arr)[_DA_SIZE_AT]--;
}

void dynamicarray_erase_range(void* arr, size_t index, size_t count) {
	size_t elem_size = dynamicarray_elem_size(arr);
	memmove((char*)arr + index * elem_size, ((char*)arr) + (index + count) * elem_size, (dynamicarray_size(arr) - index - count) * elem_size);
	_dynamicarray_header(arr)[_DA_SIZE_AT]--;
}
void dynamicarray_erase(void* arr, size_t index) {
	dynamicarray_erase_range(arr, index, 1);
}

void dynamicarray_clear(void* arr) {
	_dynamicarray_header(arr)[_DA_SIZE_AT] = 0;
}

#endif
