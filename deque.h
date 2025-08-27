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


#ifdef DEQUE_IMPLEMENTATION

#include <string.h>

deque_node_t* deque_node_new(deque_node_t* left, deque_node_t* right, void* val) {
	deque_node_t* node = malloc(sizeof(deque_node_t));
	node->left = left;
	node->right = right;
	node->val = val;
	return node;
}

void deque_node_free(deque_node_t* node) {
	free(node);
}

deque_t* deque_new(void) {
	deque_t* deque = malloc(sizeof(deque_t));
	deque->head = NULL;
	deque->tail = NULL;
	return deque;
}

void* deque_back(deque_t* deque) {
	if(!deque || !deque->tail) {
		return NULL;
	}
	return deque->tail->val;
}
void* deque_front(deque_t* deque) {
	if(!deque || !deque->head) {
		return NULL;
	}
	return deque->head->val;
}

void deque_push_back(deque_t* deque, void* val) {
	if(!deque) {
		return;
	}
	if(!deque->tail) { // deque is empty
		deque->tail = deque_node_new(NULL, NULL, val);
		deque->head = deque->tail;
		return;
	}
	deque->tail->right = deque_node_new(deque->tail, NULL, val);
	deque->tail = deque->tail->right;
}
void deque_push_front(deque_t* deque, void* val) {
	if(!deque) {
		return;
	}
	if(!deque->head) { // deque is empty
		deque->head = deque_node_new(NULL, NULL, val);
		deque->tail = deque->head;
		return;
	}
	deque->head->left = deque_node_new(NULL, deque->head, val);
	deque->head = deque->head->left;
}

void* deque_pop_back(deque_t* deque) {
	if(!deque || !deque->tail) {
		return NULL;
	}
	deque_node_t* last_tail = deque->tail;
	if(deque->tail == deque->head) { // only 1 element in deque
		deque->tail = NULL;
		deque->head = NULL;
	}else {
		deque->tail = deque->tail->left;
	}
	void* val = last_tail->val;
	deque_node_free(last_tail);
	return val;
}
void* deque_pop_front(deque_t* deque) {
	if(!deque || !deque->head) {
		return NULL;
	}
	deque_node_t* last_head = deque->head;
	if(deque->head == deque->tail) { // only 1 element in deque
		deque->head = NULL;
		deque->tail = NULL;
	}else {
		deque->head = deque->head->right;
	}
	void* val = last_head->val;
	deque_node_free(last_head);
	return val;
}

void* deque_at(deque_t* deque, size_t index) {
	if(!deque) {
		return NULL;
	}
	deque_node_t* node = deque->head;
	for(size_t i = 0; i < index; i++) {
		if(!node) {
			return NULL;
		}
		node = node->right;
	}
	if(!node) {
		return NULL;
	}
	return node->val;
}

void deque_insert(deque_t* deque, size_t index, void* val) {
	if(!deque) {
		return;
	}
	if(index == 0) {
		deque_push_front(deque, val);
		return;
	}
	deque_node_t* node = deque->head;
	for(size_t i = 0; i < index - 1; i++) {
		if(!node) {
			return;
		}
		node = node->right;
	}
	if(node == deque->tail) {
		deque_push_back(deque, val);
		return;
	}
	deque_node_t* new_node = deque_node_new(node, node->right, val);
	node->right = new_node;
	new_node->right->left = new_node;
}

void deque_erase(deque_t* deque, size_t index) {
	if(!deque) {
		return;
	}
	deque_node_t* node = deque->head;
	for(size_t i = 0; i < index; i++) {
		if(!node) {
			return;
		}
		node = node->right;
	}
	if(node == deque->head) {
		deque_pop_front(deque);
		return;
	}
	if(node == deque->tail) {
		deque_pop_back(deque);
		return;
	}
	if(node->right){
		node->right->left = node->left;
	}
	if(node->left){
		node->left->right = node->right;
	}
	deque_node_free(node);
}

size_t deque_size(deque_t* deque) {
	if(!deque) {
		return 0;
	}
	size_t i = 0;
	deque_node_t* node = deque->head;
	while(node) {
		i++;
		node = node->right;
	}
	return i;
}

int deque_contains(deque_t* deque, void* val, size_t val_size_bytes) { // returns 0 if not found, 1 if found
	if(!deque) {
		return 0;
	}
	deque_node_t* node = deque->head;
	while(node) {
		if(memcmp(node->val, val, val_size_bytes) == 0) {
			return 1;
		}
		node = node->right;
	}
	return 0;
}

#endif
