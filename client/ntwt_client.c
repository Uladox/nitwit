#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <locale.h>
#include <uniconv.h>

#include "../shared/unicode/ntwt_unihelpers.h"
#include "../shared/socket/ntwt_socket.h"
#include "../shared/interpreter/ntwt_interpreter.h"
#include "../shared/asm/ntwt_asm_compiler.h"

static void compile_and_send(const char *charset, struct ntwt_connection *sock,
			     struct ntwt_asm_program *program,
			     struct ntwt_asm_expr **stack,
			     char **io_buff, size_t *io_size,
			     uint32_t *msg_len);

int main(void)
{
	setlocale(LC_ALL, "");

	char *path = "echo_socket";
	const char *charset = locale_charset();

	uint32_t msg_len;
	char *io_buff = NULL;
	size_t io_size = 0;

	struct ntwt_connection *sock;
	struct ntwt_asm_expr *stack = NULL;
	struct ntwt_asm_program program = {
		.expr = NULL
	};


	sock = ntwt_connection_connect(path);
	if (!sock)
		return 0;
	while (!ntwt_connection_end_check(sock)) {
		ssize_t tmp;

		printf("> ");

		/* tmp holds -1 if an error occurs or C-d is pressed, so we
		 * exit the program if that happens, however, we need a
		 * constant sized unsigned value to send over to the server
		 * so we convert it to uint32_t by assigning it to msg_len.
		 */
		tmp = getline(&io_buff, &io_size, stdin);
		if (-1 == tmp)
			break;
		msg_len = tmp + 1;

		/* Replaces '\n' with '\0' */
		io_buff[tmp - 1] = '\0';
		compile_and_send(charset, sock, &program, &stack,
				 &io_buff, &io_size, &msg_len);
	}
	putchar('\n');
	free(io_buff);
#if !ASSUME_UTF8
	free(uni_buff);
#endif
	ntwt_connection_free(sock);
	if (program.expr)
		ntwt_asm_expr_free(program.expr);
	return 0;
}

#if ASSUME_UTF8
static void compile_and_send(const char *charset, struct ntwt_connection *sock,
			     struct ntwt_asm_program *program,
			     struct ntwt_asm_expr **stack,
			     char **io_buff, size_t *io_size,
			     uint32_t *msg_len)
{
	ntwt_asm_recycle(stack, program->expr);
	ntwt_asm_statements(program, stack, (uint8_t *) *io_buff);
	ntwt_asm_program_bytecode(program, io_buff,
				  io_size, msg_len);

	ntwt_connection_send(sock, msg_len, sizeof(*msg_len));
	ntwt_connection_send(sock, *io_buff, *msg_len);
}
#else
#warning "Less efficient if your locale is utf8."
uint8_t *uni_buff = NULL;
size_t uni_size = 0;

static void compile_and_send(const char *charset, struct ntwt_connection *sock,
			     struct ntwt_asm_program *program,
			     struct ntwt_asm_expr **stack,
			     char **io_buff, size_t *io_size,
			     uint32_t *msg_len)
{
	get_u8(charset, *io_buff, *msg_len, &uni_buff, &uni_size);

	ntwt_asm_recycle(stack, program->expr);
	ntwt_asm_statements(program, stack, uni_buff);
	ntwt_asm_program_bytecode(program, io_buff,
				  io_size, msg_len);

	ntwt_connection_send(sock, msg_len, sizeof(*msg_len));
	ntwt_connection_send(sock, *io_buff, *msg_len);
}
#endif
