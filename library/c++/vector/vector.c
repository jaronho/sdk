#include "vector_t.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

vector_t* vector_new(unsigned long default_size, unsigned long grow_size) {
	unsigned int i = 0;
	vector_t* new_vector = (vector_t*)malloc(sizeof(vector_t));
	new_vector->current_size = 0;
	new_vector->total_size = default_size;
	new_vector->grow_size = grow_size;
	new_vector->container = (void**)malloc(sizeof(void*) * default_size);
	for (i = 0; i < new_vector->total_size; ++i) {
		new_vector->container[i] = NULL;
	}
	return new_vector;
}

vector_t* vector_copy(vector_t* v) {
	vector_t* new_vector = NULL;
	unsigned long i = 0;
	if (NULL == v) {
		return NULL;
	}
	new_vector = vector_new();
	for (i = 0; i < v->current_size; ++i) {
		void* src = vector_at(v, i);
		void* dest = (void*)malloc(sizeof(void*));
		memcpy(dest, src, sizeof(src));
		vector_push_back(new_vector, dest);
	}
	return new_vector;
}

void vector_free(vector_t* v, void freeFunc(void*)) {
	unsigned int i = 0;
	if (NULL == v) {
		return;
	}
	// if no function was supplied, call free on each ptr contained in our vector_t.
	if (NULL == freeFunc) {
		for (i = 0; i < v->current_size; ++i) {
			free(v->container[i]);
			v->container[i] = NULL;
		}
	} else {	// otherwise, call the user supplied function.
		for (i = 0; i < v->current_size; ++i) {
			freeFunc(v->container[i]);
		}
	}
	free(v->container);
	v->container = NULL;
	free(v);
	v = NULL;
}

void realloc_container(vector_t* v, unsigned long new_size) {
	void** ptr = NULL;
	unsigned long i = 0;
	unsigned long old_total_size = 0;
	if (NULL == v || new_size == v->total_size) {
		return;
	}
	if (new_size < v->total_size) {
		for (i = new_size; i < v->total_size; ++i) {
			if (v->container[i]) {
				free(v->container[i]);
				v->container[i] = NULL;
			}
		}
		if (v->total_size - new_size < v->grow_size) {
			return;
		}
	}
	old_total_size = v->total_size;
	v->total_size = new_size;
	ptr = (void**)realloc(v->container, sizeof(void*) * new_size);
	if (NULL == ptr) {
		printf("call to realloc failed.");
		return;
	}
	v->container = ptr;
	for (i=old_total_size; i<new_size; ++i) {
		v->container[i] = NULL;
	}
}

void vector_push_back(vector_t* v, void* element) {
	if (NULL == v || NULL == element) {
		return;
	}
	if (v->current_size == v->total_size) {
		realloc_container(v, v->total_size + v->grow_size);
	}
	v->container[v->current_size++] = element;
}

void vector_insert(vector_t* v, unsigned long index, void* element) {
	if (NULL == v || index >= v->total_size ||  NULL == element) {
		return;
	}
	if (v->container[index]) {
		free(v->container[index]);
		v->container[index] = NULL;
	} else {
		v->current_size++;
	}
	v->container[index] = element;
}

void vector_erase(vector_t* v, unsigned long index) {
	void* temp = NULL;
	unsigned long i = 0;
	if (NULL == v || index >= v->total_size) {
		return;
	}
	temp = v->container[index];
	for (i = index; i < v->total_size; ++i) {
		if (i + 1 < v->total_size) {
			v->container[i] = v->container[i + 1];
		} else {
			v->container[i] = temp;
		}
	}
	v->current_size--;
	realloc_container(v, v->total_size - 1);
}

void* vector_at(vector_t* v, unsigned long index) {
	if (NULL == v || index >= v->total_size) {
		return NULL;
	}
	return v->container[index];
}

void* vector_front(vector_t* v) {
	if (NULL == v) {
		return NULL;
	}
	return v->container[0];
}

void* vector_back(vector_t* v) {
	if (NULL == v) {
		return NULL;
	}
	return v->container[v->current_size - 1];
}

extern void vector_swap(vector_t* v, unsigned long i, unsigned long j) {
	void* tmp;
	if (NULL == v || i >= v->current_size || j >= v->current_size) {
		return;
	}
	tmp = v->container[i];
	v->container[i] = v->container[j];
	v->container[j] = tmp;
}

unsigned long vector_size(vector_t* v) {
	if (NULL == v) {
		return 0;
	}
	return v->current_size;
}

unsigned long vector_capacity(vector_t* v) {
	if (NULL == v) {
		return 0;
	}
	return v->total_size;
}

void vector_reserve(vector_t* v, unsigned long length) {
	if (NULL == v || length <= v->total_size) {
		return;
	}
	realloc_container(v, length);
}