#include <stdio.h>

#include "ntwt_interpreter.h"

#define STATE(x)      s_##x :
#define GOTOSTATE(x)  goto s_##x
#define NEXTSTATE()					\
	do {						\
		++exec_ptr;				\
		goto *dtable[(uint8_t) *exec_ptr];	\
	} while(0)

static void *threaded_practise_run(void *p)
{
	ntwt_practise_run(p);
	return NULL;
}

void ntwt_interprete(const char code[], char stack[],
		     struct ntwt_practise prac[])
{
	struct ntwt_practise *context;

	const char *restrict exec_ptr = code;
	/* char *restrict stack_ptr = stack; */

	static const void *restrict const dtable[] = {
		[READ]     = &&s_read,
		[END]      = &&s_end,
		[CONTEXT]  = &&s_context,
		[RUN]      = &&s_run,
		[TEST]     = &&s_test,
		[STRONGER] = &&s_stronger
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
		pthread_create(&((struct ntwt_practise *) context)->thread,
			       NULL, threaded_practise_run,
			       (void *) context);
		NEXTSTATE();
	}
	STATE(test) {
		printf("this is a test\n");
		NEXTSTATE();
	}
	STATE(stronger) {
	        ntwt_practise_stronger(context, 0.1);
		NEXTSTATE();
	}
}

#undef STATE
#undef GOTOSTATE
#undef NEXTSTATE
