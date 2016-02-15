#ifndef NTWT_SOCKET_H
#define NTWT_SOCKET_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>


struct ntwt_connecter {
	struct sockaddr_un socket;
	unsigned int sd;
};

struct ntwt_connection {
	struct sockaddr_un socket;
	unsigned int sd;

	fd_set set;
	struct timeval timeout;

	pthread_mutex_t done_mutex;

	pthread_mutex_t end_mutex;
	int end_bool;
	unsigned int len;
};

struct ntwt_connecter *ntwt_connecter_new(char *path);

void ntwt_connecter_free(struct ntwt_connecter *sock);

struct ntwt_connection *ntwt_connection_connect(char *path);

struct ntwt_connection *ntwt_connecter_accept(struct ntwt_connecter *sock);

void ntwt_connection_free(struct ntwt_connection *socket);

int ntwt_connection_end_check(struct ntwt_connection *socket);

void ntwt_connection_end_mutate(struct ntwt_connection *socket, int value);

void ntwt_connection_kill(struct ntwt_connection *user_socket);

int ntwt_connection_read(struct ntwt_connection *sock,
			 char **str, unsigned int *old_size,
			 int *message_size, unsigned int offset);

void ntwt_connection_send(struct ntwt_connection *sock,
			  char *str, unsigned int size);

#endif
