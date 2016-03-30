#define NTWT_SHORT_NAMES
#include "client_io.h"
#include "../shared/unicode/unihelpers.h"


#if ASSUME_UTF8
void compile_and_send(const char *charset, struct ntwt_connection *sock,
		      struct ntwt_asm_program *program,
		      struct ntwt_asm_expr **stack,
		      char **io_buff, size_t *io_size,
		      uint32_t *msg_len)
{
	asm_recycle(stack, program->expr);
        asm_statements(program, stack, (uint8_t *) *io_buff);
	asm_program_bytecode(program, io_buff, io_size, msg_len);
	connection_send(sock, msg_len, sizeof(*msg_len));
	connection_send(sock, *io_buff, *msg_len);
}

void prompt(void)
{
	printf(u8"📮⬅️");
}

void free_conversions(void)
{

}

#else
#warning "Less efficient if your locale is utf8."
static uint8_t *uni_buff = NULL;
static size_t uni_size = 0;

void compile_and_send(const char *charset, struct ntwt_connection *sock,
		      struct ntwt_asm_program *program,
		      struct ntwt_asm_expr **stack,
		      char **io_buff, size_t *io_size,
		      uint32_t *msg_len)
{
	get_u8(charset, *io_buff, *msg_len, &uni_buff, &uni_size);

	asm_recycle(stack, program->expr);
	asm_statements(program, stack, uni_buff);
	asm_program_bytecode(program, io_buff, io_size, msg_len);
	connection_send(sock, msg_len, sizeof(*msg_len));
	connection_send(sock, *io_buff, *msg_len);
}

void prompt(void)
{
	printf("* ");
}

void free_conversions(void)
{
	free(uni_buff);
}
#endif
