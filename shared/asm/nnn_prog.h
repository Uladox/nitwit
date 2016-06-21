/* Include these
 * #include <stdlib.h> or <stddef.h>
 * #include <stdint.h>
 * #include <spar/core.h>
 * #include <nitlib/list.h>
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
nnn_prog_empty(struct nnn_prog *prog);

void
nnn_prog_get(struct nnn_prog *prog, uint8_t *code, int *error);

void
nnn_prog_bytecode(struct nnn_prog *prog, struct nnn_bcode *bcode, int *error);

void
nnn_prog_print(struct nnn_prog *prog);
