#ifndef NTWT_ASM_COMPILER_H
#define NTWT_ASM_COMPILER_H

#include <stdlib.h>
#include <stdint.h>

#include "../vm/vm.h"

struct ntwt_asm_program {
	unsigned int size;
	struct ntwt_asm_expr *expr;
};

struct ntwt_asm_expr {
	enum ntwt_token type;
	unsigned int lineno;
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

void ntwt_asm_statements(struct ntwt_asm_program *program,
			 struct ntwt_asm_expr **stack,
			 const uint8_t *code, int *error);

void ntwt_asm_program_bytecode(struct ntwt_asm_program *program,
			       char **code, size_t *old_size,
			       unsigned int *message_size);

void ntwt_asm_recycle(struct ntwt_asm_expr **stack,
		      struct ntwt_asm_expr *top);

void ntwt_asm_expr_free(struct ntwt_asm_expr *expr);

void asm_stack_free(struct ntwt_asm_expr *stack);

#if defined NTWT_SHORT_NAMES || defined NTWT_ASM_COMPILER_SHORT_NAMES
#define asm_statements(...) ntwt_asm_statements(__VA_ARGS__)
#define asm_program_bytecode(...) ntwt_asm_program_bytecode(__VA_ARGS__)
#define asm_recycle(...) ntwt_asm_recycle(__VA_ARGS__)
#define asm_expr_free(...) ntwt_asm_expr_free(__VA_ARGS__)
#endif

#endif
