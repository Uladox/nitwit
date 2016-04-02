#ifndef NTWT_SERVER_ARGS_H
#define NTWT_SERVER_ARGS_H

struct ntwt_svr_opts
{
	int load_image;
	char *image;
	char *output;
};

void ntwt_svr_args_parse(int argc, char **argv, struct ntwt_svr_opts *opts);

#endif
