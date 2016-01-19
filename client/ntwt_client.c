#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../shared/socket/ntwt_socket.h"

#define SOCK_PATH "echo_socket"

int main(void)
{
	char *path = "echo_socket";
	struct ntwt_connection *sock;
	unsigned int size = 256;
        int message_size;
	char **str;
	/* int end = 0; */

	str = malloc(sizeof(char *));
	*str = malloc(size);

	sock = ntwt_connection_connect(path);
	while (!ntwt_connection_end_check(sock)) {
	        printf("> ");
		fgets(*str, 100, stdin);
		if (feof(stdin))
			break;
		message_size = strlen(*str);
		(*str)[message_size - 1] = '\0';
		/* send(sock->sd, &message_size, sizeof(unsigned int), 0); */
		ntwt_connection_send(sock, (char *) &message_size,
				     sizeof(unsigned int));
		ntwt_connection_send(sock, *str, message_size);
		while (ntwt_connection_read(sock, str, &size,
					 &message_size) != 1) {
		}
		printf("echo> %s\n", *str);
	}
	free(*str);
	free(str);
	ntwt_connection_free(sock);

	return 0;
}
