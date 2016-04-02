#ifndef NTWT_VM_H
#define NTWT_VM_H

#include <stdint.h>

#include "state.h"

enum ntwt_op_code { NTWT_OP_READ, NTWT_OP_END, NTWT_OP_CONTEXT,
/* IO operations */
		    NTWT_OP_TEST, NTWT_OP_ECHO,
/* Awareness operations */
		    NTWT_OP_AWAKE,
/* Practise  operations */
		    NTWT_OP_INIT_PRAC,
		    NTWT_OP_LOAD_PRAC,
		    NTWT_OP_ACTION,
		    NTWT_OP_STRENGTH, NTWT_OP_CAN_HAPPEN, NTWT_OP_UNSATISFIED,
		    NTWT_OP_RUN, NTWT_OP_STRONGER,
/* Writing operations */
		    NTWT_OP_SAVE,
/* Packages */
		    NTWT_OP_INIT_PACK, NTWT_OP_LOAD_PACK,
		    NTWT_OP_LOAD_ACTION };

enum ntwt_token { NTWT_EOI, NTWT_SEMICOLON, NTWT_COMMAND, NTWT_OP_CODE,
		  NTWT_CHAR, NTWT_UINT, NTWT_INT, NTWT_DOUBLE, NTWT_STRING };

extern const unsigned int *restrict const ntwt_op_args[];

void ntwt_interprete(struct ntwt_vm_state *restrict state,
		     const char *restrict exec_ptr);

#endif
