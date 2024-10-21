#include "arena.h"


#define _ARENA_CHUNK_DEFAULT_CAPACITY (16384)


static arena_chunk_t* arena_chunk_new(size_t cap){
    arena_chunk_t* chunk;
    chunk = (arena_chunk_t*)malloc(sizeof(arena_chunk_t) + sizeof(uint8_t) * cap);
    chunk->cap = cap;
    chunk->size = 0;
    chunk->next = NULL;
    return chunk;
}
static void arena_chunk_free(arena_chunk_t* ch){
    free(ch);
}

arena_t* arena_new(){
    arena_t* a = (arena_t*)malloc(sizeof(arena_t));
    a->start = NULL;
    a->end = NULL;
    return a;
}
void arena_free(arena_t* a){
    if(a->start != NULL){
        arena_chunk_t* ch = a->start;
        arena_chunk_t* curr;
        do{
            curr = ch;
            ch = ch->next;
            arena_chunk_free(curr);
        } while (ch != a->start);
    }
    free(a);
}

void* arena_alloc(arena_t* a, size_t size){
    size_t alloc_size = _ARENA_CHUNK_DEFAULT_CAPACITY;
    if(size > alloc_size){
        alloc_size = ((size - 1) / _ARENA_CHUNK_DEFAULT_CAPACITY + 1) * _ARENA_CHUNK_DEFAULT_CAPACITY;
    }

    arena_chunk_t* ch = a->end;

    if(ch == NULL){
        a->end = arena_chunk_new(alloc_size);
        ch = a->end;
        a->start = ch;
        ch->next = a->start;
    }

    if(ch->size + size <= ch->cap){
        ch->size += size;
        return ch->data + ch->size - size;
    }

    while(ch->next != a->end && ch->next->size + size > ch->next->cap){
        ch = ch->next;
    }

    if(ch->next == a->end){
        ch->next->next = arena_chunk_new(alloc_size);
        ch = ch->next;
        ch->next->next = a->start;
    }

    ch->next->size += size;
    return ch->next->data + ch->next->size - size;
}

