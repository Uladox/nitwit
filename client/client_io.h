/* Include these.
 * #include <stdlib.h> or <stddef.h>
 * #include <stdint.h>
 * #include <spar/core.h>
 * #include <nitlib/socket.h>
 * #include <nitlib/list.h>
 * #include "../shared/asm/nnn_expr.h"
 * #include "../shared/asm/nnn_prog.h"
 */

void compile_and_send(const char *charset, struct nit_connection *sock,
		      struct nnn_prog *prog, struct nnn_bcode *bcode);

void prompt(void);

void free_conversions(void);
