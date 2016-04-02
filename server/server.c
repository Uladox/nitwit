#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <locale.h>

#define NTWT_SHORT_NAMES
#include "../shared/socket/socket.h"
#include "../shared/vm/vm.h"
#include "server_args.h"

#define IMAGE_OPEN_FAILURE \
	"Error: Failed to open image file, it should be in the current directory or the first command line argument. If it is the former, its name should be state.ilk. Errno: %d (%s)\n"

void *repl_getline(void *input)
{
	struct ntwt_connection *sock;
	struct ntwt_vm_state *state;
	unsigned int size = 256;
	int message_size;
	char *str;

	sock = ((void **) input)[0];
	state = ((void **) input)[1];
	str = malloc(size);

        while (!connection_end_check(sock)) {
		if (connection_read(sock, &str, &size,
				    &message_size, 1) == 1) {
			printf("message size %u\n", message_size);
			str[message_size] = NTWT_OP_END;
			ntwt_interprete(state, str);
		}
        }
	free(str);
        connection_free(sock);
	printf("bye!\n");
	return NULL;
}

static void load_state(struct ntwt_vm_state *state,
		       FILE *image)
{
	long image_size;
	char *image_code;

	/* char a = NTWT_OP_TEST; */
	/* fwrite(&a, sizeof(char), 1, image); */

	fseek(image, 0, SEEK_END);
	image_size = ftell(image);
	printf("%lu\n", image_size);
	rewind(image);
        image_code = malloc(image_size + 1);
	fread(image_code, image_size, sizeof(char), image);
	image_code[image_size] = NTWT_OP_END;
	ntwt_interprete(state, image_code);
	free(image_code);
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	struct ntwt_vm_state state;
	struct ntwt_connecter *find_socket;
	struct ntwt_connection *connect_socket;
	pthread_t user_thread;

	struct ntwt_svr_opts opts = {
	        .load_image = 1,
		.image = "state.ilk",
		.output = "state.ilk"
	};

        ntwt_svr_args_parse(argc, argv, &opts);
	if (opts.load_image) {
		FILE *image = fopen(opts.image, "rb");
		if (!image) {
			fprintf(stderr,
				IMAGE_OPEN_FAILURE,
				errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		load_state(&state, image);
		fclose(image);
	}
	find_socket = connecter_new("echo_socket");
	connect_socket = connecter_accept(find_socket);
	pthread_create(&user_thread, NULL, repl_getline,
		       &(void *[2]) { connect_socket, &state });
	pthread_join(user_thread, NULL);
        connecter_free(find_socket);
	pthread_exit(NULL);
	return 0;
}
