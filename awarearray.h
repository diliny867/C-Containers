#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct awarearray_t {
    void* data;
    size_t elem_size;
    size_t count;
    size_t cap;
    size_t* availables; // its stack thats stored end to start
} awarearray_t;


#define _AWARR_DEFAULT_CAP  32


#define rtolvalue(val) ((struct { typeof(val) _; }){val})


#define awarr_as(arr, type)  ((type*)(arr)->data)
#define awarr_get(arr, index, type)  (awarr_as(arr, type)[index])
#define awarr_getp(arr, index, type)  (awarr_as(arr, type) + (index))
#define awarr_getraw(arr, index)  ((arr)->data + (index) * (arr)->elem_size)

#define awarr_push(arr, lval)  _awarr_push(arr, (void*)&lval);
#define awarr_push_rval(arr, rval)  _awarr_push(arr, (void*)&rtolvalue(rval));
void _awarr_push(awarearray_t* arr, void* elem);
void awarr_delete(awarearray_t* arr, size_t index);

//awarearray_t* awarr_new(size_t elem_size);
//awarearray_t* awarr_new_cap(size_t elem_size, size_t min_cap);
void awarr_init(awarearray_t* arr, size_t elem_size);
void awarr_init_cap(awarearray_t* arr, size_t elem_size, size_t min_cap);
void awarr_expand(awarearray_t* arr, size_t min_cap);
void awarr_free(awarearray_t* arr);


#ifdef _AWARR_IMPLEMENTATION_

void awarr_expand(awarearray_t* arr, size_t min_cap){
    if(!arr || min_cap < 1) return;
    size_t cap = arr->cap;
    if(cap >= min_cap){
        return;
    }
    size_t availables_index = cap;

    cap = (cap < 1 ? 1 : cap);
    size_t growth = (min_cap - 1) / cap + 1;
    size_t new_cap = growth * cap;

    arr->cap = new_cap;

    arr->data = realloc(arr->data, new_cap * arr->elem_size);
    arr->availables = (size_t*)realloc(arr->availables, new_cap * sizeof(size_t));

    // fill missing available indexes
    for(; availables_index < new_cap; availables_index++){
        arr->availables[availables_index] = availables_index;
    }
}

// awarearray_t* awarr_new(size_t elem_size){
//     return awarr_new_cap(elem_size, _AWARR_DEFAULT_CAP);
// }
// awarearray_t* awarr_new_cap(size_t elem_size, size_t min_cap){
//     awarearray_t* arr = (awarearray_t*)malloc(sizeof(awarearray_t));
//     arr->count = 0;
//     arr->cap = 0;
//     arr->elem_size = elem_size;
//     awarr_expand(arr, min_cap);
//     return arr;
// }

void awarr_init(awarearray_t* arr, size_t elem_size){
    awarr_init_cap(arr, elem_size, _AWARR_DEFAULT_CAP)
}
void awarr_init_cap(awarearray_t* arr, size_t elem_size, size_t min_cap){
    arr->count = 0;
    arr->cap = 0;
    arr->elem_size = elem_size;
    awarr_expand(arr, min_cap);
}

void _awarr_push(awarearray_t* arr, void* elem){
    if(!arr || !elem) return;
    size_t count = arr->count;
    if(count >= arr->cap){
        awarr_expand(arr, arr->cap * 2);
    }
    size_t available_index = count;
    size_t index = arr->availables[available_index]; 

    //memcpy(arr->data + index * arr->elem_size, elem, arr->elem_size);
    memcpy(awarr_getraw(arr, index), elem, arr->elem_size);
    arr->count++;
}

void awarr_delete(awarearray_t* arr, size_t index){
    if(!arr || arr->count <= 0) return;
    arr->count--;
    arr->availables[arr->count] = index;
}

void awarr_free(awarearray_t* arr){
    if(!arr) return;
    if(arr->data)
        free(arr->data);
    if(arr->availables)
        free(arr->availables);
    //free(arr);
}

#endif

