/*    This file is part of nitwit.
 *
 *    nitwit is free software: you can redistribute it and/or modify
 *    it under the terms of the Lesser GNU General Public License as
 *    published by the Free Software Foundation, either version 3 of
 *    the License, or (at your option) any later version.
 *
 *    nitwit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Lesser GNU General Public License for more details.
 *
 *    You should have received a copy of the Lesser GNU General Public License
 *    along with nitwit.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Include these
 * #include <stdint.h>
 */

enum { NTWT_HASHMAP_ALREADY_PRESENT, NTWT_HASHMAP_ADDED };

struct ntwt_hashentry {
	void *key;
	uint32_t key_size;
	void *storage;
	struct ntwt_hashentry *next;
};

struct ntwt_hashbin {
	struct ntwt_hashentry *first;
};

struct ntwt_hashmap {
	int (*compare)(const void *entry_key, uint32_t entry_key_size,
		       const void *key, uint32_t key_size);
	void (*free_contents)(void *key, void *storage);
	unsigned int bin_num;
	int entry_num;
	const int *primes_pointer;
	struct ntwt_hashbin *bins;
};

struct ntwt_hashmap *ntwt_hashmap_new(unsigned int sequence,
				      int (*compare)(const void *entry_key,
						     uint32_t entry_key_size,
						     const void *key,
						     uint32_t key_size),
				      void (*free_contents)(void *key,
							    void *storage));

void ntwt_hashmap_free(struct ntwt_hashmap *hashmap);

int ntwt_hashmap_add(struct ntwt_hashmap *hashmap,
		     void *key, uint32_t key_size,
		     void *storage);

void ntwt_hashmap_remove(struct ntwt_hashmap *map,
			 void *key,
			 uint32_t key_size);

void *ntwt_hashmap_get(struct ntwt_hashmap *map,
		       const void *key,
		       uint32_t key_size);

#if defined NTWT_SHORT_NAMES || defined NTWT_HASHMAP_SHORT_NAMES
#define hashmap_new(...) ntwt_hashmap_new(__VA_ARGS__)
#define hashmap_free(...) ntwt_hashmap_free(__VA_ARGS__)
#define hashmap_add(...) ntwt_hashmap_add(__VA_ARGS__)
#define hashmap_remove(...) ntwt_hashmap_remove(__VA_ARGS__)
#define hashmap_get(...) ntwt_hashmap_get(__VA_ARGS__)
#endif
