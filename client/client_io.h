/* Include these.
 * #include <nitlib/socket.h>
 * #include "../shared/vm/state.h"
 * #include "../shared/vm/vm.h"
 * #include "../shared/asm/compiler.h"
 */

void compile_and_send(const char *charset, struct nit_connection *sock,
		      struct ntwt_asm_program *program,
		      struct ntwt_asm_expr **stack,
		      char **io_buff, size_t *io_size,
		      uint32_t *msg_len);

void prompt(void);

void free_conversions(void);
