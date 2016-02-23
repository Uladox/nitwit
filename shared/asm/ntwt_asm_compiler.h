#ifndef NTWT_ASM_COMPILER_H
#define NTWT_ASM_COMPILER_H

#include <stdlib.h>

struct ntwt_asm_tree {
	unsigned int lineno;
	unsigned int type;
	unsigned int size;
	struct ntwt_asm_tree *next;
	union {
		struct ntwt_asm_tree *branch;
		unsigned int integer;
		double decimal;
		char *string;
		char op_code;
	} contents;
};

struct ntwt_lex_info {
	char *lexme;
	unsigned int lexlen;
	unsigned int lineno;
	unsigned int token;
	unsigned int offset;
};

struct ntwt_asm_tree *ntwt_asm_statements(char *code);

void ntwt_asm_program_bytecode(struct ntwt_asm_tree *program,
			       char **code, size_t *old_size,
			       unsigned int *message_size);

#endif
