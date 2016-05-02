/* Include these
 * #include <stdint.h>
 * #include "../vm/state.h"
 * #include "../vm/vm.h"
 * #include "../vm/vm_data.h"
 * #include "lex_info.h"
 */

void ntwt_asm_lex(struct ntwt_asm_lex_info *info);

#if defined NTWT_SHORT_NAMES || defined NTWT_ASM_LEX_SHORT_NAMES
#define asm_lex(...) ntwt_asm_lex(__VA_ARGS__)
#endif

