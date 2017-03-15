#ifndef _CVECTOR_H_
#define _CVECTOR_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct s_cvector {
	unsigned long current_size;
	unsigned long total_size;
	void ** container;
} cvector;

extern cvector* cvector_new(void);

extern cvector* cvector_copy(cvector* v);

extern void cvector_free(cvector* v, void freeFunc(void*));

extern void cvector_push_back(cvector* v, void* element);

extern void cvector_set_at(cvector* v, unsigned long index, void* element);

extern void cvector_erase(cvector* v, unsigned long index);

extern void* cvector_at(cvector* v, unsigned long index);

extern void* cvector_front(cvector* v);

extern void* cvector_back(cvector* v);

extern unsigned long cvector_size(cvector* v);

extern unsigned long cvector_capacity(cvector* v);

extern void cvector_reserve(cvector* v, unsigned long length);

#ifdef __cplusplus
}
#endif

#endif	// _CVECTOR_H_
