/* Include these
 * #include <stdint.h>
 * #include "../vm/state.h"
 * #include "../vm/vm.h"
 * #include "../vm/vm_data.h"
 */

struct ntwt_asm_lex_info {
	const uint8_t *lexme;
	enum ntwt_token token;
	size_t lexlen;
	size_t units;
	unsigned int lexme_lineno;
	unsigned int lineno;
	unsigned int offset;
	struct ntwt_asm_expr **cmds;
	struct ntwt_asm_expr **trms;
	int *error;

};

