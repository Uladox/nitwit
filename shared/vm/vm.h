/* Include these
 * #include "state.h"
 */

enum ntwt_op_code {
	NTWT_OP_READ,
	NTWT_OP_END,
/* IO operations */
	NTWT_OP_TEST,
	NTWT_OP_ECHO,
	NTWT_OP_EXEC,
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

struct thread_pass;

void ntwt_interprete(struct ntwt_vm_state *restrict state,
		     struct thread_pass *pass,
		     const char *restrict exec_ptr,
		     const char *out_name);
