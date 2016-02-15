#include "ntwt_socket.h"

struct ntwt_connecter *ntwt_connecter_new(char *path)
{
	int len;
	struct ntwt_connecter *sock;

	sock = malloc(sizeof(struct ntwt_connecter));

	if ((sock->sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	sock->socket.sun_family = AF_UNIX;
	strcpy(sock->socket.sun_path, path);
	unlink(path);
	len = strlen(path) + sizeof(sock->socket.sun_family);

	if (bind(sock->sd, (struct sockaddr *)&sock->socket, len) == -1) {
		perror("bind");
		exit(1);
	}

	return sock;
}

void ntwt_connecter_free(struct ntwt_connecter *sock)
{
	close(sock->sd);
	free(sock);
}

struct ntwt_connection *ntwt_connection_connect(char *path)
{
	struct ntwt_connection *sock;

	sock = malloc(sizeof(struct ntwt_connection));

	if ((sock->sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	sock->socket.sun_family = AF_UNIX;
	strcpy(sock->socket.sun_path, path);

	sock->len = strlen(path) + sizeof(sock->socket.sun_family);

	pthread_mutex_init(&sock->end_mutex, NULL);
	pthread_mutex_init(&sock->done_mutex, NULL);

	if (connect(sock->sd, (struct sockaddr *)&sock->socket,
		    sock->len) == -1) {
		perror("connect");
		exit(1);
	}

	sock->end_bool = 0;

	return sock;
}

struct ntwt_connection *ntwt_connecter_accept(struct ntwt_connecter *sock)
{
	struct ntwt_connection *remote_sock;

	if (listen(sock->sd, 5) == -1) {
		perror("listen");
		exit(1);
	}

	remote_sock = malloc(sizeof(struct ntwt_connection));

	pthread_mutex_init(&remote_sock->end_mutex, NULL);
	pthread_mutex_init(&remote_sock->done_mutex, NULL);

	remote_sock->len = sizeof(struct sockaddr);
	remote_sock->sd = accept(sock->sd,
				 (struct sockaddr *) &remote_sock->socket,
				 &remote_sock->len);
	if (remote_sock->sd == -1) {
		perror("accept");
		exit(1);
        }
	remote_sock->end_bool = 0;

	return remote_sock;
}

void ntwt_connection_free(struct ntwt_connection *socket)
{
	pthread_mutex_destroy(&socket->end_mutex);
        pthread_mutex_destroy(&socket->done_mutex);
	close(socket->sd);
	free(socket);
}

int ntwt_connection_end_check(struct ntwt_connection *socket)
{
	int value;
	pthread_mutex_lock(&socket->end_mutex);
	value = socket->end_bool;
	pthread_mutex_unlock(&socket->end_mutex);
	return value;
}

void ntwt_connection_end_mutate(struct ntwt_connection *socket, int value)
{
	pthread_mutex_lock(&socket->end_mutex);
	socket->end_bool = value;
	pthread_mutex_unlock(&socket->end_mutex);
}

void ntwt_connection_kill(struct ntwt_connection *user_socket)
{
	int true_val = 1;
	pthread_mutex_lock(&user_socket->done_mutex);
	ntwt_connection_end_mutate(user_socket, 1);
	setsockopt(user_socket->sd, SOL_SOCKET,SO_REUSEADDR,
		   &true_val, sizeof(int));
	pthread_mutex_unlock(&user_socket->done_mutex);
}

int ntwt_connection_read(struct ntwt_connection *sock,
			 char **str, unsigned int *old_size,
			 int *message_size, unsigned int offset)
{
	int retval;
	unsigned int size = 0;
	unsigned int offset_size = offset;

	pthread_mutex_lock(&sock->done_mutex);

	FD_ZERO(&sock->set);
	FD_SET(sock->sd,
	       &sock->set);
	sock->timeout.tv_sec = 0;
	sock->timeout.tv_usec = 0;

	retval = select(FD_SETSIZE, &sock->set, NULL, NULL,
			&sock->timeout);
	if (retval == 1) {

		recv(sock->sd, &size, sizeof(unsigned int), 0);
		offset_size += size;
		/* printf("I got the number: %u\n", size); */
		if (offset_size > *old_size) {
			free(*str);
			*str = malloc(offset_size);
			*old_size = offset_size;
		}

		*message_size = recv(sock->sd, *str, size, 0);
		/* printf("and the string, %.*s", size, *str); */

		if (*message_size <= 0) {
			if (*message_size < 0)
				perror("recv");
			ntwt_connection_end_mutate(sock, 1);
		}
	}

	pthread_mutex_unlock(&sock->done_mutex);
	return retval;
}

void ntwt_connection_send(struct ntwt_connection *sock,
			  char *str, unsigned int message_size)
{
	if (!ntwt_connection_end_check(sock))
		if (send(sock->sd, str, message_size, MSG_NOSIGNAL) < 0) {
			perror("send");
			ntwt_connection_end_mutate(sock, 1);
		}
}
