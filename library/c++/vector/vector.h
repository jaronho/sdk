#ifndef _VECTOR_H_
#define _VECTOR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define VECTOR_DEFAULT_SIZE			32
#define VECTOR_DEFAULT_GROW_SIZE	16

typedef struct vector_t {
	unsigned long current_size;
	unsigned long total_size;
	unsigned long grow_size;
	void** container;
} vector_t;

extern vector_t* vector_new(unsigned long default_size, unsigned long grow_size);

extern vector_t* vector_copy(vector_t* v);

extern int vector_free(vector_t* v, void freeFunc(void*));

extern int vector_push_back(vector_t* v, void* element);

extern int vector_insert(vector_t* v, unsigned long index, void* element);

extern int vector_erase(vector_t* v, unsigned long index);

extern void* vector_at(vector_t* v, unsigned long index);

extern void* vector_front(vector_t* v);

extern void* vector_back(vector_t* v);

extern int vector_swap(vector_t* v, unsigned long i, unsigned long j);

extern unsigned long vector_size(vector_t* v);

extern unsigned long vector_capacity(vector_t* v);

extern int vector_reserve(vector_t* v, unsigned long length);

#ifdef __cplusplus
}
#endif

#endif	// _VECTOR_H_
