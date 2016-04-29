#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistr.h>

#include "state.h"
#include "vm.h"

void ntwt_vm_save(struct ntwt_vm_state *state, const char *out_name)
{
	remove(out_name);
	FILE *image = fopen(out_name, "ab");
	static const char c_op = NTWT_OP_END;

	/* save_packages(state, image); */
	/* save_practises(state, image); */
	fwrite(&c_op, sizeof(c_op), 1, image);
	fclose(image);
}
