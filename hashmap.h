#pragma once

#include <stdlib.h>

typedef struct hm_item_t {
    char* key;
    void* value;
} hm_item_t;

typedef struct hashmap_t {
    hm_item_t* items;
    size_t count;
    size_t cap; //must be power of 2
} hashmap_t;

typedef struct hm_iter_t {
    char* key;
    void* value;
    hashmap_t* _hm;
    size_t _index;
} hm_iter_t;

// use for malloced hm, else hashmap_t hm = {0} is fine to pass to funcs
void hashmap_init(hashmap_t* hm);
// added = 1, changed(existed) = 0
int hashmap_set(hashmap_t* hm, char* key, void* value);
// adds only if not existed. added = 1, existed = 0
int hashmap_tryadd(hashmap_t* hm, char* key, void* value);
// changes only if existed. changed = 1, not existed = 0
int hashmap_trychange(hashmap_t* hm, char* key, void* value);
// exists = key, not exists = 0
void* hashmap_get(hashmap_t* hm, char* key);
// removed(existed) = key, not exists = 0
void* hashmap_remove(hashmap_t* hm, char* key);

void hashmap_clear(hashmap_t* hm);
void hashmap_destroy(hashmap_t* hm);

hm_iter_t hashmap_iter(hashmap_t* hm);
int hashmap_iter_next(hm_iter_t* it);

#define hashmap_seti(hm, key, value)       hashmap_set(hm, key, (void*)value) 
#define hashmap_tryaddi(hm, key, value)    hashmap_tryadd(hm, key, (void*)value) 
#define hashmap_trychangei(hm, key, value) hashmap_trychange(hm, key, (void*)value) 
#define hashmap_geti(hm, key)              (size_t)hashmap_get(hm, key) 
#define hashmap_removei(hm, key)           (size_t)hashmap_remove(hm, key) 


#ifdef HASHMAP_IMPLEMENTATION

#include <string.h>

#define _SOME_PRIME 342049
#define _HM_INIT_CAP (1 << 8)
#define _HM_GROW_THRESHOLD 0.8

#ifndef HM_HASH_FUNC
//http://www.cse.yorku.ca/~oz/hash.html
size_t djb2(char *str){
    size_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
#define HM_HASH_FUNC djb2 
#endif

void hashmap_init(hashmap_t* hm){
    hm->items = 0;
    hm->count = 0;
    hm->cap = 0;
}

static int _hashmap_set_unchecked(hashmap_t* hm, char* key, void* value){ // for expanding
    size_t hash = HM_HASH_FUNC(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(items[i].key == NULL){
            items[i].key = key;
            items[i].value = value;
            hm->count++;
            return 1;
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

static inline void hashmap_expand(hashmap_t* hm){
    if(hm->cap == 0)
        hm->cap = _HM_INIT_CAP;
    else
        hm->cap <<= 1;
    hm_item_t* old = hm->items;
    hm->items = (hm_item_t*)calloc(hm->cap, sizeof(hm_item_t));
    if(old == NULL) return;
    size_t count = hm->count;
    hm->count = 0;
    hm_item_t* oldit = old;
    while(count){ // iterate until we find all keys, so to not iterate tail of NULLs
        if(oldit->key != NULL){
            _hashmap_set_unchecked(hm, oldit->key, oldit->value); //can use unchecked as we know keys wont repeat in past hashmap array
            count--;
        }
        oldit++;
    }
    free(old);
}

static inline void hashmap_maybe_expand(hashmap_t* hm){
    if(hm->count < hm->cap * _HM_GROW_THRESHOLD) return; // works for hm = {0}, as if wont return if cap is 0
    hashmap_expand(hm);
}

int hashmap_set(hashmap_t* hm, char* key, void* value){
    if(hm == NULL || key == NULL) return 0;
    hashmap_maybe_expand(hm);
    
    size_t hash = HM_HASH_FUNC(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(items[i].key == NULL){
            items[i].key = key;
            items[i].value = value;
            hm->count++;
            return 1;
        }
        if(strcmp(items[i].key, key) == 0){
            items[i].value = value;
            return 0;
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

int hashmap_tryadd(hashmap_t* hm, char* key, void* value){
    if(hm == NULL || key == NULL) return 0;
    hashmap_maybe_expand(hm);
    
    size_t hash = HM_HASH_FUNC(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(items[i].key == NULL){
            items[i].key = key;
            items[i].value = value;
            hm->count++;
            return 1;
        }
        if(strcmp(items[i].key, key) == 0)
            return 0;
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

int hashmap_trychange(hashmap_t* hm, char* key, void* value){
    if(hm == NULL || key == NULL) return 0;
    hashmap_maybe_expand(hm);
    
    size_t hash = HM_HASH_FUNC(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(items[i].key == NULL)
            return 0;
        if(strcmp(items[i].key, key) == 0){
            items[i].value = value;
            return 1;
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

void* hashmap_get(hashmap_t* hm, char* key){
    if(hm == NULL || hm->items == NULL || key == NULL) return 0;
    //hashmap_maybe_expand(hm);

    size_t hash = HM_HASH_FUNC(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(items[i].key == NULL)
            return 0;
        if(strcmp(items[i].key, key) == 0)
            return items[i].value;
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

// shifts last collision element, so when finding them there were no holes between elements of same hash index
static void _hashmap_group(hashmap_t* hs, size_t index, size_t hash){
    size_t ihash;
    size_t mask = hs->cap - 1;
    size_t istart = hash & mask;
    size_t i = (index + _SOME_PRIME) & mask;
    size_t ilast = index;
    while(1){
        hm_item_t item = hs->items[i];
        if(item.key == NULL){ //stop when encountering a hole
            hs->items[index] = hs->items[ilast];
            hs->items[ilast] = (hm_item_t){0, 0};
            return;
        }
        ihash = HM_HASH_FUNC((size_t)item.key);
        if((ihash & mask) == istart){
            ilast = i;
        }
        i = (i + _SOME_PRIME) & mask;
    }
}

void* hashmap_remove(hashmap_t* hm, char* key){
    if(hm == NULL || hm->items == NULL || key == NULL) return 0;
    //hashmap_maybe_expand(hm);

    size_t hash = HM_HASH_FUNC(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(items[i].key == NULL)
            return 0;
        if(strcmp(items[i].key, key) == 0){
            void* value = items[i].value;
            items[i].key = 0;
            items[i].value = 0;
            hm->count--;
            _hashmap_group(hm, i, hash);
            return value;
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

void hashmap_clear(hashmap_t* hm){
    if(hm == NULL) return;
    if(hm->items)
        memset(hm->items, 0, hm->cap * sizeof(hm_item_t));
    hm->count = 0;
}

void hashmap_destroy(hashmap_t* hm){
    if(hm == NULL) return;
    if(hm->items)
        free(hm->items);
    hm->items = 0;
    hm->count = 0;
    hm->cap = 0;
}


hm_iter_t hashmap_iter(hashmap_t* hm){
    if(hm == NULL || hm->items == NULL) return (hm_iter_t){0};
    hm_iter_t it = {NULL, NULL, hm, 0};
    return it;
}

int hashmap_iter_next(hm_iter_t* it){
    if(it == NULL || it->_hm == NULL) return 0;

    hashmap_t* hm = it->_hm;
    
    for(size_t i = it->_index; i < hm->cap; i++){
        if(hm->items[i].key != NULL){
            it->key = hm->items[i].key;
            it->value = hm->items[i].value;
            it->_index= i + 1;
            return 1;
        }
    }

    it->_index = hm->cap;
    return 0;
}

#endif
