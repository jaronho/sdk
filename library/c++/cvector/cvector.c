#include "cvector.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

const static size_t DEFAULT_CVECTOR_SIZE = 16;
const static size_t CVECTOR_GROW_SIZE = 8;


cvector* cvector_new() {
	unsigned int i = 0;
	cvector* new_cvector = (cvector*)malloc(sizeof(cvector));
	new_cvector->current_size = 0;
	new_cvector->total_size = DEFAULT_CVECTOR_SIZE;
	new_cvector->container = (void**)malloc(sizeof(void*) * DEFAULT_CVECTOR_SIZE);
	for (i = 0; i < new_cvector->total_size; ++i) {
		new_cvector->container[i] = NULL;
	}
	return new_cvector;
}

cvector* cvector_copy(cvector* v) {
	cvector* new_cvector = NULL;
	unsigned long i = 0;
	if (NULL == v) {
		return NULL;
	}
	new_cvector = cvector_new();
	for (i = 0; i < v->current_size; ++i) {
		void* src = cvector_at(v, i);
		void* dest = (void*)malloc(sizeof(void*));
		memcpy(dest, src, sizeof(src));
		cvector_push_back(new_cvector, dest);
	}
	return new_cvector;
}

void cvector_free(cvector* v, void freeFunc(void*)) {
	unsigned int i = 0;
	if (NULL == v) {
		return;
	}
	// if no function was supplied, call free on each ptr contained in our cvector.
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

void realloc_container(cvector* v, unsigned long new_size) {
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
		if (v->total_size - new_size < CVECTOR_GROW_SIZE) {
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

void cvector_push_back(cvector* v, void* element) {
	if (NULL == v || NULL == element) {
		return;
	}
	if (v->current_size == v->total_size) {
		realloc_container(v, v->total_size + CVECTOR_GROW_SIZE);
	}
	v->container[v->current_size++] = element;
}

void cvector_set_at(cvector* v, unsigned long index, void* element) {
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

void cvector_erase(cvector* v, unsigned long index) {
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

void* cvector_at(cvector* v, unsigned long index) {
	if (NULL == v || index >= v->total_size) {
		return NULL;
	}
	return v->container[index];
}

void* cvector_front(cvector* v) {
	if (NULL == v) {
		return NULL;
	}
	return v->container[0];
}

void* cvector_back(cvector* v) {
	if (NULL == v) {
		return NULL;
	}
	return v->container[v->current_size - 1];
}

unsigned long cvector_size(cvector* v) {
	if (NULL == v) {
		return 0;
	}
	return v->current_size;
}

unsigned long cvector_capacity(cvector* v) {
	if (NULL == v) {
		return 0;
	}
	return v->total_size;
}

void cvector_reserve(cvector* v, unsigned long length) {
	if (NULL == v || length <= v->total_size) {
		return;
	}
	realloc_container(v, length);
}
