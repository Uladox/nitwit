#include "vm_data.h"

const char *restrict const ntwt_type_name[] = {
	[NTWT_OP_CODE] = "OP_CODE",
	[NTWT_CHAR]    = "CHAR",
	[NTWT_UINT]    = "UINT",
	[NTWT_INT]     = "INT",
	[NTWT_DOUBLE]  = "DOUBLE",
	[NTWT_STRING]  = "STRING"
};

const int *restrict const ntwt_op_args[] = {
	[NTWT_OP_READ]        = (int []) { 0 },
	[NTWT_OP_END]         = (int []) { 0 },
	[NTWT_OP_TEST]        = (int []) { 0 },
	[NTWT_OP_ECHO]        = (int []) { 1, NTWT_STRING },
	[NTWT_OP_EXEC]        = (int []) { 1, NTWT_STRING },
	[NTWT_OP_SAVE]        = (int []) { 0 }
};

const char *restrict const ntwt_op_name[] = {
	[NTWT_OP_READ]        = "READ",
	[NTWT_OP_END]         = "END",
	[NTWT_OP_TEST]        = "TEST",
	[NTWT_OP_ECHO]        = "ECHO",
	[NTWT_OP_EXEC]        = "EXEC",
	[NTWT_OP_SAVE]        = "SAVE"
};
