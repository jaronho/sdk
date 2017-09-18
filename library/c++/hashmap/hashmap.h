#ifndef _HASH_MAP_H_
#define _HASH_MAP_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define HASHMAP_DEFAULT_CAPACITY	10240

// Inititalize hashmap iterator on hashmap 'hm'
#define HASHMAP_ITERATOR(hm) {hm, 0, hm->map[0]}

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
	hashmap_element_st* elem; 	// Curent element in the list
} hashmap_element_iterator_st;

// Hashtabe structure
typedef struct hashmap_st {
	unsigned long capacity;	// Hashmap capacity (in terms of hashed keys)
	unsigned long count;	// Count of element currently stored in the hashmap
	hashmap_element_st** map;	// The map containaing elements
} hashmap_st;

extern hashmap_st* hashmap_create(unsigned long capacity);

extern int hashmap_put(hashmap_st* hasht, const char* key, void* data);

extern void* hashmap_get(hashmap_st* hasht, const char* key);

extern void* hashmap_remove(hashmap_st* hasht, const char* key);

extern int hashmap_list_keys(hashmap_st* hasht, unsigned long keys_len, char** keys);

extern int hashmap_list_values(hashmap_st* hasht, unsigned long values_len, void** values);

extern hashmap_element_st* hashmap_iterate(hashmap_element_iterator_st* iterator);

extern const char* hashmap_iterate_keys(hashmap_element_iterator_st* iterator);

extern int hashmap_clear(hashmap_st* hasht, int free_data);

extern int hashmap_destroy(hashmap_st* hasht);

#ifdef __cplusplus
}
#endif

#endif	// _HASH_MAP_H_
