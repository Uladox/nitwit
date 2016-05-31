/* Include these
 * #include <stdint.h>
 * #include "../vm/state.h"
 * #include "../vm/vm.h"
 * #include "../list/list.h"
 */

struct ntwt_asm_program {
	unsigned int size;
	struct ntwt_asm_expr *expr;
};

struct ntwt_asm_expr {
	struct ntwt_list *next;
	enum ntwt_token type;
	unsigned int lineno;
	unsigned int size;
	union {
		struct ntwt_asm_expr *list;
		uint32_t integer;
		double decimal;
		uint8_t *string;
		char op_code;
	} contents;
};

void ntwt_asm_statements(struct ntwt_asm_program *program,
			 struct ntwt_asm_expr **stack,
			 const uint8_t *code, int *error);

void ntwt_asm_program_type_check(struct ntwt_asm_program *program,
			    int *error);

void ntwt_asm_program_bytecode(struct ntwt_asm_program *program,
			       char **code, size_t *old_size,
			       unsigned int *message_size, int *error);

void ntwt_asm_recycle(struct ntwt_asm_expr **stack,
		      struct ntwt_asm_expr *top);

void ntwt_asm_expr_free(struct ntwt_asm_expr *expr);

void ntwt_asm_stack_free(struct ntwt_asm_expr *stack);

#if defined NTWT_SHORT_NAMES || defined NTWT_ASM_COMPILER_SHORT_NAMES
#define asm_statements(...) ntwt_asm_statements(__VA_ARGS__)
#define asm_program_type_check(...) ntwt_asm_program_type_check(__VA_ARGS__)
#define asm_program_bytecode(...) ntwt_asm_program_bytecode(__VA_ARGS__)
#define asm_recycle(...) ntwt_asm_recycle(__VA_ARGS__)
#define asm_expr_free(...) ntwt_asm_expr_free(__VA_ARGS__)
#define asm_stack_free(...) ntwt_asm_stack_free(__VA_ARGS__)
#endif
