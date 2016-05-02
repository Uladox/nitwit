struct ntwt_vm_state {
	int data;
};

void ntwt_vm_save(struct ntwt_vm_state *state, const char *img_name);
