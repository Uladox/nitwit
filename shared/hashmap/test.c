#include <stdio.h>
#include <stdlib.h>
#include "ntwt_hashmap.h"

int compare(void *key1,
	    uint32_t key1_size,
	    void *key2,
	    uint32_t key2_size)
{
	if (*(int *)key1 == *(int *)key2)
		return 1;
	return 0;
}

void free_contents(void *key, void *storage)
{
	free(key);
}

void hashmap_print(struct ntwt_hashmap *map)
{
	struct ntwt_hashbin *bin = map->bins;
	unsigned int bin_num = map->bin_num;

	int i;
	for (i = 0; i != bin_num; ++i, ++bin) {
		struct ntwt_hashentry *entry = bin->first;

		while (entry) {
			printf("%u ", *(unsigned int *) entry->key);
		        entry = entry->next;
		}
		printf("\n");
	}
}

int main(int argc, char **args)
{
	struct ntwt_hashmap *h = ntwt_hashmap_new(0, compare, free_contents);
	unsigned int i;
	for (i = 0; i != 60; ++i) {
		unsigned int *j = malloc(sizeof(i));

		*j = i;
		ntwt_hashmap_add(h, j, sizeof(*j), j);
	}
	i = 5;
	ntwt_hashmap_remove(h, &i, sizeof(i));
	if (!ntwt_hashmap_get(h, &i, sizeof(i)))
		printf("yes\n");
	i = 49;
	printf("%u\n", *(unsigned int *) ntwt_hashmap_get(h, &i, sizeof(i)));
	ntwt_hashmap_free(h);
}
