/**********************************************************************
* Author:	jaron.ho
* Date:		2017-09-19
* Brief:	hashmap
**********************************************************************/
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

/* Internal funcion to calculate hash for keys.
	It's based on the DJB algorithm from Daniel J. Bernstein.
	The key must be ended by '\0' character.*/
static unsigned int ht_calc_hash(const char* key) {
	unsigned int h = 5381;
	while (*(key++)) {
		h = ((h << 5) + h) + (*key);
	}
	return h;
}

/* Create a hashmap with capacity 'capacity' and return a pointer to it*/
hashmap_st* hashmap_create(unsigned long capacity) {
	hashmap_st* hasht = (hashmap_st*)malloc(sizeof(hashmap_st));
	if (!hasht) {
		return NULL;
	}
	hasht->capacity = capacity;
	hasht->count = 0;
	if (!(hasht->map = (hashmap_element_st*)malloc(capacity * sizeof(hashmap_element_st*)))) {
		free(hasht->map);
		return NULL;
	}
	unsigned long i;
	for (i = 0; i < capacity; ++i) {
		hasht->map[i] = NULL;
	}
#ifdef HASHMAP_THREAD_SAFETY
    pthread_mutex_init(&hasht->mutex, NULL);
#endif
	return hasht;
}

/* Destroy the hash map, and free memory. Data still stored are freed*/
int hashmap_destroy(hashmap_st* hasht) {
	if (!hasht) {
		return 1;
	}
	hashmap_clear(hasht, 1); // Delete and free all.
#ifdef HASHMAP_THREAD_SAFETY
    pthread_mutex_destroy(&hasht->mutex);
#endif
	free(hasht->map);
	free(hasht);
	return 0;
}

/* Removes all elements stored in the hashmap. if free_data, all stored datas are also freed.*/
int hashmap_clear(hashmap_st* hasht, int free_data) {
	if (!hasht) {
		return 1;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_lock(&hasht->mutex);
#endif
	hashmap_element_iterator_st it = HASHMAP_ITERATOR(hasht);
	const char* k = hashmap_iterate_key(&it);
	while (k) {
		if (free_data) {
			free(hashmap_remove(hasht, k));
		} else {
			hashmap_remove(hasht, k);
		}
		k = hashmap_iterate_key(&it);
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_unlock(&hasht->mutex);
#endif
	return 0;
}

/* Store data in the hashmap. If data with the same key are already stored return */
int hashmap_put(hashmap_st* hasht, const char* key, void* data) {
	if (!hasht || !key || !data) {
		return 1;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_lock(&hasht->mutex);
#endif
	if (hasht->count >= hasht->capacity) {
		return 2;
	}
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_st* e = hasht->map[h];
	while (e) {
		if (!strcmp(e->key, key)) {	/* same key */
			return 3;
		}
		e = e->next;
	}
	// Getting here means the key doesn't already exist
	if (!(e = (hashmap_element_st*)malloc(sizeof(hashmap_element_st) + strlen(key) + 1))) {
		return 4;
	}
	strcpy(e->key, key);
	e->data = data;
	// Add the element at the beginning of the linked list
	e->next = hasht->map[h];
	hasht->map[h] = e;
	hasht->count++;
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_unlock(&hasht->mutex);
#endif
	return 0;
}

/* Retrieve data from the hashmap */
void* hashmap_get(hashmap_st* hasht, const char* key) {
	if (!hasht || !key) {
		return NULL;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_lock(&hasht->mutex);
#endif
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_st* e = hasht->map[h];
	while (e) {
		if (!strcmp(e->key, key)) {
		#ifdef HASHMAP_THREAD_SAFETY
			pthread_mutex_unlock(&hasht->mutex);
		#endif
			return e->data;
		}
		e = e->next;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_unlock(&hasht->mutex);
#endif
	return NULL;
}

/* Remove data from the hashmap. Return the data removed from the map so that we can free memory if needed */
void* hashmap_remove(hashmap_st* hasht, const char* key) {
	if (!hasht || !key) {
		return NULL;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_lock(&hasht->mutex);
#endif
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_st* e = hasht->map[h];
	hashmap_element_st* prev = NULL;
	while (e) {
		if (!strcmp(e->key, key)) {
			void* ret = e->data;
			if (prev) {
				prev->next = e->next;
			} else {
				hasht->map[h] = e->next;
			}
			free(e);
			e = NULL;
			hasht->count--;
		#ifdef HASHMAP_THREAD_SAFETY
			pthread_mutex_unlock(&hasht->mutex);
		#endif
			return ret;
		}
		prev = e;
		e = e->next;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_unlock(&hasht->mutex);
#endif
	return NULL;
}

/* List keys. keys should have length equals or greater than the number of keys */
int hashmap_list_keys(hashmap_st* hasht, unsigned long keys_len, char** keys) {
	if (!hasht || keys_len < hasht->count || !keys) {
		return 1;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_lock(&hasht->mutex);
#endif
	long ki = 0; //Index to the current string in **keys
	long i = hasht->capacity;
	while (--i >= 0) {
		hashmap_element_st* e = hasht->map[i];
		while (e) {
			keys[ki++] = e->key;
			e = e->next;
		}
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_unlock(&hasht->mutex);
#endif
	return 0;
}

/* List values. values should have length equals or greater than the number of stored elements */
int hashmap_list_values(hashmap_st* hasht, unsigned long values_len, void** values) {
	if (!hasht || values_len < hasht->count || !values) {
		return 1;
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_lock(&hasht->mutex);
#endif
	long vi = 0; //Index to the current string in **values
	long i = hasht->capacity;
	while (--i >= 0) {
		hashmap_element_st* e = hasht->map[i];
		while (e) {
			values[vi++] = e->data;
			e = e->next;
		}
	}
#ifdef HASHMAP_THREAD_SAFETY
	pthread_mutex_unlock(&hasht->mutex);
#endif
	return 0;
}

/* Iterate through map's elements. */
hashmap_element_st* hashmap_iterate(hashmap_element_iterator_st* iterator) {
	if (!iterator) {
		return NULL;
	}
	while (!iterator->elem) {
		#ifdef HASHMAP_THREAD_SAFETY
			pthread_mutex_lock(&iterator->hm->mutex);
		#endif
		if (iterator->index < iterator->hm->capacity - 1) {
			iterator->index++;
			iterator->elem = iterator->hm->map[iterator->index];
			#ifdef HASHMAP_THREAD_SAFETY
				pthread_mutex_unlock(&iterator->hm->mutex);
			#endif
		} else {
			#ifdef HASHMAP_THREAD_SAFETY
				pthread_mutex_unlock(&iterator->hm->mutex);
			#endif
			return NULL;
		}
	}
	hashmap_element_st* e = iterator->elem;
	if (e) {
		iterator->elem = e->next;
	}
	return e;
}

/* Iterate through key. */
const char* hashmap_iterate_key(hashmap_element_iterator_st* iterator) {
	if (!iterator) {
		return NULL;
	}
	hashmap_element_st* e = hashmap_iterate(iterator);
	return (e ? e->key : NULL);
}

/* Iterate through value. */
void* hashmap_iterate_value(hashmap_element_iterator_st* iterator) {
	if (!iterator) {
		return NULL;
	}
	hashmap_element_st* e = hashmap_iterate(iterator);
	return (e ? e->data : NULL);
}
