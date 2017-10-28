#include "minheap.h"

struct minheap *
	minheap_new(int cap, int(*less)(struct element *l, struct element *r)) {
		struct minheap *mh = (struct minheap*)malloc(sizeof(*mh));
		memset(mh, 0, sizeof(*mh));
		mh->elts = (struct element**)malloc(sizeof(struct element *) * cap);
		memset(mh->elts, 0, sizeof(struct element *) * cap);
		mh->cap = cap;
		mh->size = 0;
		mh->less = less;
		return mh;
	}

void
minheap_delete(struct minheap *mh) {
	free(mh->elts);
	free(mh);
	mh = NULL;
}

void
minheap_clear(struct minheap * mh, void(*clear)(struct element *elt)) {
	int i;
	for (i = 1; i <= mh->size; i++) {
		if (clear != NULL)
			clear(mh->elts[i]);
		else
			mh->elts[i]->index = 0;
	}
	mh->size = 0;
}


static inline void
swap(struct minheap * mh, int index0, int index1) {
	struct element * elt = mh->elts[index0];
	mh->elts[index0] = mh->elts[index1];
	mh->elts[index1] = elt;
	mh->elts[index0]->index = index0;
	mh->elts[index1]->index = index1;
}

void
up(struct minheap * mh, int index) {
	int parent = PARENT(index);
	while (parent >= 1) {
		if (mh->less(mh->elts[index], mh->elts[parent])) {
			minheap_swap(mh, index, parent);
			index = parent;
			parent = PARENT(index);
		}
		else {
			break;
		}
	}
}

void
down(struct minheap * mh, int index) {
	while (index <= mh->size)
	{
		int l = LEFT(index);
		int r = RIGHT(index);
		int min = index;

		if (l <= mh->size && mh->less(mh->elts[l], mh->elts[index]))
			min = l;

		if (r <= mh->size && mh->less(mh->elts[r], mh->elts[min]))
			min = r;

		if (min != index)
		{
			minheap_swap(mh, index, min);
			index = min;
		}
		else
			return;
	}
}

void
minheap_push(struct minheap * mh, struct element * elt) {
	if (elt->index)
	{
		minheap_change(mh, elt);
		return;
	}

	if (mh->size >= mh->cap - 1) {
		int nsize = mh->cap * 2;
		struct element ** elts = (struct element **)malloc(nsize * sizeof(struct element*));
		memset(elts, 0, nsize * sizeof(struct element*));
		memcpy(elts, mh->elts, mh->cap * sizeof(struct element*));
		free(mh->elts);
		mh->elts = elts;
		mh->cap = nsize;
	}
	++mh->size;
	mh->elts[mh->size] = elt;
	elt->index = mh->size;
	up(mh, elt->index);
}

struct element *
	minheap_pop(struct minheap * mh) {
		if (mh->size > 0) {
			struct element * elt = mh->elts[1];
			minheap_swap(mh, 1, mh->size);
			mh->elts[mh->size] = NULL;
			--mh->size;
			down(mh, 1);
			elt->index = 0;
			return elt;
		}
		return NULL;
	}

