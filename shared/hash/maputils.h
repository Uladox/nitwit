/* Include these
 * #include <stdint.h>
 * #include <stdio.h>
 * #include "hashmap.h"
 */

void
ntwt_hashmap_gen(struct ntwt_hashmap *map, FILE *output, const char *name,
		 const char *headers, const char *compare,
		 void (*key_print)(FILE *output, void *key, uint32_t key_size),
		 void (*storage_print)(FILE *output, void *storage));
