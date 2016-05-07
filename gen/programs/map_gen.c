#!/usr/bin/c ../../shared/hash/hashmap.c ../../shared/hash/maputils.c -lunistring --

#include <stdint.h>
#include <stdio.h>
#include <unistr.h>

#define NTWT_SHORT_NAMES
#include "../../shared/hash/hashmap.h"
#include "../../shared/hash/maputils.h"
#include "../../shared/vm/state.h"
#include "../../shared/vm/vm.h"

#define QUOTE(...) #__VA_ARGS__
#define MAP_ADD_OP(MAP, KEY, STORAGE)					\
	hashmap_add(MAP, KEY, sizeof(KEY) - 1, &(char){ STORAGE })	\

static const char *compare_str = QUOTE(
	static int
	compare(const void *entry_key, uint32_t entry_key_size,
		const void *key, uint32_t key_size)
	{
		if (entry_key_size != key_size)
			return 0;
		else
			return !u8_strncmp(key, entry_key, key_size);
	});

static int
compare(const void *entry_key, uint32_t entry_key_size,
	const void *key, uint32_t key_size)
{
	if (entry_key_size != key_size)
		return 0;
	else
		return !u8_strncmp(key, entry_key, key_size);
}

static void
free_contents(void *key, void *storage)
{

}

void
key_print(FILE *output, void *key, uint32_t key_size)
{
	fprintf(output, "u8\"%s\"", key);
}

void
storage_print(FILE *output, void *storage)
{
	fprintf(output, "&(char) { %i }", *(char *) storage);
}


int
main(int argc, char **args)
{
	remove("op_map.c");

	FILE *output = fopen("op_map.c", "ab");
	struct ntwt_hashmap *map = hashmap_new(0, compare, free_contents);

	MAP_ADD_OP(map, u8"TEST", NTWT_OP_TEST);
	MAP_ADD_OP(map, u8"END", NTWT_OP_END);
	MAP_ADD_OP(map, u8"ECHO", NTWT_OP_ECHO);
	MAP_ADD_OP(map, u8"EXEC", NTWT_OP_EXEC);
	MAP_ADD_OP(map, u8"SAVE", NTWT_OP_SAVE);

	ntwt_hashmap_gen(map, output, "ntwt_op_map",
			 "#include <unistr.h>\n"
			 "#include \"../../shared/hash/hashmap.h\"\n"
			 "#include \"../../shared/macros.h\"\n",
			 compare_str, key_print, storage_print);

	hashmap_free(map);
	fclose(output);

	return 0;
}
