#include <argp.h>
#include <stdlib.h>

#include "client_args.h"

const char *argp_program_version = "Not even close to ready";
const char *argp_program_bug_address = "<uladox@tuta.io>";

/* Program documentation. */
static char doc[] = "The client of nitwit.";

/* A description of the arguments we accept. */
static char args_doc[] = { 0 };

/* The options we understand. */
static struct argp_option options[] = {
	{ "compile",  'c', "FILE", 0,
	  "Compiles an nitwit assembly file to bytecode" },
	{ "out", 'o', "FILE", 0,
	  "Specifies the name of the output file, else it is state.ilk" },
	{ 0 }
};


/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	/* Get the input argument from argp_parse, which we
	 * know is a pointer to our arguments structure.
	 */
	struct ntwt_clnt_opts *opts = state->input;

	switch (key) {
	case 'c':
		opts->compile = 1;
		opts->input = arg;
		break;
	case 'o':
		opts->out_set = 1;
		opts->output = arg;
		break;
	case ARGP_KEY_ARG:
		argp_usage(state);
		break;
	case ARGP_KEY_END:
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

void ntwt_clnt_args_parse(int argc, char **argv, struct ntwt_clnt_opts *opts)
{
	argp_parse(&argp, argc, argv, 0, 0, opts);
}
