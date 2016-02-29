/*    This file is part of nitwit.
 *
 *    nitwit is free software: you can redistribute it and/or modify
 *    it under the terms of the Lesser GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    nitwit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    Lesser GNU General Public License for more details.
 *
 *    You should have received a copy of the Lesser GNU General Public License
 *    along with Ruspma.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ntwt_hashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BIN_MAX_DENSITY 5
#define HASH_SEED 37

const static int hashmap_primes[] = {
	8 + 3, 16 + 3, 32 + 5, 64 + 3, 128 + 3, 256 + 27, 512 + 9,
	1024 + 9, 2048 + 5, 4096 + 3, 8192 + 27, 16384 + 43, 32768 + 3,
	65536 + 45, 131072 + 29, 262144 + 3, 524288 + 21, 1048576 + 7,
        2097152 + 17, 4194304 + 15, 8388608 + 9, 16777216 + 43, 33554432 + 35,
	67108864 + 15, 134217728 + 29, 268435456 + 3, 536870912 + 11,
	1073741824 + 85, 0
};

static void rehash(struct ntwt_hashmap *map);

#define ROT32(x, y) ((x << y) | (x >> (32 - y))) // avoid effort
uint32_t murmur3_32(const char *key, uint32_t len, uint32_t seed)
{
	static const uint32_t c1 = 0xcc9e2d51;
	static const uint32_t c2 = 0x1b873593;
	static const uint32_t r1 = 15;
	static const uint32_t r2 = 13;
	static const uint32_t m = 5;
	static const uint32_t n = 0xe6546b64;

	uint32_t hash = seed;

	const int nblocks = len / 4;
	const uint32_t *blocks = (const uint32_t *) key;
	int i;
	uint32_t k;
	for (i = 0; i < nblocks; i++) {
		k = blocks[i];
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;

		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}

	const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
	uint32_t k1 = 0;

	switch (len & 3) {
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0];

		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
	}

	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85ebca6b;
	hash ^= (hash >> 13);
	hash *= 0xc2b2ae35;
	hash ^= (hash >> 16);

	return hash;
}

/* Copies value of request pointer into the hashbin */
static struct ntwt_hashentry *new_hashentry(void *key, uint32_t key_size,
					    void *storage)
{
	struct ntwt_hashentry *entry = malloc(sizeof(*entry));

        entry->key = key;
	entry->key_size = key_size;
        entry->storage = storage;
        entry->next = NULL;
	return entry;
}

/* frees hashbin without freeing request, used in rehashing */
static void hashentry_free(struct ntwt_hashentry *hashentry)
{
	free(hashentry);
}

/* creates a new hashmap */
struct ntwt_hashmap *ntwt_hashmap_new(unsigned int sequence,
				      int (*compare)(void *key1,
						     uint32_t key1_size,
						     void *key2,
						     uint32_t key2_size),
				      void (*free_contents)(void *key, void *storage))
{
	struct ntwt_hashmap *map = malloc(sizeof(*map));
	struct ntwt_hashbin *bin;

	map->compare = compare;
	map->free_contents = free_contents;
	map->bin_num = hashmap_primes[sequence];
	map->bins = malloc(sizeof(*map->bins) * hashmap_primes[sequence]);
	map->entry_num = 0;
        map->primes_pointer = hashmap_primes;
	bin = map->bins;

	int i;
	for (i = 0; i != hashmap_primes[sequence]; ++i) {
		bin->first = NULL;
		++bin;
	}
	return map;
}

/* Final freeing of hashmap */
void ntwt_hashmap_free(struct ntwt_hashmap *map)
{
	struct ntwt_hashbin *bin = map->bins;

	int i;
	for (i = 0; i != map->bin_num; ++i, ++bin) {
		struct ntwt_hashentry *entry = bin->first;

		while (entry) {
			struct ntwt_hashentry *tmp = entry;

			map->free_contents(entry->key, entry->storage);
		        entry = entry->next;
			free(tmp);
		}
	}
	free(map->bins);
	free(map);
}

