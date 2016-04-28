#ifndef NTWT_VM_H
#define NTWT_VM_H

#include <stdint.h>

#include "state.h"

enum ntwt_op_code {
	NTWT_OP_READ,
	NTWT_OP_END,
/* IO operations */
	NTWT_OP_TEST,
	NTWT_OP_ECHO,
/* Writing operations */
	NTWT_OP_SAVE
};

enum ntwt_token {
	NTWT_EOI,
	NTWT_SEMICOLON,
	NTWT_COMMAND,
	NTWT_OP_CODE,
	NTWT_CHAR,
	NTWT_UINT,
	NTWT_INT,
	NTWT_DOUBLE,
	NTWT_STRING
};

void ntwt_interprete(struct ntwt_vm_state *restrict state,
		     const char *restrict exec_ptr,
		     const char *out_name);
#endif
