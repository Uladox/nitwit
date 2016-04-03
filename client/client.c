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

#define NTWT_SHORT_NAMES
#include "client_io.h"

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


	sock = connection_connect(path);
	if (!sock)
		return 0;
	while (!connection_end_check(sock)) {
		ssize_t tmp;

		prompt();

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
	free_conversions();
	connection_free(sock);
	asm_expr_free(program.expr);
	asm_stack_free(stack);
	return 0;
}

