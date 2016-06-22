#define NTWT_FILE_MAGIC 0xAFAD4BADE6ADC0DE

enum ntwt_op_code {
	NTWT_OP_INVALID,
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
	NTWT_SEMI,
	NTWT_OP_CODE,
	NTWT_UINT,
	NTWT_INT,
	NTWT_DOUBLE,
	NTWT_STRING
};

struct thread_pass;

struct ntwt_vm_state {
	struct thread_pass *pass;
	const char *output;
	const char *input;
};


void ntwt_interprete(struct ntwt_vm_state *restrict state,
		     const char *restrict exec_ptr);
