#pragma once

#include <stdlib.h>


typedef struct hashset_t {
    void** data;
    size_t count;
    size_t cap; // must be power of 2
} hashset_t;


// added = 1, already exists = 0
int hashset_add(hashset_t* hs, void* val);
// removed = 1, not exists = 0
int hashset_remove(hashset_t* hs, void* val);
// exists = 1, not exists = 0
int hashset_in(hashset_t* hs, void* val);
void hashset_clear(hashset_t* hs);
void hashset_destroy(hashset_t* hs);


#ifdef HASHSET_IMPLEMENTATION

#include <string.h>
#include <assert.h>

#define _HS_CHUNK_SIZE (1 << 8)
#define _HS_INIT_CHUNK_COUNT 4

#ifndef HS_HASH_FUNC
//http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
static size_t murmur3(size_t h) {
  h ^= h >> 33;
  h *= 0xff51afd7ed558ccd;
  h ^= h >> 33;
  h *= 0xc4ceb9fe1a85ec53;
  h ^= h >> 33;
  return h;
}
#define HS_HASH_FUNC murmur3 
#endif

//static size_t _hash(size_t val){
//    size_t h = HS_HASH_FUNC(val);
//    return h + (h == 0); // make sure its not 0
//}

static void hashset_expand(hashset_t* hs){
    if(hs == NULL) return;
    size_t cap = hs->cap;
    assert((cap & (cap - 1)) == 0); // assert its power of 2 (1 or 0 bits set total)
    if(hs->cap == 0) {
        hs->cap = _HS_CHUNK_SIZE * _HS_INIT_CHUNK_COUNT;
    } else { 
        hs->cap <<= 1; // *= 2
    }
    hs->data = (void**)realloc(hs->data, hs->cap * sizeof(void*));
    memset(hs->data + cap, 0, (hs->cap - cap) * sizeof(void*)); // because there is no realloc for calloc
}

int hashset_add(hashset_t* hs, void* val){
    if(hs == NULL) return 0;
    if(hs->data == NULL) hashset_expand(hs);
    if(val == NULL){ //special case for 0/nullptr
        if(hs->data[0] != 0)
            return 0;
        hs->data[0] = (void*)1;
        hs->count++;
        return 1;
    }
    
    size_t sv = (size_t)val;
    //size_t hash = _hash(sv);
    size_t hash = HS_HASH_FUNC(sv);
    
    size_t mask = _HS_CHUNK_SIZE - 1;
    size_t i = hash & mask;
    i += (i == 0) * _HS_CHUNK_SIZE; //skip first 0 slot

    while(1){
        if(i >= hs->cap)
            hashset_expand(hs);
        if(hs->data[i] == 0){
            hs->data[i] = (void*)sv;
            hs->count++;
            return 1;
        }
        if(hs->data[i] == (void*)sv)
            return 0;
        i += _HS_CHUNK_SIZE;
    }
    return 0;
}

int hashset_remove(hashset_t* hs, void* val){
    if(hs == NULL || hs->count == 0) return 0;
    if(hs->data == NULL) hashset_expand(hs);
    if(val == NULL){ //special case for 0/nullptr
        if(hs->data[0] == 0)
            return 0;
        hs->data[0] = (void*)0;
        hs->count--;
        return 1;
    }

    size_t sv = (size_t)val;
    //size_t hash = _hash(sv);
    size_t hash = HS_HASH_FUNC(sv);
    
    size_t mask = _HS_CHUNK_SIZE - 1;
    size_t i = hash & mask;
    i += (i == 0) * _HS_CHUNK_SIZE; //skip first 0 slot

    while(1){
        if(i >= hs->cap || hs->data[i] == 0)
            return 0;
        if(hs->data[i] == (void*)sv){
            hs->data[i] = 0;
            hs->count--;
            return 1;
        }
        i += _HS_CHUNK_SIZE;
    }
    return 0;
}

int hashset_in(hashset_t* hs, void* val){
    if(hs == NULL || hs->count == 0) return 0;
    if(hs->data == NULL) hashset_expand(hs);
    if(val == NULL){ //special case for 0/nullptr
        if(hs->data[0] == 0)
            return 0;
        return 1;
    }

    size_t sv = (size_t)val;
    //size_t hash = _hash(sv);
    size_t hash = HS_HASH_FUNC(sv);
    
    size_t mask = _HS_CHUNK_SIZE - 1;
    size_t i = hash & mask;
    i += (i == 0) * _HS_CHUNK_SIZE; //skip first 0 slot

    while(1){
        if(i >= hs->cap || hs->data[i] == 0)
            return 0;
        if(hs->data[i] == (void*)sv)
            return 1;
        i += _HS_CHUNK_SIZE;
    }
    return 0;
}

void hashset_clear(hashset_t* hs){
    if(hs == NULL) return;
    hs->count = 0;
    memset(hs->data, 0, hs->cap * sizeof(void*));
}

void hashset_destroy(hashset_t* hs){
    if(hs == NULL) return;
    if(hs->data)
        free(hs->data);
    hs->count = 0;
    hs->cap = 0;
}


// PS: i have yet to know how hashsets are actually implemented
#endif
