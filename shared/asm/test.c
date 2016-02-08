#include <stdlib.h>
#include <stdio.h>
#include "ntwt_asm_compiler.h"
#include "../interpreter/ntwt_interpreter.h"

void main(void)
{
	struct ntwt_asm_tree *tree =
		statements("TEST; TEST 365.24 \"hello, world\"; TEST;");
	/* TEST 1.0 \"hello, world\"; */
	/* char *a = ";"; */
	/* tree = statements(a); */
        struct ntwt_asm_tree *t;
	for (t = tree; t; t = t->next) {
		printf("command code: %u\n", t->type);
		struct ntwt_asm_tree *line;
		for (line = t->contents.branch; line; line = line->next) {
			printf("line %u\n", line->type);
			if (line->type == NTWT_DOUBLE)
				printf ("double: %f\n", line->contents.decimal);
			else if (line->type == NTWT_STRING)
				printf ("string: %s\n", line->contents.string);
		}
	}
}
