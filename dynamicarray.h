#pragma once


#include <stdlib.h>


size_t dynamicarray_capacity(void* arr);
size_t dynamicarray_size(void* arr);
size_t dynamicarray_elem_size(void* arr);

void* dynamicarray_create(size_t elem_size);
void dynamicarray_destroy(void* arr);

void dynamicarray_pop(void* arr);

void dynamicarray_erase_range(void* arr, size_t index, size_t count);
void dynamicarray_erase(void* arr, size_t index);

void dynamicarray_clear(void* arr);

#define dynamicarray_push(arr, elem) \
	do{ \
		void** tmp_arr = (void**)&arr; \
		*tmp_arr = _dynamicarray_push((arr), (void*)&(elem)); \
	} while(0)
#define dynamicarray_push_rvalue(arr, elem) \
	do{ \
		auto tmp = (elem); \
		void** tmp_arr = (void**)&arr; \
		*tmp_arr = _dynamicarray_push((arr), (void*)&tmp); \
	} while(0)
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
	do{ \
		auto tmp = (elem); \
		void** tmp_arr = (void**)&arr; \
		*tmp_arr = _dynamicarray_insert((arr), (index), (void*)&tmp); \
	} while(0)


