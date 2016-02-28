#ifndef NTWT_ASM_COMPILER_H
#define NTWT_ASM_COMPILER_H

#include <stdlib.h>

struct ntwt_asm_program {
	unsigned int size;
	struct ntwt_asm_expr *expr;
};

struct ntwt_asm_expr {
	unsigned int lineno;
	unsigned int type;
	unsigned int size;
	struct ntwt_asm_expr *next;
	union {
		struct ntwt_asm_expr *list;
		unsigned int integer;
		double decimal;
		char *string;
		char op_code;
	} contents;
};

struct ntwt_lex_info {
	const char *lexme;
	unsigned int lexlen;
	unsigned int lineno;
	unsigned int token;
	unsigned int offset;
};

struct ntwt_asm_program *ntwt_asm_statements(const char *code);

void ntwt_asm_program_bytecode(struct ntwt_asm_program *program,
			       char **code, size_t *old_size,
			       unsigned int *message_size);

#endif
