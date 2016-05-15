#define _DEFAULT_SOURCE
#include <endian.h>
#include <errno.h>
#include <locale.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <uniconv.h>
#include <unistd.h>

#define NTWT_SHORT_NAMES
#include "../shared/socket/socket.h"
#include "../shared/vm/state.h"
#include "../shared/vm/vm.h"
#include "../shared/asm/compiler.h"
#include "client_args.h"
#include "client_io.h"


static void
repl(const char *charset, struct ntwt_asm_expr **stack,
     struct ntwt_asm_program *program)
{
	char *path = "echo_socket";
	uint32_t msg_len;
	char *io_buff = NULL;
	size_t io_size = 0;
	struct ntwt_connection *sock = connection_connect(path);

	if (!sock)
		return;

	while (!connection_end_check(sock)) {
		ssize_t tmp;

		prompt();

		/* tmp holds -1 if an error occurs or C-d is pressed, so we
		 * exit the program if that happens, however, we need a
		 * constant sized unsigned value to send over to the server
		 * so we convert it to uint32_t by assigning it to msg_len.
		 */
		tmp = getline(&io_buff, &io_size, stdin);

		if (tmp == -1)
			break;

		msg_len = tmp + 1;

		/* Replaces '\n' with '\0' */
		io_buff[tmp - 1] = '\0';
		compile_and_send(charset, sock, program, stack,
				 &io_buff, &io_size, &msg_len);
	}

	putchar('\n');
	free(io_buff);
	free_conversions();
	connection_free(sock);
	asm_expr_free(program->expr);
	asm_stack_free(*stack);
}

static void
file_error(const char *filename)
{
	fprintf(stderr,
		"Error: file \"%s\" not found\n",
		filename);
	exit(EXIT_FAILURE);
}


static void
compile_out(FILE *out,
	    struct ntwt_asm_expr **stack,
	    struct ntwt_asm_program *program,
	    uint32_t *msg_len,
	    char **io_buff, size_t *io_size)
{
	int error = 0;

	asm_statements(program, stack, (uint8_t *) *io_buff, &error);
	asm_program_type_check(program, &error);

	if (error)
		goto cleanup;

	asm_program_bytecode(program, io_buff, io_size, msg_len, &error);

	if (error)
		goto cleanup;

	fwrite(&(uint64_t) { htobe64(NTWT_FILE_MAGIC) },
	       sizeof(uint64_t), 1, out);
	if (*msg_len > 0)
		fwrite(*io_buff, sizeof(char), *io_size, out);
cleanup:
	fclose(out);
	free(*io_buff);
	asm_expr_free(program->expr);
	asm_stack_free(*stack);

	if (error)
		exit(EXIT_FAILURE);
}

static void
compile_read(struct ntwt_asm_expr **stack,
	     struct ntwt_asm_program *program,
	     FILE *src, FILE *out)
{
	uint32_t msg_len;
	char *io_buff;
	size_t io_size = 0;

	fseek(src, 0, SEEK_END);
	io_size = ftell(src) + 1;
	rewind(src);
	io_buff = malloc(io_size);
	fread(io_buff, io_size, sizeof(char), src);
	io_buff[io_size - 1] = '\0';
	msg_len = io_size;
	fclose(src);

	compile_out(out, stack, program, &msg_len, &io_buff, &io_size);
}

static void
compile(struct ntwt_asm_expr **stack,
	struct ntwt_asm_program *program,
	char *input, char *output)
{
	FILE *src = fopen(input, "rb");

	if (!src)
		file_error(input);

	remove(output);
	FILE *out = fopen(output, "ab");

	if (!out) {
		fclose(src);
		file_error(output);
	}

	compile_read(stack, program, src, out);
}

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	const char *charset = locale_charset();
	struct ntwt_asm_expr *stack = NULL;
	struct ntwt_asm_program program = {
		.expr = NULL
	};

	struct ntwt_clnt_opts opts = {
		.compile = 0,
		.input = NULL,
		.out_set = 0,
		.output = "state.ilk"
	};

	ntwt_clnt_args_parse(argc, argv, &opts);


	if (opts.compile) {
		compile(&stack, &program, opts.input, opts.output);
	} else if (opts.out_set) {
		fprintf(stderr,
			"Error: no input file specified for output \"%s\"\n",
			opts.output);
		exit(EXIT_FAILURE);
	} else {
		repl(charset, &stack, &program);
	}



	return 0;
}

