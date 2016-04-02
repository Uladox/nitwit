#include <stdlib.h>
#include <argp.h>

#include "server_args.h"

const char *argp_program_version = "Not even close to ready";
const char *argp_program_bug_address = "<uladox@tuta.io>";

/* Program documentation. */
static char doc[] = "The server of nitwit.";

/* A description of the arguments we accept. */
static char args_doc[] = { 0 };

/* The options we understand. */
static struct argp_option options[] = {
	{"image",  'i', "FILE", 0, "Loads from FILE instead of state.ilk" },
	{"none",   'n', 0,      0, "Does not load a program image."},
	{"output", 'o', "FILE", 0, "Saves to FILE instead of state.ilk" },
	{ 0 }
};


/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct ntwt_svr_opts *opts = state->input;

  switch (key) {
  case 'n':
	  opts->load_image = 0;
	  break;
  case 'i':
	  opts->image = arg;
	  break;
  case 'o':
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

void ntwt_svr_args_parse(int argc, char **argv, struct ntwt_svr_opts *opts)
{
	argp_parse(&argp, argc, argv, 0, 0, opts);
}
