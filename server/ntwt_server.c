#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "../shared/socket/ntwt_socket.h"
#include "../shared/interpreter/ntwt_interpreter.h"

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
	char *str;

	/* struct ntwt_practise p[100]; */
	struct ntwt_instance state;

	state.package_max = 1;
	state.practises = calloc(100, sizeof(*state.practises));
	state.practise_max = 100;
	ntwt_practise_load(state.practises,
			   ntwt_std_package.actions,
			   0.05, 0.005, 0.007);
	state.context = NULL;

	sock = input;
	str = malloc(size);

        while (!ntwt_connection_end_check(sock)) {
		if (ntwt_connection_read
		    (sock, &str, &size, &message_size, 1) == 1) {
			printf("message size %u\n", message_size);
			str[message_size] = NTWT_OP_END;
			ntwt_interprete(&state, str);
		}
        }
	free(str);
	ntwt_connection_free(sock);
	printf("bye!\n");
	return NULL;
}

int main(int argc, char *args[])
{
	struct ntwt_instance state;

	struct ntwt_connecter *find_socket;
	struct ntwt_connection *connect_socket;
	pthread_t user_thread;

	FILE *image;
	long image_size;
	char *image_code;

	state.practises = calloc(100, sizeof(struct ntwt_practise));
	ntwt_practise_load(state.practises,
			   ntwt_action_new("yell", 0, 0, yell),
			   0.5, 0.5, 0.5);
	state.context = NULL;

	if (argc >= 2)
		if (!strcmp(args[1], "-none"))
			goto no_file;
		else
			image = fopen(args[1], "wrb");
	else
		image = fopen("state.ilk", "rb");

	if (image == NULL) {
		fprintf(stderr,
			"Error: Failed to open image file, it should be in "
			"the current directory or the first command line "
			"argument. If it is the former, its name should be "
			"state.ilk. Errno: %d (%s)\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fseek(image, 0, SEEK_END);
	image_size = ftell(image);
	rewind(image);
        image_code = malloc(image_size + 1);
	fread(image_code, image_size, sizeof(char), image);
	fclose(image);

	image_code[image_size] = NTWT_OP_END;
	ntwt_interprete(&state, image_code);
	free(image_code);

no_file:

	find_socket = ntwt_connecter_new("echo_socket");
	do
		connect_socket = ntwt_connecter_accept(find_socket);
	while (!connect_socket);

	pthread_create(&user_thread, NULL, echo_socket,
		       (void *) connect_socket);
	pthread_join(user_thread, NULL);
	ntwt_connecter_free(find_socket);
	pthread_exit(NULL);
	return 0;
}
