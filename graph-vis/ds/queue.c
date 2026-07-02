#include "queue.h"

struct AQueue AQ_Init(size_t cap) {
	struct AQueue q = { L_Init(cap) };
	return q;
}

void AQ_Enqueue(int val, struct AQueue* q) { L_Add(val, &q->list); }

int AQ_Dequeue(struct AQueue* q) {
	int val = q->list.arr[0];
	L_Del(0, &q->list);
	return val;
}

int AQ_IsEmpty(struct AQueue* q) { return q->list.len == 0; }

struct FQueue FQ_Init(size_t cap) {
	struct FQueue q = { (int*) malloc(cap * sizeof(int)), 0, 0, 0, cap };
	return q;
}

void FQ_Enqueue(int val, struct FQueue* q) {
	q->arr[q->tail] = val;
	q->tail = (q->tail + 1) % q->cap;
	q->len++;
}

int FQ_Dequeue(struct FQueue* q) {
	int val = q->arr[q->head];
	q->head = (q->head + 1) % q->cap;
	q->len--;
	return val;
}

int FQ_IsEmpty(struct FQueue* q) { return q->len == 0; }
int FQ_IsFull(struct FQueue* q)  { return q->len == q->cap; }
