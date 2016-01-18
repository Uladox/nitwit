#include <stdio.h>

#include "ntwt_interpreter.h"

#define STATE(x)      s_##x :
#define GOTOSTATE(x)  goto s_##x
#define NEXTSTATE()  do { ++exec_ptr; goto *dtable[*exec_ptr]; } while(0)

void ntwt_interprete(const uint_fast8_t code[], uint_fast8_t stack[])
{
	uint_fast8_t char_num;
	uint_fast8_t char_count;

	const uint_fast8_t *restrict exec_ptr = code;
	uint_fast8_t *restrict stack_ptr = stack;

	static const void *restrict const dtable[] = {
		[READ]  = &&s_read,
		[END]   = &&s_end,
		[PRINT] = &&s_print,
		[HI]    = &&s_hi
	};

	STATE(read) {
		NEXTSTATE();
	}
	STATE(end) {
		return;
	}
	STATE(print) {
		char_count = 0;
		char_num = *(stack_ptr - 1);
		stack_ptr -= char_num + 1;
		while (char_num - char_count) {
			printf("%c", *(stack_ptr + char_count));
			++char_count;
		}
		NEXTSTATE();
	}
	STATE(hi) {
		*stack_ptr++ = 'H';
		*stack_ptr++ = 'i';
		*stack_ptr++ = '\n';
		*stack_ptr++ = 3;
		NEXTSTATE();
	}
}

#undef STATE
#undef GOTOSTATE
#undef NEXTSTATE
