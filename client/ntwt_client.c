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

#define SOCK_PATH "echo_socket"

int main(void)
{
	setlocale(LC_ALL, "");

	char *path = "echo_socket";
	const char *charset = locale_charset();

	uint32_t msg_len;
	char *io_buff = NULL;
	uint8_t *uni_buff = NULL;
	size_t io_size = 0;
	size_t uni_size = 0;
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
		get_u8(charset, io_buff, msg_len, &uni_buff, &uni_size);
		/* get_encoded(charset, uni_str, uni_size, &str, &size); */
		/* printf("%s\n", str); */

		ntwt_asm_recycle(&stack, program.expr);
		ntwt_asm_statements(&program, &stack, uni_buff);
		ntwt_asm_program_bytecode(&program, &io_buff,
					  &io_size, &msg_len);

		ntwt_connection_send(sock, &msg_len, sizeof(msg_len));
		ntwt_connection_send(sock, io_buff, msg_len);
	}
	putchar('\n');
	free(io_buff);
	free(uni_buff);
	ntwt_connection_free(sock);
	if (program.expr)
		ntwt_asm_expr_free(program.expr);
	return 0;
}

