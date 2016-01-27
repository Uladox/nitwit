#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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

static void *threaded_awareness_run(void *a)
{
	while (1) {
		printf("AWARE!\n");
		usleep(4000 * 100);
	}
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
		[TEST]     = &&s_test,
		[AWAKE]    = &&s_awake,
		[RUN]      = &&s_run,
		[STRONGER] = &&s_stronger
	};

	goto *dtable[(uint8_t) *exec_ptr];

	STATE (read) {
		NEXTSTATE();
	}
	STATE (end) {
		return;
	}
	STATE (context) {
		++code;
		context = prac + *code;
		NEXTSTATE();
	}
	STATE (test) {
		printf("this is a test\n");
		NEXTSTATE();
	}
	STATE (awake) {
		pthread_create(malloc(sizeof(pthread_t)), NULL,
			       threaded_awareness_run, NULL);
		NEXTSTATE();
	}
	STATE (run) {
		pthread_create(&((struct ntwt_practise *) context)->thread,
			       NULL, threaded_practise_run,
			       (void *) context);
		NEXTSTATE();
	}
	STATE (stronger) {
	        ntwt_practise_stronger(context, 0.1);
		NEXTSTATE();
	}
}

#undef STATE
#undef GOTOSTATE
#undef NEXTSTATE
