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

/* void compile(char *input, char *output) */
/* { */
/* 	for(; *input; ++input, ++output) */
/* 	{ */
/* 		switch(*input) { */
/* 		case 'r': */
/* 			*output = NTWT_OP_READ; */
/* 			break; */
/* 		case 'e': */
/* 			*output = NTWT_OP_END; */
/* 			break; */
/* 		case 'c': */
/* 			*output = NTWT_OP_CONTEXT; */
/* 			break; */
/* 		case 'u': */
/* 			*output = NTWT_OP_RUN; */
/* 			break; */
/* 		case 't': */
/* 			*output = NTWT_OP_TEST; */
/* 			break; */
/* 		case 'a': */
/* 			*output = NTWT_OP_AWAKE; */
/* 			break; */
/* 		case 's': */
/* 			*output = NTWT_OP_STRONGER; */
/* 			break; */
/* 		case '0': */
/* 			*output = 0; */
/* 			break; */
/* 		case '!': */
/* 			*output = NTWT_OP_SAVE; */
/* 			break; */
/* 		case '\n': */
/* 			printf("oh no!\n"); */
/* 			break; */
/* 		default: */
/* 			break; */
/* 		} */
/* 	} */
/* } */

int main(void)
{
	char *path = "echo_socket";
	struct ntwt_connection *sock;
	unsigned int size = 256;
        unsigned int message_size;
	unsigned int input_size;
	char *str;
	/* int end = 0; */

	str = malloc(size);

	sock = ntwt_connection_connect(path);
	while (!ntwt_connection_end_check(sock)) {
	        printf("> ");
		fgets(str, 100, stdin);
		/* For C-d */
		if (feof(stdin))
			break;
		input_size = strlen(str);
		/* Replaces '\n' with '\0' */
		(str)[input_size - 1] = '\0';
		ntwt_asm_program_bytecode(ntwt_asm_statements(str),
					  &str, &size, &message_size);
		ntwt_connection_send(sock, (char *) &message_size,
				     sizeof(unsigned int));
		ntwt_connection_send(sock, str, message_size);
	}
	putchar('\n');
	free(str);
	ntwt_connection_free(sock);

	return 0;
}
