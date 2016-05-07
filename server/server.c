#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define NTWT_SHORT_NAMES
#include "../shared/socket/socket.h"
#include "../shared/vm/state.h"
#include "../shared/vm/vm.h"
#include "server_args.h"

#define IMAGE_OPEN_FAILURE						\
	"Error: Failed to open image file, it should be in the current directory or the first command line argument. If it is the former, its name should be state.ilk. Errno: %d (%s)\n"

void *
repl_getline(struct ntwt_connection *sock, struct ntwt_vm_state *state,
	     const char *output)
{
	unsigned int size = 256;
	char *str = malloc(size);

	while (!connection_end_check(sock)) {
		int msg_size;

		if (connection_read(sock, &str, &size, &msg_size, 1) == 1) {
			str[msg_size] = NTWT_OP_END;
			ntwt_interprete(state, str, output);
		}
	}
	free(str);
	connection_free(sock);
	printf("bye!\n");
	return NULL;
}

static void
load_state(struct ntwt_vm_state *state, FILE *image, const char *out_name)
{
	long image_size;
	char *image_code;

	fseek(image, 0, SEEK_END);
	image_size = ftell(image);
	rewind(image);
	image_code = malloc(image_size + 1);
	fread(image_code, image_size, sizeof(char), image);
	image_code[image_size] = NTWT_OP_END;
	ntwt_interprete(state, image_code, out_name);
	free(image_code);
}

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	struct ntwt_vm_state state;
	struct ntwt_connecter *find_socket;
	struct ntwt_connection *connect_socket;

	struct ntwt_svr_opts opts = {
		.load_image = 1,
		.diff_output = 0,
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
		load_state(&state, image, opts.output);
		fclose(image);
	}
	find_socket = connecter_new("echo_socket");
	connect_socket = connecter_accept(find_socket);
	repl_getline(connect_socket, &state, opts.output);
	return 0;
}
