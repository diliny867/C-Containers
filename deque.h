#pragma once

#include <stdlib.h>


typedef struct _deque_node {
	struct _deque_node* left;
	struct _deque_node* right;
	void* val;
} deque_node_t;

typedef struct {
	deque_node_t* head;
	deque_node_t* tail;
} deque_t;


deque_node_t* deque_node_new(deque_node_t* left, deque_node_t* right, void* val);

void deque_node_free(deque_node_t* node);

deque_t* deque_new(void);

void* deque_back(deque_t* deque);
void* deque_front(deque_t* deque);

void deque_push_back(deque_t* deque, void* val);
void deque_push_front(deque_t* deque, void* val);

void* deque_pop_back(deque_t* deque);
void* deque_pop_front(deque_t* deque);

void* deque_at(deque_t* deque, size_t index);

void deque_insert(deque_t* deque, size_t index, void* val);

void deque_erase(deque_t* deque, size_t index);

size_t deque_size(deque_t* deque);

int deque_contains(deque_t* deque, void* val, size_t val_size_bytes);
