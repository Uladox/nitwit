#include <stdlib.h>
#include <stdio.h>
#include "ntwt_asm_compiler.h"
#include "../interpreter/ntwt_interpreter.h"

void main(void)
{
	struct ntwt_asm_tree *program = statements("TEST;");
	printf("program size: %u\n", program->size);
	/* TEST 1.0 \"hello, world\"; */
	/* char *a = ";"; */
	/* tree = statements(a); */
	struct ntwt_asm_tree *command;
	for (command = program->contents.branch;
	     command; command = command->next) {
		printf("command code: %u\n", command->type);
		printf("command size: %u\n", command->size);
		struct ntwt_asm_tree *term;
		for (term = command->contents.branch;
		     term; term = term->next) {
			printf("term %u\n", term->type);
			printf("term size: %u\n", term->size);
			if (term->type == NTWT_DOUBLE)
				printf ("double: %f\n", term->contents.decimal);
			else if (term->type == NTWT_STRING)
				printf ("string: %s\n", term->contents.string);
			else if (term->type == NTWT_OP_CODE)
				if (term->contents.op_code == NTWT_OP_TEST)
					printf("test\n");
		}
	}
	remove("state.ilk");
	FILE *image = fopen("state.ilk", "ab");
	char *code = ntwt_asm_program_bytecode(program);
        fwrite(code, sizeof(char), program->size, image);
	/* char op[1] = { NTWT_OP_END }; */
	/* fwrite(op, sizeof(char), 1, image); */
	fclose(image);
        /* struct ntwt_asm_tree *t; */
	/* for (t = tree; t; t = t->next) { */
	/* 	printf("command code: %u\n", t->type); */
	/* 	struct ntwt_asm_tree *term; */
	/* 	for (term = t->contents.branch; term; term = term->next) { */
	/* 		printf("term %u\n", term->type); */
	/* 		if (term->type == NTWT_DOUBLE) */
	/* 			printf ("double: %f\n", term->contents.decimal); */
	/* 		else if (term->type == NTWT_STRING) */
	/* 			printf ("string: %s\n", term->contents.string); */
	/* 	} */
	/* } */
}
