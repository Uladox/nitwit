struct ntwt_asm_tree {
	unsigned int lineno;
	unsigned int type;
	struct ntwt_asm_tree *next;
	union {
		struct ntwt_asm_tree *branch;
		unsigned int integer;
		double decimal;
		char *string;
		char op_code;
	} contents;
};

struct ntwt_lex_info {
	char *lexme;
	unsigned int lexlen;
	unsigned int lineno;
	unsigned int token;
	unsigned int offset;
};

struct ntwt_asm_tree *statements(char *code);

