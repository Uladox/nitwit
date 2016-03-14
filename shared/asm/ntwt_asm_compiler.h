#ifndef NTWT_ASM_COMPILER_H
#define NTWT_ASM_COMPILER_H

#include <stdlib.h>
#include <stdint.h>

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
		uint8_t *string;
		char op_code;
	} contents;
};

struct ntwt_lex_info {
	const uint8_t *lexme;
	size_t lexlen;
	size_t units;
	unsigned int lineno;
	unsigned int token;
	unsigned int offset;
};


void ntwt_asm_statements(struct ntwt_asm_program *program,
			 struct ntwt_asm_expr **stack,
			 const uint8_t *code);

void ntwt_asm_program_bytecode(struct ntwt_asm_program *program,
			       char **code, size_t *old_size,
			       unsigned int *message_size);

void ntwt_asm_recycle(struct ntwt_asm_expr **stack,
		      struct ntwt_asm_expr *top);

void ntwt_asm_expr_free(struct ntwt_asm_expr *expr);

#endif
