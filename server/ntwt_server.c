#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "../shared/socket/ntwt_socket.h"

/* void *stuff[] = { */
/* 	[0] = NULL, */
/* 	[1] =  */
/* } */

struct practise {
	void (*action)(double *,
		       double *,
		       double *);
	double can_happen;
	double strength;
	double unsatisfied;
};

void yell(double *can_happen,
	  double *strength,
	  double *unsatisfied)
{
	if (*can_happen * *strength > (1.0 - *unsatisfied)) {
		printf("YELL!\n");
		*unsatisfied -= (1.0 - *strength);
	} else {
		*unsatisfied += *strength;
	}
}

struct practise *practise_new(void (*action)(double *,
					     double *,
					     double *),
			      double can_happen,
			      double strength,
			      double unsatisfied)
{
	struct practise *p;

	p = malloc(sizeof(struct practise));
	p->action = action;
	p->can_happen = can_happen;
	p->strength = strength;
	p->unsatisfied = unsatisfied;

	return p;
}

void run_practise(struct practise *p)
{
	while (1) {
		usleep(4000);
		p->action(&p->can_happen,
			  &p->strength,
			  &p->unsatisfied);
	}
}



void *echo_socket(void *input)
{
	struct ntwt_connection *sock;
	unsigned int size = 256;
	int message_size;
	char **str;

	sock = input;
	str = malloc(sizeof(char *));
	*str = malloc(size);

        while (!ntwt_connection_end_check(sock)) {
		if (ntwt_connection_read
		    (sock, str, &size, &message_size) == 1) {
			printf("Got string: %s\n", *str);
			send(sock->sd, &message_size,
			     sizeof(unsigned int), MSG_NOSIGNAL);
			/* write(sock->sd, &message_size, sizeof(unsigned int)); */
			ntwt_connection_send(sock, *str,
					     message_size);
		}
        }
	free(*str);
	free(str);
	/* close(sock->sd); */
	printf("bye!\n");
	return NULL;
}

int main(int argc, char **args)
{
	/* struct practise *p; */
	struct ntwt_connecter *find_socket;
	struct ntwt_connection *connect_socket;
	pthread_t user_thread;

	find_socket = ntwt_connecter_new("echo_socket");
        connect_socket = ntwt_connecter_accept(find_socket);
	pthread_create(&user_thread, NULL, echo_socket, (void *) connect_socket);

	/* sleep(3); */
	/* ntwt_connection_kill(connect_socket); */

	/* p = practise_new(yell, 0.05, 0.005, 0.007); */
	/* run_practise(p); */
	pthread_join(user_thread, NULL);
	ntwt_connection_free(connect_socket);
	ntwt_connecter_free(find_socket);
	pthread_exit(NULL);
	return 0;
}
