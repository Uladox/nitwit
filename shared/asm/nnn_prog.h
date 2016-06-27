/* Include these
 * #include <stdlib.h> or <stddef.h>
 * #include <stdint.h>
 * #include <spar/core.h>
 * #include <nitlib/list.h>
 * #include "../vm/vm.h"
 * #include "nnn_expr.h"
 */

#define NNN_PROG_INIT(NAME)					\
	struct nnn_prog NAME = {				\
		.size = 0,					\
		.expr = NULL,					\
		.stack = NULL					\
	}

struct nnn_bcode {
	char *code;
	size_t max;
	size_t size;
};

void
nnn_prog_push(struct nnn_prog *prog, struct nnn_expr *expr);

struct nnn_expr *
nnn_prog_pop(struct nnn_prog *prog);

void
nnn_prog_empty(struct nnn_prog *prog);

void
nnn_prog_get(struct nnn_prog *prog, uint8_t *code, int *parsed);

void
nnn_prog_bytecode(struct nnn_prog *prog, struct nnn_bcode *bcode, int *parsed);

void
nnn_prog_type_check(struct nnn_prog *prog, int *parsed);

void
nnn_prog_print(struct nnn_prog *prog);
