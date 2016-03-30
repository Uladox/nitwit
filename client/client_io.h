#ifndef CLIENT_IO_H
#define CLIENT_IO_H

#include "../shared/socket/socket.h"
#include "../shared/asm/asm_compiler.h"

void compile_and_send(const char *charset, struct ntwt_connection *sock,
		      struct ntwt_asm_program *program,
		      struct ntwt_asm_expr **stack,
		      char **io_buff, size_t *io_size,
		      uint32_t *msg_len);

void prompt(void);

void free_conversions(void);

#endif
