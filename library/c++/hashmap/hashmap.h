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
typedef struct hashmap_element_t {
	struct hashmap_element_t* next; // Next element in case of a collision
	void* data;	// Pointer to the stored element
	char key[]; 	// Key of the stored element
} hashmap_element_t;

// Structure used for iterations
typedef struct hashmap_element_iterator_t {
	struct hashmap_t* hm; 	// The hashmap on which we iterate
	unsigned long index;	// Current index in the map
	hashmap_element_t* elem; 	// Curent element in the list
} hashmap_element_iterator_t;

// Hashtabe structure
typedef struct hashmap_t {
	unsigned long capacity;	// Hashmap capacity (in terms of hashed keys)
	unsigned long count;	// Count of element currently stored in the hashmap
	hashmap_element_t** map;	// The map containaing elements
} hashmap_t;

extern hashmap_t* hashmap_create(unsigned long capacity);

extern void* hashmap_put(hashmap_t* hasht, const char* key, void* data);

extern void* hashmap_get(hashmap_t* hasht, const char* key);

extern void* hashmap_remove(hashmap_t* hasht, const char* key);

extern void hashmap_list_keys(hashmap_t* hasht, unsigned long keys_len, char** keys);

extern void hashmap_list_values(hashmap_t* hasht, unsigned long values_len, void** values);

extern hashmap_element_t* hashmap_iterate(hashmap_element_iterator_t* iterator);

extern const char* hashmap_iterate_keys(hashmap_element_iterator_t* iterator);

extern void hashmap_clear(hashmap_t* hasht, int free_data);

extern void hashmap_destroy(hashmap_t* hasht);

#ifdef __cplusplus
}
#endif

#endif	// _HASH_MAP_H_
