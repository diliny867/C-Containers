#pragma once

#include <stdlib.h>


typedef struct hashset_t {
    void** data;
    size_t count;
    size_t cap; // must be power of 2
} hashset_t;

typedef struct hs_iter_t {
    void* value;
    hashset_t* _hs;
    size_t _index;
} hs_iter_t;

// use if malloced, else not needed with hashset_t hs = {0};
void hashset_init(hashset_t* hs);
// added = 1, already exists = 0
int hashset_add(hashset_t* hs, void* val);
// removed = 1, not exists = 0
int hashset_remove(hashset_t* hs, void* val);
// exists = 1, not exists = 0
int hashset_has(hashset_t* hs, void* val);
void hashset_clear(hashset_t* hs);
void hashset_destroy(hashset_t* hs);

#define hashset_addi(hs, num)    hashset_add(hs, (void*)num)
#define hashset_removei(hs, num) hashset_remove(hs, (void*)num)
#define hashset_hasi(hs, num)    hashset_has(hs, (void*)num)


#ifdef HASHSET_IMPLEMENTATION

#include <string.h>

#define _SOME_PRIME 342049
#define _HS_INIT_CAP (1 << 8)
#define _HS_GROW_THRESHOLD 0.8

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

void hashset_init(hashset_t* hs){
    hs->data = 0;
    hs->count = 0;
    hs->cap = 0;
}

static int _hashset_add_unchecked(hashset_t* hs, void* val){
    size_t sv = (size_t)val;
    size_t hash = HS_HASH_FUNC(sv);
    
    size_t mask = hs->cap - 1;
    size_t i = hash & mask;

    while(1){
        if(hs->data[i] == NULL){
            hs->data[i] = val;
            hs->count++;
            return 1;
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }
    return 0;
}

static inline void hashset_expand(hashset_t* hs){
    size_t cap = hs->cap;
    //assert((cap & (cap - 1)) == 0); // assert its power of 2 (1 or 0 bits set total)
    if(cap == 0) {
        hs->cap = _HS_INIT_CAP;
    } else { 
        hs->cap <<= 1; // *= 2
    }
    void** old = hs->data;
    hs->data = (void**)calloc(hs->cap + 1, sizeof(void*));
    if(old == NULL) return;
    size_t count = hs->count;
    hs->count = 0;
    void** oldit = old;
    if(oldit[cap] != NULL){ //special case for 0/NULL
        hs->data[hs->cap] = oldit[cap];
        hs->count++;
        count--;
    }
    while(count){ // iterate until we find all values, so to not iterate tail of NULLs
        if(*oldit != NULL){
            _hashset_add_unchecked(hs, *oldit); //can use unchecked as we know values wont repeat and we already handled 0/NULL case
            count--;
        }
        oldit++;
    }
    free(old);
}

static inline void hashset_maybe_expand(hashset_t* hs){
    if(hs->count < hs->cap * _HS_GROW_THRESHOLD) return; // works for hm = {0}, as if wont return if cap is 0
    hashset_expand(hs);
}

int hashset_add(hashset_t* hs, void* val){
    if(hs == NULL) return 0;
    hashset_maybe_expand(hs);
    if(val == NULL){ //special case for 0/NULL
        if(hs->data[hs->cap] != NULL)
            return 0;
        hs->data[hs->cap] = (void*)1;
        hs->count++;
        return 1;
    }
    
    size_t sv = (size_t)val;
    size_t hash = HS_HASH_FUNC(sv);
    
    size_t mask = hs->cap - 1;
    size_t i = hash & mask;

    while(1){
        if(hs->data[i] == NULL){
            hs->data[i] = val;
            hs->count++;
            return 1;
        }
        if(hs->data[i] == val)
            return 0;
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }
    return 0;
}

// shifts last collision element, so when finding them there were no holes between elements of same hash index
static void _hashset_group(hashset_t* hs, size_t index, size_t hash){
    size_t ihash;
    size_t mask = hs->cap - 1;
    size_t istart = hash & mask;
    size_t i = (index + _SOME_PRIME) & mask;
    size_t ilast = index;
    while(1){
        void* val = hs->data[i];
        if(val == NULL){ //stop when encountering a hole
            hs->data[index] = hs->data[ilast];
            hs->data[ilast] = 0;
            return;
        }
        ihash = HS_HASH_FUNC((size_t)val);
        if((ihash & mask) == istart){
            ilast = i;
        }
        i = (i + _SOME_PRIME) & mask;
    }
}

int hashset_remove(hashset_t* hs, void* val){
    if(hs == NULL || hs->data == NULL || hs->count == 0) return 0;
    //hashset_maybe_expand(hs);
    if(val == NULL){ //special case for 0/NULL
        if(hs->data[hs->cap] == NULL)
            return 0;
        hs->data[hs->cap] = NULL;
        hs->count--;
        return 1;
    }

    size_t sv = (size_t)val;
    size_t hash = HS_HASH_FUNC(sv);
    
    size_t mask = hs->cap - 1;
    size_t i = hash & mask;

    while(1){
        if(hs->data[i] == NULL)
            return 0;
        if(hs->data[i] == val){
            hs->data[i] = 0;
            hs->count--;
            _hashset_group(hs, i, hash);
            return 1;
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }
    return 0;
}

int hashset_has(hashset_t* hs, void* val){
    if(hs == NULL || hs->data == NULL || hs->count == 0) return 0;
    //hashset_maybe_expand(hs);
    if(val == NULL){ //special case for 0/NULL
        if(hs->data[hs->cap] == NULL)
            return 0;
        return 1;
    }

    size_t sv = (size_t)val;
    size_t hash = HS_HASH_FUNC(sv);
    
    size_t mask = hs->cap - 1;
    size_t i = hash & mask;

    while(1){
        if(hs->data[i] == 0)
            return 0;
        if(hs->data[i] == val)
            return 1;
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }
    return 0;
}

void hashset_clear(hashset_t* hs){
    if(hs == NULL) return;
    if(hs->data)
        memset(hs->data, 0, (hs->cap + 1) * sizeof(void*));
    hs->count = 0;
}

void hashset_destroy(hashset_t* hs){
    if(hs == NULL) return;
    if(hs->data)
        free(hs->data);
    hs->data = 0;
    hs->count = 0;
    hs->cap = 0;
}


hs_iter_t hashset_iter(hashset_t* hs) {
    if(hs == NULL || hs->data == NULL) return (hs_iter_t){0};
    hs_iter_t it = {NULL, hs, 0};
    return it;
}

int hashset_iter_next(hs_iter_t* it){
    if(it == NULL || it->_hs == NULL) return 0;

    hashset_t* hs = it->_hs;
    
    for(size_t i = it->_index; i < hs->cap + 1; i++){
        if(hs->data[i] != NULL){
            if(i == hs->cap)
                it->value = NULL; //special case for 0/NULL
            else
                it->value = hs->data[i];
            it->_index = i + 1;
            return 1;
        }
    }

    it->_index = hs->cap + 1;
    return 0;
}


// PS: i have yet to know how hashsets are actually implemented
#endif
