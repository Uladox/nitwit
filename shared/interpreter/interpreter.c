#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistr.h>
#include <uniconv.h>

#include "interpreter.h"

#include "../unicode/unihelpers.h"

#include "macros/define.h"

/* Included functions stored in other files for keeping this file clean */
#include "subfuncts/saving.c"
#include "subfuncts/threaded.c"

const unsigned int *restrict const ntwt_op_args[] = {
	[NTWT_OP_READ]        = (unsigned int []) { 0 },
	[NTWT_OP_END]         = (unsigned int []) { 0 },
	[NTWT_OP_CONTEXT]     = (unsigned int []) { 1, NTWT_CHAR },
	[NTWT_OP_TEST]        = (unsigned int []) { 0 },
	[NTWT_OP_ECHO]        = (unsigned int []) { 1, NTWT_STRING },
	[NTWT_OP_AWAKE]       = (unsigned int []) { 0 },
	[NTWT_OP_INIT_PRAC]   = (unsigned int []) { 1, NTWT_UINT },
	[NTWT_OP_LOAD_PRAC]   = (unsigned int []) { 6,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_DOUBLE,
						    NTWT_DOUBLE,
						    NTWT_DOUBLE },
	[NTWT_OP_ACTION]      = (unsigned int []) { 2,
						    NTWT_UINT,
						    NTWT_UINT },
	[NTWT_OP_CAN_HAPPEN]  = (unsigned int []) { 1,
						    NTWT_DOUBLE },
	[NTWT_OP_STRENGTH]    = (unsigned int []) { 1,
						    NTWT_DOUBLE },
	[NTWT_OP_UNSATISFIED] = (unsigned int []) { 1,
						    NTWT_DOUBLE },
	[NTWT_OP_RUN]         = (unsigned int []) { 0 },
	[NTWT_OP_STRONGER]    = (unsigned int []) { 0 },
	[NTWT_OP_SAVE]        = (unsigned int []) { 0 },
	[NTWT_OP_INIT_PACK]   = (unsigned int []) { 1,
						    NTWT_UINT },
	[NTWT_OP_LOAD_PACK]   = (unsigned int []) { 3,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_STRING },
	[NTWT_OP_LOAD_ACTION] = (unsigned int []) { 3,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_STRING }
};

void ntwt_interprete(struct ntwt_instance *state, const char code[])
{

	const char *restrict exec_ptr = code;

	static const void *restrict const dtable[] = {
		[NTWT_OP_READ]        = &&s_read,
		[NTWT_OP_END]         = &&s_end,
		[NTWT_OP_CONTEXT]     = &&s_context,
		[NTWT_OP_TEST]        = &&s_test,
		[NTWT_OP_ECHO]        = &&s_echo,
		[NTWT_OP_AWAKE]       = &&s_awake,
		[NTWT_OP_INIT_PRAC]   = &&s_init_prac,
		[NTWT_OP_LOAD_PRAC]   = &&s_load_prac,
		[NTWT_OP_ACTION]      = &&s_action,
		[NTWT_OP_CAN_HAPPEN]  = &&s_can_happen,
		[NTWT_OP_STRENGTH]    = &&s_strength,
		[NTWT_OP_UNSATISFIED] = &&s_unsatisfied,
		[NTWT_OP_RUN]         = &&s_run,
		[NTWT_OP_STRONGER]    = &&s_stronger,
		[NTWT_OP_SAVE]        = &&s_save,
		[NTWT_OP_INIT_PACK]   = &&s_init_pack,
		[NTWT_OP_LOAD_PACK]   = &&s_load_pack,
		[NTWT_OP_LOAD_ACTION] = &&s_load_action
	};

	goto *dtable[(uint8_t) *exec_ptr];

#include "states.c"

}

#include "macros/undef.h"
