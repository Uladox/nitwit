/* Include these
 * #include <stdlib.h> or <stddef.h>
 * #include <stdint.h>
 * #include <spar/core.h>
 * #include <nitlib/list.h>
 * #include "../vm/vm.h"
 */

struct nnn_prog {
	size_t size;
	int parsed;
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
	     struct spar_token *token);
