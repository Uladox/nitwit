#include <stdio.h>
#include <stdlib.h>
#include <unistr.h>

#include "ntwt_op_map.h"

static int compare(const void *entry_key, uint32_t entry_key_size,
		   const void *key, uint32_t key_size)
{
	if (entry_key_size != key_size)
		return 0;
	else
		return !u8_strncmp(key, entry_key, key_size);
}

static void free_contents(void *key, void *storage)
{
	fprintf(stderr, "Error: Tried to free op map!\n");
	exit(1);
}

struct ntwt_hashmap ntwt_op_map = {
	.compare = compare,
	.free_contents = free_contents,
