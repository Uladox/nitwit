#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../shared/socket/ntwt_socket.h"
#include "../shared/interpreter/ntwt_interpreter.h"
#include "../shared/asm/ntwt_asm_compiler.h"

#define SOCK_PATH "echo_socket"

int main(void)
{
	char *path = "echo_socket";
	struct ntwt_connection *sock;
	uint32_t msg_len;
	char *str = NULL;
	size_t size = 0;

	sock = ntwt_connection_connect(path);
	while (!ntwt_connection_end_check(sock)) {
		ssize_t tmp;

	        printf("> ");

		/* tmp holds -1 if an error occurs or C-d is pressed, so we
		 * exit the program if that happens, however, we need a
		 * constant sized unsigned value to send over to the server
		 * so we convert it to uint32_t by assigning it to msg_len.
		 */
		tmp = getline(&str, &size, stdin);
	        if (-1 == tmp)
			break;
		else
			msg_len = tmp + 1;

		/* Replaces '\n' with '\0' */
		str[tmp - 1] = '\0';
		printf("%s\n", str);
		ntwt_asm_program_bytecode(ntwt_asm_statements(str),
					  &str, &size, &msg_len);
		ntwt_connection_send(sock, (char *) &msg_len,
				     sizeof(uint32_t));
		ntwt_connection_send(sock, str, msg_len);
	}
	putchar('\n');
	free(str);
	ntwt_connection_free(sock);

	return 0;
}

