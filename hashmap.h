#pragma once

#include <stdlib.h>

typedef struct hm_item_t {
    char* key;
    void* value;
#ifdef HASHMAP_TOMBSTONES
    char deleted; // becomes 1 when removed and gets skipped when searching for element
#endif
} hm_item_t;

typedef struct hashmap_t {
    hm_item_t* items;
    size_t count;
    size_t cap; //must be power of 2

    size_t (*_hash) (char*); // hash function
    int (*_equal) (char*, char*); // equal function
} hashmap_t;

typedef struct hm_iter_t {
    char* key;
    void* value;
    hashmap_t* _hm;
    size_t _index;
} hm_iter_t;

void hashmap_init(hashmap_t* hm, size_t (*hash_func) (char*), int (*equal_func) (char*, char*));
void hashmap_init_c(hashmap_t* hm);
// added = 1, changed(existed) = 0
int hashmap_set_(hashmap_t* hm, char* key, void* value);
// adds only if not existed. added = 1, existed = 0
int hashmap_tryadd_(hashmap_t* hm, char* key, void* value);
// changes only if existed. changed = 1, not existed = 0
int hashmap_trychange_(hashmap_t* hm, char* key, void* value);
// exists = key, not exists = 0
void* hashmap_get(hashmap_t* hm, char* key);
// removed(existed) = key, not exists = 0
void* hashmap_remove(hashmap_t* hm, char* key);

void hashmap_clear(hashmap_t* hm);
void hashmap_destroy(hashmap_t* hm);

hm_iter_t hashmap_iter(hashmap_t* hm);
int hashmap_iter_next(hm_iter_t* it);

#define hashmap_set(hm, key, value)       hashmap_set_(hm, key, (void*)value) 
#define hashmap_tryadd(hm, key, value)    hashmap_tryadd_(hm, key, (void*)value) 
#define hashmap_trychange(hm, key, value) hashmap_trychange_(hm, key, (void*)value) 
// #define hashmap_geti(hm, key)              (size_t)hashmap_get(hm, key) 
// #define hashmap_removei(hm, key)           (size_t)hashmap_remove(hm, key) 


#ifdef HASHMAP_IMPLEMENTATION

#include <string.h>

#define _SOME_PRIME 342049
#define _HM_INIT_CAP (1 << 8)
#define _HM_GROW_THRESHOLD 0.8

#ifdef HASHMAP_TOMBSTONES
#define _HS_NOT_TOMBSTONE(i) i.deleted == 0
#define _HS_SET_TOMBSTONE(i, v) i.deleted = v
#else
#define _HS_NOT_TOMBSTONE(i) 1
#define _HS_SET_TOMBSTONE(i, v)
#endif

//http://www.cse.yorku.ca/~oz/hash.html
static size_t _djb2(char *str){
    size_t hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
static int _str_equal(char* s1, char* s2){
    return strcmp(s1, s2) == 0;
}

void hashmap_init(hashmap_t* hm, size_t (*hash_func) (char*), int (*equal_func) (char*, char*)){
    hm->items = 0;
    hm->count = 0;
    hm->cap = 0;
    hm->_hash = hash_func ? hash_func : _djb2;
    hm->_equal = equal_func ? equal_func : _str_equal;
}
void hashmap_init_c(hashmap_t* hm){
    hashmap_init(hm, _djb2, _str_equal);
}

static int _hashmap_set_unchecked(hashmap_t* hm, char* key, void* value){ // for expanding
    size_t hash = hm->_hash(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(items[i].key == NULL){
            items[i].key = key;
            items[i].value = value;
            // items[i].deleted = 0; //can skip zeroing it as we calloced items
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
        if(oldit->key != NULL && _HS_NOT_TOMBSTONE(oldit)){
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

int hashmap_set_(hashmap_t* hm, char* key, void* value){
    if(hm == NULL || key == NULL) return 0;
    hashmap_maybe_expand(hm);
    
    size_t hash = hm->_hash(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* slot = NULL; //leave for not tombstones as it wont really change anything (fr wont change anything in actual optimizng compiler)
    hm_item_t* items = hm->items;
    while(1){
        if(_HS_NOT_TOMBSTONE(items[i])){
            if(items[i].key == NULL){
                if(slot == NULL)
                    slot = items + i;
                slot->key = key;
                slot->value = value;
                _HS_SET_TOMBSTONE(slot, 0);
                hm->count++;
                return 1;
            }
            if(hm->_equal(items[i].key, key)){
                items[i].value = value;
                return 0;
            }
        }else if(slot == NULL)
            slot = items + i;
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

int hashmap_tryadd_(hashmap_t* hm, char* key, void* value){
    if(hm == NULL || key == NULL) return 0;
    hashmap_maybe_expand(hm);
    
    size_t hash = hm->_hash(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* slot = NULL; //leave for not tombstones as it wont really change anything (fr wont change anything in actual optimizng compiler)
    hm_item_t* items = hm->items;
    while(1){
        if(_HS_NOT_TOMBSTONE(items[i])){
            if(items[i].key == NULL){
                if(slot == NULL)
                    slot = items + i;
                slot->key = key;
                slot->value = value;
                _HS_SET_TOMBSTONE(slot, 0);
                hm->count++;
                return 1;
            }
            if(hm->_equal(items[i].key, key))
                return 0;
        }else if(slot == NULL)
            slot = items + i;
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

int hashmap_trychange_(hashmap_t* hm, char* key, void* value){
    if(hm == NULL || key == NULL) return 0;
    hashmap_maybe_expand(hm);
    
    size_t hash = hm->_hash(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(_HS_NOT_TOMBSTONE(items[i])){
            if(items[i].key == NULL)
                return 0;
            if(hm->_equal(items[i].key, key)){
                items[i].value = value;
                return 1;
            }
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

void* hashmap_get(hashmap_t* hm, char* key){
    if(hm == NULL || hm->items == NULL || key == NULL) return 0;
    //hashmap_maybe_expand(hm);

    size_t hash = hm->_hash(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(_HS_NOT_TOMBSTONE(items[i])){
            if(items[i].key == NULL)
                return 0;
            if(hm->_equal(items[i].key, key))
                return items[i].value;
        }
        i = (i + _SOME_PRIME) & mask; // calc next slot
    }

    return 0;
}

void* hashmap_remove(hashmap_t* hm, char* key){
    if(hm == NULL || hm->items == NULL || key == NULL) return 0;
    //hashmap_maybe_expand(hm);

    size_t hash = hm->_hash(key);
    
    size_t mask = hm->cap - 1;
    size_t i = hash & mask;

    hm_item_t* items = hm->items;
    while(1){
        if(_HS_NOT_TOMBSTONE(items[i])){
            if(items[i].key == NULL)
                return 0;
            if(hm->_equal(items[i].key, key)){
                void* value = items[i].value;
                items[i].key = 0;
                items[i].value = 0;
                _HS_SET_TOMBSTONE(items[i], 1);
                hm->count--;
                return value;
            }
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
    hm_item_t* items = hm->items;
    for(size_t i = it->_index; i < hm->cap; i++){
        if(items[i].key != NULL && _HS_NOT_TOMBSTONE(items[i])){
            it->key = items[i].key;
            it->value = items[i].value;
            it->_index= i + 1;
            return 1;
        }
    }

    it->_index = hm->cap;
    return 0;
}

#endif
