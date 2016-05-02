/* Include these
 * #include <pthread.h>
 * #include <stdint.h>
 * #include <sys/select.h>
 * #include <sys/socket.h>
 * #include <sys/un.h>
 */

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

#if defined NTWT_SHORT_NAMES || defined NTWT_SOCKET_SHORT_NAMES
#define connecter_new(...) ntwt_connecter_new(__VA_ARGS__)
#define connecter_free(...) ntwt_connecter_free(__VA_ARGS__)
#define connection_connect(...) ntwt_connection_connect(__VA_ARGS__)
#define connecter_accept(...) ntwt_connecter_accept(__VA_ARGS__)
#define connection_free(...) ntwt_connection_free(__VA_ARGS__)
#define connection_end_check(...) ntwt_connection_end_check(__VA_ARGS__)
#define connection_end_mutate(...) ntwt_connection_end_mutate(__VA_ARGS__)
#define connection_kill(...) ntwt_connection_kill(__VA_ARGS__)
#define connection_read(...) ntwt_connection_read(__VA_ARGS__)
#define connection_send(...) ntwt_connection_send(__VA_ARGS__)
#endif
