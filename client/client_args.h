struct ntwt_clnt_opts {
	int compile;
	char *input;

	int out_set;
	char *output;
};

void ntwt_clnt_args_parse(int argc, char **argv, struct ntwt_clnt_opts *opts);
