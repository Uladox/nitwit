#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "ntwt_interpreter.h"

#define STATE(x)      s_##x :
#define GOTOSTATE(x)  goto s_##x
#define NEXTSTATE()					\
	do {						\
		++exec_ptr;				\
		goto *dtable[(uint8_t) *exec_ptr];	\
	} while(0)
#define POINTEDSTATE()				\
	goto *dtable[(uint8_t) *exec_ptr]
#define MOVEBY(TYPE, AMOUNT)				\
	exec_ptr = ((char *) (((TYPE *) exec_ptr) + (AMOUNT)))

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
		[NTWT_OP_INIT_PRAC]   = &&s_init_prac,
		[NTWT_OP_ACTION]      = &&s_action,
		[NTWT_OP_STRENGTH]    = &&s_strength,
		[NTWT_OP_CAN_HAPPEN]  = &&s_can_happen,
		[NTWT_OP_UNSATISFIED] = &&s_unsatisfied,
		[NTWT_OP_RUN]         = &&s_run,
		[NTWT_OP_STRONGER]    = &&s_stronger,
		[NTWT_OP_SAVE]        = &&s_save,
		[NTWT_OP_INIT_PACK]   = &&s_init_pack,
		[NTWT_OP_LOAD_PACK]   = &&s_load_pack,
		[NTWT_OP_LOAD_ACTION] = &&s_load_action
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
	STATE (init_prac) {
		++exec_ptr;
		state->practise_max = *((unsigned int *) exec_ptr);
		MOVEBY(unsigned int, 1);
		state->practises = calloc(state->practise_max,
					  sizeof(struct ntwt_practise));
		POINTEDSTATE();
	}
	STATE (action) {
		unsigned int action_package_location;
		unsigned int action_id;

		++exec_ptr;
		action_package_location =
			*((unsigned int *) exec_ptr);
		MOVEBY(unsigned int, 1);
		action_id = *((unsigned int *) exec_ptr);
		MOVEBY(unsigned int, 1);
		state->context->action =
			(state->packages + action_package_location)
			->actions + action_id;
		POINTEDSTATE();
	}
	STATE (strength) {
		++exec_ptr;
		ntwt_practise_strength(state->context, *((double *) exec_ptr));
		MOVEBY(double, 1);
		POINTEDSTATE();
	}
	STATE (can_happen) {
		++exec_ptr;
		ntwt_practise_can_happen(state->context, *((double *) exec_ptr));
		MOVEBY(double, 1);
		POINTEDSTATE();
	}
	STATE (unsatisfied) {
		++exec_ptr;
		ntwt_practise_unsatisfied(state->context, *((double *) exec_ptr));
		MOVEBY(double, 1);
		POINTEDSTATE();
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
		/* static const char null_terminator[1] = { '\0' }; */
		char c_op[1];
		/* double d_op[1]; */

		c_op[0] = NTWT_OP_TEST;
		fwrite(c_op, 1, 1, image);

		/* Writes packages and their actions */
		c_op[0] = NTWT_OP_INIT_PACK;
		fwrite(c_op, sizeof(char), 1, image);
		fwrite(&state->package_max,
		       sizeof(unsigned int), 1, image);
		uint_fast32_t i;
		struct ntwt_package *p;
		for (i = 1; i != state->package_max; ++i) {
			p = state->packages + i;
			if (!p->loaded)
				continue;
			unsigned int p_args[2] = {
				[0] = p->package_num,
				[1] = p->action_max
			};
			c_op[0] = NTWT_OP_LOAD_PACK;
			fwrite(c_op, sizeof(char), 1, image);
			fwrite(p_args, sizeof(unsigned int), 2, image);
			fwrite(p->location, sizeof(char),
			       strlen(p->location) + 1, image);
			/* fwrite(null_terminator, sizeof(char), 1, image); */

			uint_fast32_t j;
			struct ntwt_action *a;
			for (j = 0; j != p->action_max; ++j) {
				a = p->actions + j;
				if (!a->loaded)
					continue;
				unsigned int a_args[2] = {
					[0] = a->package_num,
					[1] = a->id
				};
				c_op[0] = NTWT_OP_LOAD_ACTION;
				fwrite(c_op, sizeof(char), 1, image);
				fwrite(a_args, sizeof(unsigned int),
				       2, image);
				fwrite(a->name, sizeof(char),
				       strlen(a->name) + 1, image);
			}
		}

		/* Writes practises and gives them actions */

		/* c_op[0] = NTWT_OP_TEST; */
		/* fwrite(c_op, 1, 1, image); */

		/* c_op[0] = NTWT_OP_CONTEXT; */
		/* fwrite(&c_op[0], 1, 1, image); */

		/* c_op[0] = 0; */
		/* fwrite(c_op, 1, 1, image); */

		/* c_op[0] = NTWT_OP_ACTION; */
		/* fwrite(&c_op[0], 1, 1, image); */

		/* c_op[0] = NTWT_OP_STRENGTH; */
		/* fwrite(c_op, sizeof(char), 1, image); */
		/* d_op[0] = state->context->strength; */
		/* fwrite(d_op, sizeof(double), 1, image); */

		/* c_op[0] = NTWT_OP_CAN_HAPPEN; */
		/* fwrite(c_op, sizeof(char), 1, image); */
		/* d_op[0] = state->context->can_happen; */
		/* fwrite(d_op, sizeof(double), 1, image); */

		/* c_op[0] = NTWT_OP_UNSATISFIED; */
		/* fwrite(c_op, sizeof(char), 1, image); */
		/* d_op[0] = state->context->unsatisfied; */
		/* fwrite(d_op, sizeof(double), 1, image); */

		/* c_op[0] = NTWT_OP_RUN; */
		/* fwrite(c_op, 1, 1, image); */

		c_op[0] = NTWT_OP_END;
		fwrite(c_op, 1, 1, image);

		fclose(image);
		NEXTSTATE();
	}
	/* package_max */
	STATE (init_pack) {
		++exec_ptr;
		state->package_max = *((unsigned int *) exec_ptr) + 1;
		state->packages = calloc
			(state->package_max,
			 sizeof(struct ntwt_package));
		*state->packages = ntwt_std_package;
		state->package_ptr = 1;
		MOVEBY(unsigned int, 1);
		printf("wot?!\n");

		/* For testing only */
		/* state->practises->can_happen = 0.5; */
		/* state->practises->strength = 0.5; */
		/* state->practises->unsatisfied = 0.5; */
		POINTEDSTATE();
	}
	/* package_num, action_max, location */
	STATE (load_pack) {
		unsigned int package_num;
		unsigned int action_max;

		++exec_ptr;
		package_num = *((unsigned int *) exec_ptr);
		MOVEBY(unsigned int, 1);
		action_max = *((unsigned int *) exec_ptr);
		MOVEBY(unsigned int, 1);

	        ntwt_instance_load_package(state,
					   package_num,
					   action_max,
					   exec_ptr);
		exec_ptr += strlen(exec_ptr);
		/* Note: This abuses the fact that strlen does not count
		   '\0'.
		*/
		NEXTSTATE();
	}
	/* package_num, id, action_name */
	STATE (load_action) {
		unsigned int package_num, id;

		++exec_ptr;
		package_num = *((unsigned int *) exec_ptr);
		MOVEBY(unsigned int, 1);
		id = *((unsigned int *) exec_ptr);
		MOVEBY(unsigned int, 1);
	        ntwt_package_load_action(state->packages + package_num,
					 id, exec_ptr);
		exec_ptr += strlen(exec_ptr);
		/* Note: This abuses the fact that strlen does not count
		   '\0'.
		*/
		NEXTSTATE();
	}
}

#undef STATE
#undef GOTOSTATE
#undef NEXTSTATE
#undef POINTEDSTATE
