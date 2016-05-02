struct ntwt_svr_opts {
	int load_image;
	int diff_output;
	char *image;
	char *output;
};

void ntwt_svr_args_parse(int argc, char **argv, struct ntwt_svr_opts *opts);
