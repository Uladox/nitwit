#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ntwt_interpreter.h"

#include "macros/define.h"

/* Included functions stored in other files for keeping this file clean */
#include "subfuncts/saving.c"
#include "subfuncts/threaded.c"

void ntwt_interprete(struct ntwt_instance *state, const char code[])
{

	const char *restrict exec_ptr = code;

	static const void *restrict const dtable[] = {
		[NTWT_OP_READ]        = &&s_read,
		[NTWT_OP_END]         = &&s_end,
		[NTWT_OP_CONTEXT]     = &&s_context,
		[NTWT_OP_TEST]        = &&s_test,
		[NTWT_OP_AWAKE]       = &&s_awake,
		[NTWT_OP_INIT_PRAC]   = &&s_init_prac,
		[NTWT_OP_LOAD_PRAC]   = &&s_load_prac,
		[NTWT_OP_ACTION]      = &&s_action,
		[NTWT_OP_STRENGTH]    = &&s_strength,
		[NTWT_OP_CAN_HAPPEN]  = &&s_can_happen,
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
