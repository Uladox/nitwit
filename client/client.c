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

#include <spar/core.h>

#define NIT_SHORT_NAMES
#include <nitlib/list.h>
#include <nitlib/socket.h>

#define NTWT_SHORT_NAMES
#include "client_args.h"
#include "../shared/vm/vm.h"
#include "../shared/asm/nnn_expr.h"
#include "../shared/asm/nnn_prog.h"
#include "client_io.h"

static void
repl(const char *charset, struct nnn_prog *prog)
{
	char *path = "echo_socket";
	struct nnn_bcode bcode = {
		.code = NULL,
		.max = 0
	};

	struct nit_connection *sock = connection_connect(path);

	if (!sock)
		return;

	while (!connection_end_check(sock)) {
		ssize_t tmp;

		prompt();

		tmp = getline(&bcode.code, &bcode.max, stdin);

		if (tmp == -1)
			break;

	        bcode.size = tmp + 1;

		/* Replaces '\n' with '\0' */
		bcode.code[tmp - 1] = '\0';
		compile_and_send(charset, sock, prog, &bcode);
	}

	putchar('\n');
	free(bcode.code);
	free_conversions();
	connection_free(sock);
	nnn_prog_empty(prog);
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
compile_out(FILE *out, struct nnn_prog *prog, struct nnn_bcode *bcode)
{
	int error = 0;

	nnn_prog_get(prog, (uint8_t *) bcode->code, &error);
	nnn_prog_type_check(prog, &error);

	if (error)
		goto cleanup;

	nnn_prog_bytecode(prog, bcode, &error);

	if (error)
		goto cleanup;

	fwrite(&(uint64_t) { htobe64(NTWT_FILE_MAGIC) },
	       sizeof(uint64_t), 1, out);

	if (bcode->size > 0)
		fwrite(bcode->code, sizeof(char), bcode->size, out);
cleanup:
	fclose(out);
	free(bcode->code);
	nnn_prog_empty(prog);

	if (error)
		exit(EXIT_FAILURE);
}

static void
compile_read(struct nnn_prog *prog, FILE *src, FILE *out)
{
	struct nnn_bcode bcode;

	fseek(src, 0, SEEK_END);
	bcode.size = (bcode.max = ftell(src) + 1);
	rewind(src);

	bcode.code = malloc(bcode.size);
	fread(bcode.code, bcode.size, sizeof(char), src);
	bcode.code[bcode.size - 1] = '\0';

	fclose(src);

	compile_out(out, prog, &bcode);
}

static void
compile(struct nnn_prog *prog, char *input, char *output)
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

	compile_read(prog, src, out);
}

int
main(int argc, char *argv[])
{
	setlocale(LC_ALL, "");

	const char *charset = locale_charset();

        NNN_PROG_INIT(prog);

	struct ntwt_clnt_opts opts = {
		.compile = 0,
		.input = NULL,
		.out_set = 0,
		.output = "state.ilk"
	};

	ntwt_clnt_args_parse(argc, argv, &opts);


	if (opts.compile) {
		compile(&prog, opts.input, opts.output);
	} else if (opts.out_set) {
		fprintf(stderr,
			"Error: no input file specified for output \"%s\"\n",
			opts.output);
		exit(EXIT_FAILURE);
	} else {
		repl(charset, &prog);
	}



	return 0;
}

