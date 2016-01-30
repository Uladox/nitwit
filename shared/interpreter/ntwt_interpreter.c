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

void ntwt_interprete(struct ntwt_instance *state, const char code[])
{

	const char *restrict exec_ptr = code;

	/* char *restrict stack_ptr = stack; */

	static const void *restrict const dtable[] = {
		[NTWT_OP_READ]        = &&s_read,
		[NTWT_OP_END]         = &&s_end,
		[NTWT_OP_CONTEXT]     = &&s_context,
		[NTWT_OP_TEST]        = &&s_test,
		[NTWT_OP_AWAKE]       = &&s_awake,
		[NTWT_OP_ACTION]      = &&s_action,
		[NTWT_OP_STRENGTH]    = &&s_strength,
		[NTWT_OP_CAN_HAPPEN]  = &&s_can_happen,
		[NTWT_OP_UNSATISFIED] = &&s_unsatisfied,
		[NTWT_OP_RUN]         = &&s_run,
		[NTWT_OP_STRONGER]    = &&s_stronger,
		[NTWT_OP_SAVE]        = &&s_save,
		[NTWT_OP_INIT_PACK]   = &&s_init_pack
	};

	goto *dtable[(uint8_t) *exec_ptr];

	STATE (read) {
		NEXTSTATE();
	}
	STATE (end) {
		return;
	}
	STATE (context) {
		++exec_ptr;
		state->context = state->practises + *exec_ptr;
		NEXTSTATE();
	}
	STATE (test) {
		printf("this is a test\n");
		NEXTSTATE();
	}
	STATE (awake) {
		pthread_create(&state->awareness, NULL,
			       threaded_awareness_run, NULL);
		NEXTSTATE();
	}
	STATE (action) {
		unsigned int action_package_location;
		unsigned int action_id;

		++exec_ptr;
		action_package_location =
			*((unsigned int *) exec_ptr);
		action_id = *(((unsigned int *) exec_ptr) + 1);
		state->context->action =
			(*(state->packages + action_package_location))
			->actions + action_id;
		exec_ptr = ((char *) (((unsigned int *) exec_ptr) + 2)) - 1;
		NEXTSTATE();
	}
	STATE (strength) {
		++exec_ptr;
		ntwt_practise_strength(state->context, *((double *) exec_ptr));
		exec_ptr = ((char *) (((double *) exec_ptr) + 1)) - 1;
		NEXTSTATE();
	}
	STATE (can_happen) {
		++exec_ptr;
		ntwt_practise_can_happen(state->context, *((double *) exec_ptr));
		exec_ptr = ((char *) (((double *) exec_ptr) + 1)) - 1;
		NEXTSTATE();
	}
	STATE (unsatisfied) {
		++exec_ptr;
		ntwt_practise_unsatisfied(state->context, *((double *) exec_ptr));
		exec_ptr = ((char *) (((double *) exec_ptr) + 1)) - 1;
		NEXTSTATE();
	}
	STATE (run) {
		pthread_create(&state->context->thread,
			       NULL, threaded_practise_run,
			       (void *) state->context);
		NEXTSTATE();
	}
	STATE (stronger) {
	        ntwt_practise_stronger(state->context, 0.1);
		NEXTSTATE();
	}
	STATE (save) {
		remove("state.ilk");
		FILE *image = fopen("state.ilk", "ab");
		/* char test_save[] = { TEST, CONTEXT, 0, RUN, END}; */
		/* fwrite(&test_save, sizeof(char), sizeof(test_save), image); */
		char s_op[1];
		double d_op[1];

		s_op[0] = NTWT_OP_TEST;
		fwrite(s_op, 1, 1, image);

		s_op[0] = NTWT_OP_TEST;
		fwrite(s_op, 1, 1, image);

		s_op[0] = NTWT_OP_CONTEXT;
		fwrite(&s_op[0], 1, 1, image);

		s_op[0] = 0;
		fwrite(s_op, 1, 1, image);

		s_op[0] = NTWT_OP_ACTION;
		fwrite(&s_op[0], 1, 1, image);

		s_op[0] = NTWT_OP_STRENGTH;
		fwrite(s_op, sizeof(char), 1, image);
		d_op[0] = state->context->strength;
		fwrite(d_op, sizeof(double), 1, image);

		s_op[0] = NTWT_OP_CAN_HAPPEN;
		fwrite(s_op, sizeof(char), 1, image);
		d_op[0] = state->context->can_happen;
		fwrite(d_op, sizeof(double), 1, image);

		s_op[0] = NTWT_OP_UNSATISFIED;
		fwrite(s_op, sizeof(char), 1, image);
		d_op[0] = state->context->unsatisfied;
		fwrite(d_op, sizeof(double), 1, image);

		s_op[0] = NTWT_OP_RUN;
		fwrite(s_op, 1, 1, image);

		s_op[0] = NTWT_OP_END;
		fwrite(s_op, 1, 1, image);

		fclose(image);
		NEXTSTATE();
	}
	STATE (init_pack) {
		++exec_ptr;
		state->package_max = *((unsigned int *) exec_ptr);
		state->packages = malloc
			(sizeof(struct ntwt_package *) *
			 state->package_max);
		*state->packages = &ntwt_std_package;
		state->package_ptr = 1;
		exec_ptr = ((char *) (((unsigned int *) exec_ptr) + 1)) - 1;
		printf("wot?!\n");

		/* For testing only */
		state->practises->can_happen = 0.5;
		state->practises->strength = 0.5;
		state->practises->unsatisfied = 0.5;
		NEXTSTATE();
	}
}

#undef STATE
#undef GOTOSTATE
#undef NEXTSTATE
