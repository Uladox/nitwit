#include <stdlib.h>
#include <stdio.h>
#include "ntwt_asm_compiler.h"

void main(void)
{
	statements("TEST; TEST 1.0 \"hello, world\";");
	/* struct ntwt_asm_tree *tree; */
	/* char *a = ";"; */
	/* tree = statements(a); */
	/* printf("command code: %u\n", tree->type); */
        /* struct ntwt_asm_tree *t; */
	/* for (t = tree; t; t = t->next) { */
	/* 	printf("command code: %u\n", t->type); */
	/* 	struct ntwt_asm_tree *line; */
	/* 	for (line = t->contents.branch; line; line = line->next) { */
	/* 		printf("line %u\n", line->type); */
	/* 	} */
	/* } */
}
