#include "ntwt_socket.h"

struct ntwt_connecter *ntwt_connecter_new(char *path)
{
	int len;
	struct ntwt_connecter *cntr;

	cntr = malloc(sizeof(*cntr));

	if ((cntr->sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	cntr->socket.sun_family = AF_UNIX;
	strcpy(cntr->socket.sun_path, path);
	unlink(path);
	len = strlen(path) + sizeof(cntr->socket.sun_family);

	if (bind(cntr->sd, (struct sockaddr *)&cntr->socket, len) == -1) {
		perror("bind");
		exit(1);
	}

	return cntr;
}

void ntwt_connecter_free(struct ntwt_connecter *cntr)
{
	close(cntr->sd);
	free(cntr);
}

struct ntwt_connection *ntwt_connection_connect(char *path)
{
	struct ntwt_connection *cntn;

	cntn = malloc(sizeof(struct ntwt_connection));

	if ((cntn->sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	cntn->socket.sun_family = AF_UNIX;
	strcpy(cntn->socket.sun_path, path);

	cntn->len = strlen(path) + sizeof(cntn->socket.sun_family);

	pthread_mutex_init(&cntn->end_mutex, NULL);
	pthread_mutex_init(&cntn->done_mutex, NULL);

	if (connect(cntn->sd, (struct sockaddr *)&cntn->socket,
		    cntn->len) == -1) {
		perror("connect");
		exit(1);
	}

	cntn->end_bool = 0;

	return cntn;
}

struct ntwt_connection *ntwt_connecter_accept(struct ntwt_connecter *cntr)
{
	struct ntwt_connection *remote_cntr;

	if (listen(cntr->sd, 5) == -1) {
		perror("listen");
		exit(1);
	}

	remote_cntr = malloc(sizeof(struct ntwt_connection));

	pthread_mutex_init(&remote_cntr->end_mutex, NULL);
	pthread_mutex_init(&remote_cntr->done_mutex, NULL);

	remote_cntr->len = sizeof(struct sockaddr);
	remote_cntr->sd = accept(cntr->sd,
				 (struct sockaddr *) &remote_cntr->socket,
				 &remote_cntr->len);
	if (remote_cntr->sd == -1) {
		perror("accept");
		exit(1);
        }
	remote_cntr->end_bool = 0;

	return remote_cntr;
}

void ntwt_connection_free(struct ntwt_connection *cntn)
{
	pthread_mutex_destroy(&cntn->end_mutex);
        pthread_mutex_destroy(&cntn->done_mutex);
	close(cntn->sd);
	free(cntn);
}

int ntwt_connection_end_check(struct ntwt_connection *cntn)
{
	int value;
	pthread_mutex_lock(&cntn->end_mutex);
	value = cntn->end_bool;
	pthread_mutex_unlock(&cntn->end_mutex);
	return value;
}

void ntwt_connection_end_mutate(struct ntwt_connection *cntn, int value)
{
	pthread_mutex_lock(&cntn->end_mutex);
	cntn->end_bool = value;
	pthread_mutex_unlock(&cntn->end_mutex);
}

void ntwt_connection_kill(struct ntwt_connection *user_cntn)
{
	int true_val = 1;
	pthread_mutex_lock(&user_cntn->done_mutex);
	ntwt_connection_end_mutate(user_cntn, 1);
	setsockopt(user_cntn->sd, SOL_SOCKET,SO_REUSEADDR,
		   &true_val, sizeof(int));
	pthread_mutex_unlock(&user_cntn->done_mutex);
}

int ntwt_connection_read(struct ntwt_connection *cntn,
			 char **str, unsigned int *old_size,
			 int *message_size, unsigned int offset)
{
	int retval;
	unsigned int size = 0;
	unsigned int offset_size = offset;

	pthread_mutex_lock(&cntn->done_mutex);

	FD_ZERO(&cntn->set);
	FD_SET(cntn->sd,
	       &cntn->set);
	cntn->timeout.tv_sec = 0;
	cntn->timeout.tv_usec = 0;

	retval = select(FD_SETSIZE, &cntn->set, NULL, NULL,
			&cntn->timeout);
	if (retval == 1) {

		recv(cntn->sd, &size, sizeof(unsigned int), 0);
		offset_size += size;
		/* printf("I got the number: %u\n", size); */
		if (offset_size > *old_size) {
			free(*str);
			*str = malloc(offset_size);
			*old_size = offset_size;
		}

		*message_size = recv(cntn->sd, *str, size, 0);
		/* printf("and the string, %.*s", size, *str); */

		if (*message_size <= 0) {
			if (*message_size < 0)
				perror("recv");
			ntwt_connection_end_mutate(cntn, 1);
		}
	}

	pthread_mutex_unlock(&cntn->done_mutex);
	return retval;
}

void ntwt_connection_send(struct ntwt_connection *cntn,
			  char *str, unsigned int message_size)
{
	if (!ntwt_connection_end_check(cntn))
		if (send(cntn->sd, str, message_size, MSG_NOSIGNAL) < 0) {
			perror("send");
			ntwt_connection_end_mutate(cntn, 1);
		}
}
