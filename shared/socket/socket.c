#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "socket.h"

struct ntwt_connecter *
ntwt_connecter_new(char *path)
{
	int len;
	struct ntwt_connecter *cntr = malloc(sizeof(*cntr));

	cntr->sd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (cntr->sd == -1) {
		perror("socket");
		free(cntr);
		exit(1);
	}

	cntr->socket.sun_family = AF_UNIX;
	strcpy(cntr->socket.sun_path, path);
	unlink(path);
	len = strlen(path) + sizeof(cntr->socket.sun_family);

	if (bind(cntr->sd, (struct sockaddr *)&cntr->socket, len) == -1) {
		perror("bind");
		free(cntr);
		exit(1);
	}

	return cntr;
}

void
ntwt_connecter_free(struct ntwt_connecter *cntr)
{
	close(cntr->sd);
	free(cntr);
}

struct ntwt_connection *
ntwt_connection_connect(char *path)
{
	struct ntwt_connection *cntn = malloc(sizeof(*cntn));

	cntn->sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (cntn->sd == -1) {
		perror("socket");
		free(cntn);
		return NULL;
	}

	cntn->socket.sun_family = AF_UNIX;
	strcpy(cntn->socket.sun_path, path);

	cntn->len = strlen(path) + sizeof(cntn->socket.sun_family);

	pthread_mutex_init(&cntn->end_mutex, NULL);
	pthread_mutex_init(&cntn->done_mutex, NULL);

	if (connect(cntn->sd, (struct sockaddr *)&cntn->socket,
		    cntn->len) == -1) {
		perror("connect");
		free(cntn);
		return NULL;
	}

	cntn->end_bool = 0;

	return cntn;
}

struct ntwt_connection *
ntwt_connecter_accept(struct ntwt_connecter *cntr)
{
	struct ntwt_connection *cntn;

	if (listen(cntr->sd, 5) == -1) {
		perror("listen");
		return NULL;
	}

	cntn = malloc(sizeof(*cntn));

	cntn->len = sizeof(struct sockaddr);
	cntn->sd = accept(cntr->sd,
			  (struct sockaddr *) &cntn->socket,
			  &cntn->len);
	if (cntn->sd == -1) {
		perror("accept");
		free(cntn);
		return NULL;
	}

	pthread_mutex_init(&cntn->end_mutex, NULL);
	pthread_mutex_init(&cntn->done_mutex, NULL);
	cntn->end_bool = 0;

	return cntn;
}

void
ntwt_connection_free(struct ntwt_connection *cntn)
{
	pthread_mutex_destroy(&cntn->end_mutex);
	pthread_mutex_destroy(&cntn->done_mutex);
	close(cntn->sd);
	free(cntn);
}

int
ntwt_connection_end_check(struct ntwt_connection *cntn)
{
	int value;

	pthread_mutex_lock(&cntn->end_mutex);
	value = cntn->end_bool;
	pthread_mutex_unlock(&cntn->end_mutex);
	return value;
}

void
ntwt_connection_end_mutate(struct ntwt_connection *cntn, int value)
{
	pthread_mutex_lock(&cntn->end_mutex);
	cntn->end_bool = value;
	pthread_mutex_unlock(&cntn->end_mutex);
}

void
ntwt_connection_kill(struct ntwt_connection *cntn)
{
	int true_val = 1;

	pthread_mutex_lock(&cntn->done_mutex);
	ntwt_connection_end_mutate(cntn, 1);
	setsockopt(cntn->sd, SOL_SOCKET, SO_REUSEADDR,
		   &true_val, sizeof(int));
	pthread_mutex_unlock(&cntn->done_mutex);
}

int
ntwt_connection_read(struct ntwt_connection *cntn,
		     char **str, uint32_t *old_size,
		     int *msg_size, uint32_t offset)
{
	int retval;
	uint32_t size = 0;
	uint32_t offset_size = offset;

	pthread_mutex_lock(&cntn->done_mutex);

	FD_ZERO(&cntn->set);
	FD_SET(cntn->sd,
	       &cntn->set);
	cntn->timeout.tv_sec = 0;
	cntn->timeout.tv_usec = 0;

	retval = select(FD_SETSIZE, &cntn->set, NULL, NULL,
			&cntn->timeout);
	if (retval == 1) {

		recv(cntn->sd, &size, sizeof(uint32_t), 0);
		offset_size += size;

		if (offset_size > *old_size) {
			free(*str);
			*str = malloc(offset_size);
			*old_size = offset_size;
		}

		*msg_size = recv(cntn->sd, *str, size, 0);

		if (*msg_size <= 0) {
			if (*msg_size < 0)
				perror("recv");
			ntwt_connection_end_mutate(cntn, 1);
		}
	}

	pthread_mutex_unlock(&cntn->done_mutex);
	return retval;
}

void
ntwt_connection_send(struct ntwt_connection *cntn,
		     const void *msg, uint32_t msg_size)
{
	if (!ntwt_connection_end_check(cntn))
		if (send(cntn->sd, msg, msg_size, MSG_NOSIGNAL) < 0) {
			perror("send");
			ntwt_connection_end_mutate(cntn, 1);
		}
}
