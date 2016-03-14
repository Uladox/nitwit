#ifndef NTWT_SOCKET_H
#define NTWT_SOCKET_H

#include <stdint.h>
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

void ntwt_connecter_free(struct ntwt_connecter *cntr);

struct ntwt_connection *ntwt_connection_connect(char *path);

struct ntwt_connection *ntwt_connecter_accept(struct ntwt_connecter *cntr);

void ntwt_connection_free(struct ntwt_connection *cntn);

int ntwt_connection_end_check(struct ntwt_connection *cntn);

void ntwt_connection_end_mutate(struct ntwt_connection *cntn, int value);

void ntwt_connection_kill(struct ntwt_connection *cntn);

int ntwt_connection_read(struct ntwt_connection *cntn,
			 char **str, uint32_t *old_size,
			 int *message_size, uint32_t offset);

void ntwt_connection_send(struct ntwt_connection *cntn,
			  const void *msg, uint32_t msg_size);

#endif
