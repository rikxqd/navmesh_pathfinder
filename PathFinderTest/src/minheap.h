#ifndef _MINHEAP_H
#define _MINHEAP_H

#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#ifndef _MSC_VER
#include <stdbool.h>
#else
#define inline __inline
#define false 0
#endif

struct element {
	int index;
};

struct minheap {
	int cap;
	int size;
	int(*less)(struct element *l, struct element *r);
	struct element **elts;
};

#define PARENT(index) (index/2)
#define LEFT(index) (index*2)
#define RIGHT(index) (index*2+1)

struct minheap * minheap_new(int cap, int(*less)(struct element *l, struct element *r));
void minheap_delete(struct minheap *mh);
void minheap_clear(struct minheap * mh, void(*clear)(struct element *elt));
void minheap_push(struct minheap * mh, struct element * elt);
struct element * minheap_pop(struct minheap * mh);
void up(struct minheap * mh, int index);
void down(struct minheap * mh, int index);


#define MINHEAP_SWAP(mh,index0,index1) do \
{\
struct element * elt = mh->elts[index0]; \
	mh->elts[index0] = mh->elts[index1]; \
	mh->elts[index1] = elt; \
	mh->elts[index0]->index = index0; \
	mh->elts[index1]->index = index1; \
} while (false);


#define MINHEAP_CHANGE(mh,elt) do \
{\
	int index = elt->index; \
	down(mh, index); \
if (index == elt->index)\
	up(mh, index); \
} while (false);

#define MINHEAP_TOP(mh) (mh->size > 0 ? mh->elts[1]:NULL)

#endif