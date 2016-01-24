#include <stdio.h>

#include "ntwt_interpreter.h"

#define STATE(x)      s_##x :
#define GOTOSTATE(x)  goto s_##x
#define NEXTSTATE()					\
	do {						\
		++exec_ptr;				\
		goto *dtable[(uint8_t) *exec_ptr];	\
	} while(0)

void ntwt_interprete(const char code[], char stack[],
		     struct ntwt_practise prac[])
{
	struct ntwt_practise *context;

	const char *restrict exec_ptr = code;
	/* char *restrict stack_ptr = stack; */

	static const void *restrict const dtable[] = {
		[READ]    = &&s_read,
		[END]     = &&s_end,
		[CONTEXT] = &&s_context,
		[RUN]     = &&s_run,
		[TEST]    = &&s_test
	};

	goto *dtable[(uint8_t) *exec_ptr];

	STATE(read) {
		NEXTSTATE();
	}
	STATE(end) {
		return;
	}
	STATE(context) {
		++code;
		context = prac + *code;
		NEXTSTATE();
	}
	STATE(run) {
		ntwt_practise_run(context);
		NEXTSTATE();
	}
	STATE(test) {
		printf("this is a test\n");
		NEXTSTATE();
	}
}

#undef STATE
#undef GOTOSTATE
#undef NEXTSTATE
