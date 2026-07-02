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
	memmove(&list->arr[index], &list->arr[index + 1], (list->len - index - 1) * sizeof(int));
	list->len--;
}
