/* Include these
 * #include <stdlib.h> or <stddef.h>
 * #include <stdint.h>
 * #include <spar/core.h>
 * #include <nitlib/list.h>
 * #include "../vm/state.h"
 * #include "../vm/vm.h"
 */

enum ntwt_token {
	NTWT_EOI,
	NTWT_SEMI,
	NTWT_OP_CODE,
	NTWT_UINT,
	NTWT_INT,
	NTWT_DOUBLE,
	NTWT_STRING
};

enum ntwt_op_code {
	NTWT_OP_INVALID,
	NTWT_OP_READ,
	NTWT_OP_END,
/* IO operations */
	NTWT_OP_TEST,
	NTWT_OP_ECHO,
	NTWT_OP_EXEC,
/* Writing operations */
	NTWT_OP_SAVE
};

struct nnn_prog {
	size_t size;
	struct nnn_expr *expr;
	struct nnn_expr *stack;
};

struct nnn_expr {
	struct nit_list next;
	enum ntwt_token type;
	size_t size;
	size_t line;
	union {
		uint32_t integer;
		double decimal;
		char *string;
		char op_code;
	} dat;
};

struct nnn_expr *
nnn_expr_get(struct nnn_prog *prog, struct spar_lexinfo *info,
	     struct spar_token *token, enum spar_parsed *error);