/* Adds an element to the hashmap */
int ntwt_hashmap_add(struct ntwt_hashmap *map,
		     void *key, uint32_t key_size,
		     void *storage)
{
	struct ntwt_hashentry *entry;
	unsigned int row;

	if (map->entry_num / map->bin_num >= BIN_MAX_DENSITY)
		rehash(map);

	row = murmur3_32(key, key_size, HASH_SEED) % map->bin_num;
        entry = map->bins[row].first;

	if (!entry) {
		map->bins[row].first = new_hashentry(key, key_size, storage);
		++map->entry_num;
		return NTWT_HASHMAP_ADDED;
	}

	if (map->compare(entry->key, entry->key_size, key, key_size))
		return NTWT_HASHMAP_ALREADY_PRESENT;


	while (entry->next) {
		entry = entry->next;
		if (map->compare(entry->key, entry->key_size, key, key_size))
			return NTWT_HASHMAP_ALREADY_PRESENT;
	}

	entry->next = new_hashentry(key, key_size, storage);
	++map->entry_num;
	return NTWT_HASHMAP_ADDED;
}

void ntwt_hashmap_remove(struct ntwt_hashmap *map, void *key,
			 uint32_t key_size)
{
	struct ntwt_hashentry *entry;
	unsigned int row;

	row = murmur3_32(key, key_size, HASH_SEED) % map->bin_num;
        entry = map->bins[row].first;

	if (!entry)
		return;

	if (map->compare(entry->key, entry->key_size, key, key_size)) {
		map->bins[row].first = entry->next;
		map->free_contents(entry->key, entry->storage);
		--map->entry_num;
		return;
	}

	struct ntwt_hashentry *next;
	for (next = entry->next; entry->next; entry = next) {

		if (map->compare(next->key, next->key_size, key, key_size)) {
			entry->next = next->next;
			map->free_contents(next->key, next->storage);
			free(next);
			--map->entry_num;
			return;
		}
	}
}

/* Gets an element from a hashmap */
void *ntwt_hashmap_get(struct ntwt_hashmap *map, void *key, uint32_t key_size)
{
	struct ntwt_hashentry *entry;
	unsigned int row;

        row = murmur3_32(key, key_size, HASH_SEED) % map->bin_num;

	entry = map->bins[row].first;
	while (entry) {
		if (map->compare(entry->key, entry->key_size, key, key_size))
			return entry->storage;
		entry = entry->next;
	}
	return NULL;
}

/* Adds to a bin something already in the hashmap during a rehash */
static void rehash_add(struct ntwt_hashbin *bin,
		       struct ntwt_hashentry *entry)
{
	struct ntwt_hashentry *tmp = bin->first;

	entry->next = NULL;
	if (!tmp) {
		bin->first = entry;
		return;
	}
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = entry;
}

/* Rehashes the hashmap */
static void rehash(struct ntwt_hashmap *map)
{
	struct ntwt_hashbin *new_bins;
	unsigned int new_bin_num;

	new_bin_num = map->primes_pointer[1];
	new_bins = malloc(sizeof(*new_bins) * new_bin_num);

	unsigned int i;
	for (i = 0; i != new_bin_num; ++i)
		new_bins[i].first = NULL;

	struct ntwt_hashbin *bin = map->bins;
	for (i = 0; i != map->bin_num; ++i, ++bin) {

		struct ntwt_hashentry *entry = bin->first;
		while (entry) {
			unsigned int row;
			struct ntwt_hashentry *tmp = entry->next;

			row = murmur3_32(entry->key, entry->key_size,
					 HASH_SEED) % new_bin_num;
			rehash_add(new_bins + row, entry);
			entry = tmp;
		}
	}
	free(map->bins);
	map->bins = new_bins;
	++map->primes_pointer;
	map->bin_num = new_bin_num;
}
