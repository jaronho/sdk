/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-19
* Brief:	hashmap
**********************************************************************/
#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#define HASHMAP_THREAD_SAFETY	1	/* 1.thread safe, 0.no thread safe */

#if HASHMAP_THREAD_SAFETY
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define HASHMAP_DEFAULT_CAPACITY	10240

// Inititalize hashmap iterator on hashmap 'hm'
#define HASHMAP_ITERATOR(hm)	{hm, 0, hm->map[0]}

// Hashmap element structure
typedef struct hashmap_element_st {
	struct hashmap_element_st* next; // Next element in case of a collision
	void* data;	// Pointer to the stored element
	char key[]; 	// Key of the stored element
} hashmap_element_st;

// Structure used for iterations
typedef struct hashmap_element_iterator_st {
	struct hashmap_st* hm; 	// The hashmap on which we iterate
	unsigned long index;	// Current index in the map
	hashmap_element_st* elem; 	// Current element in the list
} hashmap_element_iterator_st;

// Hashtabe structure
typedef struct hashmap_st {
	unsigned long capacity;	// Hashmap capacity (in terms of hashed keys)
	unsigned long count;	// Count of element currently stored in the hashmap
	hashmap_element_st** map;	// The map containaing elements
#if HASHMAP_THREAD_SAFETY
	pthread_mutex_t mutex;
#endif
} hashmap_st;

/*
 * Brief:	create a hashmap
 * Param:	capacity - the max size of the hashmap
 * Return:	hashmap_st*
 */
extern hashmap_st* hashmap_create(unsigned long capacity);

/*
 * Brief:	destroy a hashmap
 * Param:	hasht - a hashmap
 * Return:	int, 0.ok, 1.fail
 */
extern int hashmap_destroy(hashmap_st* hasht);

/*
 * Brief:	clear a hashmap
 * Param:	hasht - a hashmap
 *			free_data - 1.free data from memory
 * Return:	int, 0.ok, 1.fail
 */
extern int hashmap_clear(hashmap_st* hasht, int free_data);

/*
 * Brief:	put data to hashmap
 * Param:	hasht - a hashmap
 *			key - key for data
 *			data - data
 * Return:	int, 0.ok, 1.param error, 2.out of capacity, 3.exist same key, 4.malloc error
 */
extern int hashmap_put(hashmap_st* hasht, const char* key, void* data);

/*
 * Brief:	get data from hashmap
 * Param:	hasht - a hashmap
 *			key - key for data
 * Return:	void*
 */
extern void* hashmap_get(hashmap_st* hasht, const char* key);

/*
 * Brief:	remove data from hashmap
 * Param:	hasht - a hashmap
 *			key - key for data
 * Return:	void*
 */
extern void* hashmap_remove(hashmap_st* hasht, const char* key);

/*
 * Brief:	get keys array from hashmap
 * Param:	hasht - a hashmap
 *			keys_len - length of keys array
 *			keys - keys array
 * Return:	int, 0.ok, 1.fail
 */
extern int hashmap_list_keys(hashmap_st* hasht, unsigned long keys_len, char** keys);

/*
 * Brief:	get values array from hashmap
 * Param:	hasht - a hashmap
 *			values_len - length of values array
 *			values - values array
 * Return:	int, 0.ok, 1.fail
 */
extern int hashmap_list_values(hashmap_st* hasht, unsigned long values_len, void** values);

/*
 * Brief:	get hashmap element from iterator
 * Param:	iterator - hashmap iterator
 * Return:	hashmap_element_st*
 */
extern hashmap_element_st* hashmap_iterate(hashmap_element_iterator_st* iterator);

/*
 * Brief:	get key from iterator
 * Param:	iterator - hashmap iterator
 * Return:	const char*
 */
extern const char* hashmap_iterate_key(hashmap_element_iterator_st* iterator);

/*
 * Brief:	get value from iterator
 * Param:	iterator - hashmap iterator
 * Return:	void*
 */
extern void* hashmap_iterate_value(hashmap_element_iterator_st* iterator);

#ifdef __cplusplus
}
#endif

#endif	// _HASHMAP_H_
