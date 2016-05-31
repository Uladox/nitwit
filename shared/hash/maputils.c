#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "../list/list.h"
#include "hashmap.h"

#define QUOTE(...) #__VA_ARGS__

static const char *free_contents = QUOTE(
	static void free_contents(void *key, void *storage)
	{
		fprintf(stderr, "Error: Tried to free generated map!\n");
		exit(1);
	});


static inline void
entry_gen(struct ntwt_hashentry *entry, FILE *output,
	  void (*key_print)(FILE *output, void *key, uint32_t key_size),
	  void (*storage_print)(FILE *output, void *storage))
{
	int entry_num = 0;

	for (; entry; entry = NTWT_LIST_NEXT(entry)) {
		++entry_num;
		fputs("&(struct ntwt_hashentry) {", output);
		fprintf(output, "\n\t\t.key_size = %"PRIu32, entry->key_size);

		fputs(",\n\t\t.key = ", output);
		key_print(output, entry->key, entry->key_size);
		fputs(",\n\t\t.storage = ", output);
		storage_print(output, entry->storage);

		fputs(",\n\t\t.next.next = ", output);
	}
	fputs("NULL", output);
	for (; entry_num; --entry_num)
		fputc('}', output);
	fputc(',', output);
}

static inline void
bins_gen(struct ntwt_hashbin *bin, FILE *output, int bin_num,
	 void (*key_print)(FILE *output, void *key, uint32_t key_size),
	 void (*storage_print)(FILE *output, void *storage))
{
	int i = 0;

	fprintf(output, ",\n.bins = (struct ntwt_hashbin[%i]) {", bin_num);

	for (; i != bin_num; ++i, ++bin) {
		fprintf(output, "\n\t[%i].first = ", i);
		entry_gen(bin->first, output, key_print, storage_print);
	}

	fputs("\n}\n", output);
}

void
ntwt_hashmap_gen(struct ntwt_hashmap *map, FILE *output, const char *name,
		 const char *headers, const char *compare,
		 void (*key_print)(FILE *output, void *key, uint32_t key_size),
		 void (*storage_print)(FILE *output, void *storage))
{
	struct ntwt_hashbin *bin = map->bins;

	fputs("/* Do not edit this file, it is generated! */\n", output);
	fputs("#include <stdio.h>\n"
	      "#include <stdlib.h>\n"
	      "#include <stdint.h>\n"
	      "\n", output);
	fputs(headers, output);
	fputc('\n', output);

	fputs(free_contents, output);
	fputc('\n', output);
	fputs(compare, output);
	fputc('\n', output);

	fprintf(output, "\nconst struct ntwt_hashmap %s = {", name);

	fputs("\n.free_contents = free_contents", output);
	fputs(",\n.compare = compare", output);
	fprintf(output, ",\n.bin_num = %u", map->bin_num);
	fprintf(output, ",\n.entry_num = %i", map->entry_num);
	fprintf(output, ",\n.primes_pointer = &(int) { %i }",
		*map->primes_pointer);
	bins_gen(map->bins, output, map->bin_num, key_print, storage_print);

	fputs("\n};", output);

}
