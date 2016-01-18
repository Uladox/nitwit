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
	/* int s, t, len; */
	/* unsigned int size; */
        /* int message_size = 100; */
	/* struct sockaddr_un remote; */
	/* char *str = malloc(256); */
	/* size = 256; */

	/* if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) { */
	/* 	perror("socket"); */
	/* 	exit(1); */
	/* } */

	/* printf("Trying to connect...\n"); */

	/* remote.sun_family = AF_UNIX; */
	/* strcpy(remote.sun_path, SOCK_PATH); */
	/* len = strlen(remote.sun_path) + sizeof(remote.sun_family); */
	/* if (connect(s, (struct sockaddr *)&remote, len) == -1) { */
	/* 	perror("connect"); */
	/* 	exit(1); */
	/* } */

	/* printf("Connected.\n"); */

	/* while(printf("> "), fgets(str, 100, stdin), !feof(stdin)) { */
	/* 	size = strlen(str); */
	/* 	send(s, &size, sizeof(unsigned int), 0); */
	/* 	if (send(s, str, size, 0) == -1) { */
	/* 		perror("send"); */
	/* 		exit(1); */
	/* 	} */

	/* 	if ((t=recv(s, str, 100, 0)) > 0) { */
	/* 		str[t] = '\0'; */
	/* 		printf("echo> %s", str); */
	/* 	} else { */
	/* 		if (t < 0) perror("recv"); */
	/* 		else printf("Server closed connection\n"); */
	/* 		exit(1); */
	/* 	} */
	/* } */
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
	/* printf("> "); */
	/* fgets(*str, 100, stdin); */
	/* message_size = strlen(*str) + 1; */
	/* send(sock->sd, &message_size, sizeof(unsigned int), 0); */
	/* ntwt_connection_send(sock, str, message_size); */
	/* while (!end) */
	/* 	if (ntwt_connection_read(sock, str, &size, */
	/* 				 &message_size) == 1) { */
	/* 		printf("echo> %s\n", *str); */
	/* 		end = 1; */
	/* 	} */
	/* close(sock->sd); */

	return 0;
}
