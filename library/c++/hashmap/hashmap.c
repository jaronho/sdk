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
hashmap_t* hashmap_create(unsigned long capacity) {
	hashmap_t* hasht = malloc(sizeof(hashmap_t));
	if (!hasht) {
		return NULL;
	}
	if (NULL == (hasht->map = malloc(capacity * sizeof(hashmap_element_t*)))) {
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

/* Store data in the hashmap. If data with the same key are already stored,
	they are overwritten, and return by the function. Else it return NULL.
	Return HT_ERROR if there are memory alloc error*/
void* hashmap_put(hashmap_t* hasht, const char* key, void* data) {
	if (NULL == hasht || NULL == key || NULL == data) {
		return NULL;
	}
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_t* e = hasht->map[h];
	while (NULL != e) {
		if (!strcmp(e->key, key)) {
			void* ret = e->data;
			e->data = data;
			return ret;
		}
		e = e->next;
	}
	// Getting here means the key doesn't already exist
	if (NULL == (e = malloc(sizeof(hashmap_element_t) + strlen(key) + 1))) {
		return HT_ERROR;
	}
	strcpy(e->key, key);
	e->data = data;
	// Add the element at the beginning of the linked list
	e->next = hasht->map[h];
	hasht->map[h] = e;
	hasht->count ++;
	return NULL;
}

/* Retrieve data from the hashmap */
void* hashmap_get(hashmap_t* hasht, const char* key) {
	if (NULL == hasht || NULL == key) {
		return NULL;
	}
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_t* e = hasht->map[h];
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
void* ht_remove(hashmap_t* hasht, const char* key) {
	if (NULL == hasht || NULL == key) {
		return NULL;
	}
	unsigned int h = ht_calc_hash(key) % hasht->capacity;
	hashmap_element_t* e = hasht->map[h];
	hashmap_element_t* prev = NULL;
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
void hashmap_list_keys(hashmap_t* hasht, unsigned long keys_len, char** keys) {
	if (NULL == hasht || keys_len < hasht->count || NULL == keys) {
		return;
	}
	long ki = 0; //Index to the current string in **keys
	long i = hasht->capacity;
	while (--i >= 0) {
		hashmap_element_t* e = hasht->map[i];
		while (e) {
			keys[ki++] = e->key;
			e = e->next;
		}
	}
}

/* List values. values should have length equals or greater than the number of stored elements */
void hashmap_list_values(hashmap_t* hasht, unsigned long values_len, void** values) {
	if (NULL == hasht || values_len < hasht->count || NULL == values) {
		return;
	}
	long vi = 0; //Index to the current string in **values
	long i = hasht->capacity;
	while (--i >= 0) {
		hashmap_element_t* e = hasht->map[i];
		while (e) {
			values[vi++] = e->data;
			e = e->next;
		}
	}
}

/* Iterate through map's elements. */
hashmap_element_t* hashmap_iterate(hashmap_element_iterator_t* iterator) {
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
	hashmap_element_t* e = iterator->elem;
	if (e) {
		iterator->elem = e->next;
	}
	return e;
}

/* Iterate through keys. */
const char* hashmap_iterate_keys(hashmap_element_iterator_t* iterator) {
	if (NULL == iterator) {
		return NULL;
	}
	hashmap_element_t* e = hashmap_iterate(iterator);
	return (NULL == e ? NULL : e->key);
}

/* Iterate through values. */
void* ht_iterate_values(hashmap_element_iterator_t* iterator) {
	if (NULL == iterator) {
		return NULL;
	}
	hashmap_element_t* e = hashmap_iterate(iterator);
	return (NULL == e ? NULL : e->data);
}

/* Removes all elements stored in the hashmap. if free_data, all stored datas are also freed.*/
void hashmap_clear(hashmap_t* hasht, int free_data) {
	if (NULL == hasht) {
		return;
	}
	hashmap_element_iterator_t it = HT_ITERATOR(hasht);
	const char* k = hashmap_iterate_keys(&it);
	while (NULL != k) {
		if (free_data) {
			free(ht_remove(hasht, k));
		} else {
			ht_remove(hasht, k);
		}
		k = hashmap_iterate_keys(&it);
	}
}

/* Destroy the hash map, and free memory. Data still stored are freed*/
void hashmap_destroy(hashmap_t* hasht) {
	if (NULL == hasht) {
		return;
	}
	hashmap_clear(hasht, 1); // Delete and free all.
	free(hasht->map);
	free(hasht);
}
