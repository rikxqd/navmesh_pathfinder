#ifndef _LIST_H
#define _LIST_H
#include <stdint.h>
#include <stdio.h>



struct list_node {
	struct list_node * pre;
	struct list_node * next;
};

struct list {
	struct list_node head;
	struct list_node tail;
};


#define LIST_INIT(dl) \
do \
{ \
	dl->head.pre = dl->tail.next = NULL; \
	dl->head.next = &dl->tail; \
	dl->tail.pre = &dl->head; \
} \
while (false);

#define LIST_HEAD(dl) (dl->head.next)
#define LIST_TAIL(dl) (&dl->tail)
#define LIST_EMPTY(dl) (dl->head.next == &dl->tail ? 1:0)

#define LIST_REMOVE(node) \
do \
{\
if (node->pre && node->next) \
{\
	node->pre->next = node->next; \
	node->next->pre = node->pre; \
	node->pre = node->next = NULL; \
}\
}\
while (false);

static inline struct list_node *
list_pop(struct list * dl) {
	struct list_node * node = NULL;
	if (!LIST_EMPTY(dl)) {
		node = dl->head.next;
		LIST_REMOVE(node);
	}
	return node;
}

#define LIST_POP(dl) (list_pop(dl))

#define LIST_PUSH(dl,node) do \
{\
if (node->pre == NULL && node->next == NULL)\
{\
	dl->tail.pre->next = node; \
	node->pre = dl->tail.pre; \
	dl->tail.pre = node; \
	node->next = &dl->tail; \
}\
} while (false);

#endif