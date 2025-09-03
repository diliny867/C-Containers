#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct bitset_t {
    uint64_t* data;
    size_t count; //in bits
    size_t cap; //in uint64_ts
} bitfield_t;


void bitset_init(bitset_t* bs){
    bs->data = NULL;
    bs->count = 0;
    bs->cap = 0;
}
void bitset_clear(bitset_t* bs){
    bs->count = 0;
    if(bs->data)
        memset(bs->data, 0, bs->cap * sizeof(uint64_t));
}
void bitset_destroy(bitset_t* bs){
    bs->count = 0;
    bs->cap = 0;
    if(bs->data)
        free(bs->data);
}

#define _BITSET_INIT_CAP 1

#define _intindex(index) (index >> 6)
#define _valbit(index) (index & ((1 << 6) - 1))

static void bitset_maybe_expand(bitset_t* bs){
    if(bs->count >= (bs->cap << 6)){
        if(bs->cap == 0)
            bs->cap = _BITSET_INIT_CAP;
        bs->cap <<= 1;
        if(bs->data)
            free(bs->data);
        bs->data = (uint64_t*)calloc(bs->cap, sizeof(uint64_t*));
    }
}

void bitset_push(bitset_t* bs, int val){
    if(bs == NULL) return;
    bitset_maybe_expand(bs);

    size_t intindex = _intindex(bs->count);
    size_t valbit = _valbit(bs->count);
    if(valbit == (1 << 6) - 1){
        intindex++;
        valbit = 0;
    }
    bs->data[intindex] |= !!val << valbit;
    bs->count++;
}

int bitset_get(bitset_t* bs, size_t index){
    if(bs == NULL || bs->count == 0) return 0;
    assert(index < bs->count);

    size_t intindex = _intindex(index);
    size_t valbit = _valbit(index);
    return (bs->data[intindex] >> valbit) & 1;
}
int bitset_peek(bitset_t* bs){
    if(bs == NULL || bs->count == 0) return 0;

    return bitset_get(bs, bs->count - 1);
}

int bitset_clean(bitset_t* bs, size_t index){
    if(bs == NULL || bs->count == 0) return 0;
    assert(index < bs->count);

    size_t intindex = _intindex(bs->count);
    size_t valbit = _valbit(bs->count);

    bs->data[intindex] &= ~(1 << valbit);
    return (bs->data[intindex] >> valbit) & 1;
}
int bitset_put(bitset_t* bs, size_t index){
    if(bs == NULL || bs->count == 0) return 0;
    assert(index < bs->count);

    size_t intindex = _intindex(bs->count);
    size_t valbit = _valbit(bs->count);

    bs->data[intindex] |= 1 << valbit;
    return (bs->data[intindex] >> valbit) & 1;
}
int bitset_set(bitset_t* bs, size_t index, int val){
    if(bs == NULL || bs->count == 0) return 0;
    assert(index < bs->count);

    size_t intindex = _intindex(bs->count);
    size_t valbit = _valbit(bs->count);

    bs->data[intindex] = (bs->data[intindex] & ~(1 << valbit)) | !!val << valbit;
    return (bs->data[intindex] >> valbit) & 1;
}
int bitset_toggle(bitset_t* bs, size_t index){
    if(bs == NULL || bs->count == 0) return 0;
    assert(index < bs->count);

    size_t intindex = _intindex(bs->count);
    size_t valbit = _valbit(bs->count);

    bs->data[intindex] ^= 1 << valbit;
    return (bs->data[intindex] >> valbit) & 1;
}

int bitset_erase(bitset_t* bs, size_t index){
    if(bs == NULL || bs->count == 0) return 0;
    assert(index < bs->count);

    bs->count--;
    return bitset_get(bs, index);
}
int bitset_pop(bitset_t* bs){
    if(bs == NULL || bs->count == 0) return 0;

    return bitset_erase(bs, bs->count - 1);
}

void bitset_insert(bitset_t* bs, size_t index, int val){
    if(bs == NULL) return;
    assert(index <= bs->count);
    bitset_maybe_expand(bs);

    //if(index >= bs->count){
    //    bitset_push(bs, val);
    //    return;
    //}

    size_t intindex = _intindex(index);
    size_t valbit = _valbit(index);
    
    size_t maxintindex = _intindex(bs->count);
    size_t maxvalbit = _valbit(bs->count);
    if(maxvalbit == (1 << 6) - 1){
        maxintindex++;
        maxvalbit = 0;
    }

    while(maxintindex > intindex){
        bs->data[maxintindex] = (bs->data[maxintindex] << 1) | ((bs->data[maxintindex - 1] >> 63) & 1);
        maxintindex--;
    }

    bs->data[intindex] = (bs->data[intindex] << 1) | 
            (bs->data[intindex] & ((1 << valbit) - 1)) | 
            (!!val << valbit); //part left to bit + part right to bit + bit
}

