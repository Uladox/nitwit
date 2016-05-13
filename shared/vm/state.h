struct thread_pass;

struct ntwt_vm_state {
	struct thread_pass *pass;
	const char *output;
	const char *input;
};

void ntwt_vm_save(struct ntwt_vm_state *state, const char *img_name);
