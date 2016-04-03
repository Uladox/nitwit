#ifndef NTWT_VM_STATE_H
#define NTWT_VM_STATE_H

#include "../practise/practise.h"

struct ntwt_vm_state {
	unsigned int package_ptr;
	unsigned int package_max;
	struct ntwt_package *packages;

	struct ntwt_practise *context;

	unsigned int practise_max;
	struct ntwt_practise *practises;
	pthread_t awareness;
};

void ntwt_vm_state_load_package(struct ntwt_vm_state *state,
				uint32_t package_num,
				uint32_t action_max,
				uint8_t *location);

void ntwt_vm_save(struct ntwt_vm_state *state, const char *img_name);

#endif
