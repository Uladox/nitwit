#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "../shared/socket/ntwt_socket.h"
#include "../shared/interpreter/ntwt_interpreter.h"

/* void *stuff[] = { */
/* 	[0] = NULL, */
/* 	[1] =  */
/* } */

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



void *echo_socket(void *input)
{
	struct ntwt_connection *sock;
	unsigned int size = 256;
	int message_size;
	char **str;

	struct ntwt_practise p[100];
	char stack[100];

	ntwt_practise_load(p, yell, 0.05, 0.005, 0.007);

	sock = input;
	str = malloc(sizeof(char *));
	*str = malloc(size);

        while (!ntwt_connection_end_check(sock)) {
		if (ntwt_connection_read
		    (sock, str, &size, &message_size) == 1) {
			ntwt_interprete(*str, stack, p);
		}
        }
	free(*str);
	free(str);
	ntwt_connection_free(sock);
	printf("bye!\n");
	return NULL;
}

int main(int argc, char **args)
{
	/* struct ntwt_practise p[100]; */
	struct ntwt_connecter *find_socket;
	struct ntwt_connection *connect_socket;
	pthread_t user_thread;

	find_socket = ntwt_connecter_new("echo_socket");
        connect_socket = ntwt_connecter_accept(find_socket);
	pthread_create(&user_thread, NULL, echo_socket, (void *) connect_socket);

	/* sleep(3); */
	/* ntwt_connection_kill(connect_socket); */
	/* char code[] = { */
	/* 	CONTEXT, */
	/* 	PRAC_1, */
	/* 	RUN, */
	/* 	END */
	/* }; */
	/* char stack[100]; */

	/* ntwt_practise_load(p, yell, 0.05, 0.005, 0.007); */

	/* ntwt_interprete(code, stack, p); */

	/* ntwt_practise_run(p); */

	pthread_join(user_thread, NULL);
	ntwt_connecter_free(find_socket);
	pthread_exit(NULL);
	return 0;
}
