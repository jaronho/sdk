#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

char err_ptr;
void* HT_ERROR = &err_ptr; // Data pointing to HT_ERROR are returned in case of error

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

/* Create a hashmap with capacity 'capacity'
	and return a pointer to it*/
hashmap_st* hashmap_create(unsigned long capacity) {
	hashmap_st* hasht = malloc(sizeof(hashmap_st));
	if (!hasht) {
		return NULL;
	}
	if (NULL == (hasht->map = malloc(capacity * sizeof(hashmap_element_st*)))) {
		free(hasht->map);
		return NULL;
	}
	hasht->capacity = capacity;
	hasht->count = 0;
	unsigned long i;
	for (i = 0; i < capacity; ++i) {
		hasht->map[i] = NULL;
	}
	return hasht;
}

/* Store data in the hashmap. If data with the same key are already stored return */
int hashmap_put(hashmap_st* hasht, const char* key, void* data) {
	if (NULL == hasht || NULL == key || NULL == data) {
		return 1;
	}
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_st* e = hasht->map[h];
	while (NULL != e) {
		if (!strcmp(e->key, key)) {	/* same key */
			return 2;
		}
		e = e->next;
	}
	// Getting here means the key doesn't already exist
	if (NULL == (e = malloc(sizeof(hashmap_element_st) + strlen(key) + 1))) {
		return 3;
	}
	strcpy(e->key, key);
	e->data = data;
	// Add the element at the beginning of the linked list
	e->next = hasht->map[h];
	hasht->map[h] = e;
	hasht->count ++;
	return 0;
}

/* Retrieve data from the hashmap */
void* hashmap_get(hashmap_st* hasht, const char* key) {
	if (NULL == hasht || NULL == key) {
		return NULL;
	}
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_st* e = hasht->map[h];
	while (NULL != e) {
		if (!strcmp(e->key, key)) {
			return e->data;
		}
		e = e->next;
	}
	return NULL;
}

/* Remove data from the hashmap. Return the data removed from the map
	so that we can free memory if needed */
void* hashmap_remove(hashmap_st* hasht, const char* key) {
	if (NULL == hasht || NULL == key) {
		return NULL;
	}
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_st* e = hasht->map[h];
	hashmap_element_st* prev = NULL;
	while (NULL != e) {
		if (!strcmp(e->key, key)) {
			void* ret = e->data;
			if (NULL != prev) {
				prev->next = e->next;
			} else {
				hasht->map[h] = e->next;
			}
			free(e);
			e = NULL;
			hasht->count--;
			return ret;
		}
		prev = e;
		e = e->next;
	}
	return NULL;
}

/* List keys. keys should have length equals or greater than the number of keys */
int hashmap_list_keys(hashmap_st* hasht, unsigned long keys_len, char** keys) {
	if (NULL == hasht || keys_len < hasht->count || NULL == keys) {
		return 1;
	}
	long ki = 0; //Index to the current string in **keys
	long i = hasht->capacity;
	while (--i >= 0) {
		hashmap_element_st* e = hasht->map[i];
		while (e) {
			keys[ki++] = e->key;
			e = e->next;
		}
	}
	return 0;
}

/* List values. values should have length equals or greater than the number of stored elements */
int hashmap_list_values(hashmap_st* hasht, unsigned long values_len, void** values) {
	if (NULL == hasht || values_len < hasht->count || NULL == values) {
		return 1;
	}
	long vi = 0; //Index to the current string in **values
	long i = hasht->capacity;
	while (--i >= 0) {
		hashmap_element_st* e = hasht->map[i];
		while (e) {
			values[vi++] = e->data;
			e = e->next;
		}
	}
	return 0;
}

/* Iterate through map's elements. */
hashmap_element_st* hashmap_iterate(hashmap_element_iterator_st* iterator) {
	if (NULL == iterator) {
		return NULL;
	}
	while (NULL == iterator->elem) {
		if (iterator->index < iterator->hm->capacity - 1) {
			iterator->index++;
			iterator->elem = iterator->hm->map[iterator->index];
		} else {
			return NULL;
		}
	}
	hashmap_element_st* e = iterator->elem;
	if (e) {
		iterator->elem = e->next;
	}
	return e;
}

/* Iterate through keys. */
const char* hashmap_iterate_keys(hashmap_element_iterator_st* iterator) {
	if (NULL == iterator) {
		return NULL;
	}
	hashmap_element_st* e = hashmap_iterate(iterator);
	return (NULL == e ? NULL : e->key);
}

/* Iterate through values. */
void* hashmap_iterate_values(hashmap_element_iterator_st* iterator) {
	if (NULL == iterator) {
		return NULL;
	}
	hashmap_element_st* e = hashmap_iterate(iterator);
	return (NULL == e ? NULL : e->data);
}

/* Removes all elements stored in the hashmap. if free_data, all stored datas are also freed.*/
int hashmap_clear(hashmap_st* hasht, int free_data) {
	if (NULL == hasht) {
		return 1;
	}
	hashmap_element_iterator_st it = HASHMAP_ITERATOR(hasht);
	const char* k = hashmap_iterate_keys(&it);
	while (NULL != k) {
		if (free_data) {
			free(ht_remove(hasht, k));
		} else {
			ht_remove(hasht, k);
		}
		k = hashmap_iterate_keys(&it);
	}
	return 0;
}

/* Destroy the hash map, and free memory. Data still stored are freed*/
int hashmap_destroy(hashmap_st* hasht) {
	if (NULL == hasht) {
		return 1;
	}
	hashmap_clear(hasht, 1); // Delete and free all.
	free(hasht->map);
	free(hasht);
	return 0;
}
