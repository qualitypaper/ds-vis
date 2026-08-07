#include "list.h"

struct List L_Init(size_t cap) {
	struct List list = { (int*) malloc(cap * sizeof(int)), 0, cap };
	return list;
}

void L_Grow(struct List* list) {
	list->cap = (size_t)(list->cap * GROWTH_FACTOR);
	list->arr = (int*) realloc(list->arr, list->cap * sizeof(int));
}

void L_Add(int val, struct List* list) {
	if (list->len >= list->cap) L_Grow(list);
	list->arr[list->len++] = val;
}

void L_Del(size_t index, struct List* list) {
	if (list->len == 0) return;
	MemoryCopy(&list->arr[index], &list->arr[index + 1], (list->len - index - 1) * sizeof(int));
	list->len--;
}

//////////////////////////////
//~ Singly linked list (S32, null-terminated)

SLinkedList SLL_Init(void) {
	SLinkedList l = { 0 };
	return l;
}

void SLL_PushFront(S32 val, SLinkedList* l) {
	SLinkedNode* n = (SLinkedNode*) malloc(sizeof(SLinkedNode));
	n->val = val;
	n->next = l->head;
	l->head = n;
}

void SLL_PushBack(S32 val, SLinkedList* l) {
	SLinkedNode* n = (SLinkedNode*) malloc(sizeof(SLinkedNode));
	n->val = val;
	n->next = NULL;
	if (l->head == NULL) {
		l->head = n;
	} else {
		SLinkedNode* c = l->head;
		while (c->next != NULL) c = c->next;
		c->next = n;
	}
}

S32 SLL_PopFront(SLinkedList* l) {
	SLinkedNode* n = l->head;
	S32 val = n->val;
	l->head = n->next;
	free(n);
	return val;
}

SLinkedNode* SLL_Find(S32 val, const SLinkedList* l) {
	for (SLinkedNode* n = l->head; n != NULL; n = n->next)
		if (n->val == val) return n;
	return NULL;
}

void SLL_Delete(SLinkedNode* node, SLinkedList* l) {
	if (l->head == node) {
		l->head = node->next;
	} else {
		SLinkedNode* p = l->head;
		while (p != NULL && p->next != node) p = p->next;
		if (p != NULL) p->next = node->next;
	}
	free(node);
}

size_t SLL_Count(const SLinkedList* l) {
	size_t c = 0;
	for (SLinkedNode* n = l->head; n != NULL; n = n->next) c++;
	return c;
}

void SLL_Free(SLinkedList* l) {
	SLinkedNode* n = l->head;
	while (n != NULL) {
		SLinkedNode* nx = n->next;
		free(n);
		n = nx;
	}
	l->head = NULL;
}

//////////////////////////////
//~ Doubly linked list (S32, null-terminated, head + tail)

LinkedList LL_Init(void) {
	LinkedList l = { 0 };
	return l;
}

void LL_PushFront(S32 val, LinkedList* l) {
	LinkedNode* n = (LinkedNode*) malloc(sizeof(LinkedNode));
	n->val = val;
	n->prev = NULL;
	n->next = l->head;
	if (l->head != NULL) l->head->prev = n;
	else l->tail = n;
	l->head = n;
}

void LL_PushBack(S32 val, LinkedList* l) {
	LinkedNode* n = (LinkedNode*) malloc(sizeof(LinkedNode));
	n->val = val;
	n->prev = l->tail;
	n->next = NULL;
	if (l->tail != NULL) l->tail->next = n;
	else l->head = n;
	l->tail = n;
}

S32 LL_PopFront(LinkedList* l) {
	LinkedNode* n = l->head;
	S32 val = n->val;
	l->head = n->next;
	if (l->head != NULL) l->head->prev = NULL;
	else l->tail = NULL;
	free(n);
	return val;
}

S32 LL_PopBack(LinkedList* l) {
	LinkedNode* n = l->tail;
	S32 val = n->val;
	l->tail = n->prev;
	if (l->tail != NULL) l->tail->next = NULL;
	else l->head = NULL;
	free(n);
	return val;
}

LinkedNode* LL_Find(S32 val, const LinkedList* l) {
	for (LinkedNode* n = l->head; n != NULL; n = n->next)
		if (n->val == val) return n;
	return NULL;
}

void LL_Delete(LinkedNode* node, LinkedList* l) {
	if (node->prev != NULL) node->prev->next = node->next;
	else l->head = node->next;
	if (node->next != NULL) node->next->prev = node->prev;
	else l->tail = node->prev;
	free(node);
}

size_t LL_Count(const LinkedList* l) {
	size_t c = 0;
	for (LinkedNode* n = l->head; n != NULL; n = n->next) c++;
	return c;
}

void LL_Free(LinkedList* l) {
	LinkedNode* n = l->head;
	while (n != NULL) {
		LinkedNode* nx = n->next;
		free(n);
		n = nx;
	}
	l->head = l->tail = NULL;
}
