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
#define MOVEBY(POINTER, TYPE, AMOUNT)				\
	POINTER = ((char *) (((TYPE *) POINTER) + (AMOUNT)))
#define COPY(VARIABLE, POINTER, TYPE)		\
	VARIABLE = *((TYPE *) POINTER)
#define POP(VARIABLE, POINTER, TYPE)		\
	do {					\
		COPY(VARIABLE, POINTER, TYPE);	\
		MOVEBY(POINTER, TYPE, 1);	\
	} while (0)
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

static void save_packages(struct ntwt_instance *state, FILE *image)
{
	unsigned int i;
	unsigned int j;
	struct ntwt_package *pack;
	struct ntwt_action *action;
	char c_op[1] = { NTWT_OP_INIT_PACK };

	/* Writes packages and their actions */
	fwrite(c_op, sizeof(char), 1, image);
	fwrite(&state->package_max,
	       sizeof(unsigned int), 1, image);
	for (i = 1; i != state->package_max; ++i) {
		pack = state->packages + i;
		if (!pack->loaded)
			continue;
		unsigned int pack_args[2] = {
			[0] = pack->package_num,
			[1] = pack->action_max
		};
		c_op[0] = NTWT_OP_LOAD_PACK;
		fwrite(c_op, sizeof(char), 1, image);
		fwrite(pack_args, sizeof(unsigned int), 2, image);
		fwrite(pack->location, sizeof(char),
		       strlen(pack->location) + 1, image);

		for (j = 0; j != pack->action_max; ++j) {
			action = pack->actions + j;
			if (!action->loaded)
				continue;
			unsigned int action_args[2] = {
				[0] = action->package_num,
				[1] = action->id
			};
			c_op[0] = NTWT_OP_LOAD_ACTION;
			fwrite(c_op, sizeof(char), 1, image);
			fwrite(action_args, sizeof(unsigned int),
			       2, image);
			fwrite(action->name, sizeof(char),
			       strlen(action->name) + 1, image);
		}
	}
}

static void save_practises(struct ntwt_instance *state, FILE *image)
{
	/* Writes practises and gives them actions */
	struct ntwt_practise *prac;
	unsigned int i;
	char c_op[1] = { NTWT_OP_LOAD_PRAC };

	for (i = 0; i != state->practise_max; ++i) {
		printf("hello, %u\n", i);
		prac = state->practises + i;
		if (!prac->loaded)
			continue;
		printf("loaded");
		fwrite(c_op, sizeof(char), 1, image);
		unsigned int int_args[3] = {
			[0] = i,
			[1] = prac->action->package_num,
			[2] = prac->action->id
		};
		double double_args[3] = {
			[0] = prac->strength,
			[1] = prac->can_happen,
			[2] = prac->unsatisfied
		};
		fwrite(int_args, sizeof(unsigned int),
		       3, image);
		fwrite(double_args, sizeof(double), 3, image);
	}
}

static void save(struct ntwt_instance *state)
{
	remove("state.ilk");
	FILE *image = fopen("state.ilk", "ab");
	char c_op[1];

	c_op[0] = NTWT_OP_TEST;
	fwrite(c_op, 1, 1, image);
	save_packages(state, image);
	save_practises(state, image);

	/* c_op[0] = NTWT_OP_TEST; */
	/* fwrite(c_op, 1, 1, image); */
	c_op[0] = NTWT_OP_CONTEXT;
	fwrite(&c_op[0], 1, 1, image);
	c_op[0] = 0;
	fwrite(c_op, 1, 1, image);
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
	c_op[0] = NTWT_OP_RUN;
	fwrite(c_op, 1, 1, image);
	c_op[0] = NTWT_OP_END;
	fwrite(c_op, 1, 1, image);
	fclose(image);
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
		[NTWT_OP_LOAD_PRAC]   = &&s_load_prac,
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
		POP(state->practise_max, exec_ptr, unsigned int);
		state->practises = calloc(state->practise_max,
					  sizeof(struct ntwt_practise));
		POINTEDSTATE();
	}
	/* practise_id, action_package_location, action_id,
	   strength, can_happen, unsatisfied */
	STATE (load_prac) {
		unsigned int practise_id;
		unsigned int action_package_location, action_id;
		double strength, can_happen, unsatisfied;
		struct ntwt_practise *prac;
		struct ntwt_action *action;

		++exec_ptr;

		POP(practise_id, exec_ptr, unsigned int);
		POP(action_package_location, exec_ptr, unsigned int);
		POP(action_id, exec_ptr, unsigned int);
		POP(strength, exec_ptr, double);
		POP(can_happen, exec_ptr, double);
		POP(unsatisfied, exec_ptr, double);

		prac = state->practises + practise_id;
		action = (state->packages + action_package_location)
			->actions + action_id;
		ntwt_practise_load(prac, action, can_happen, strength,
				   unsatisfied);
		POINTEDSTATE();
	}
	STATE (action) {
		unsigned int action_package_location;
		unsigned int action_id;

		++exec_ptr;
		POP(action_package_location, exec_ptr, unsigned int);
		POP(action_id, exec_ptr, unsigned int);

		state->context->action =
			(state->packages + action_package_location)
			->actions + action_id;
		POINTEDSTATE();
	}
	STATE (strength) {
		++exec_ptr;
		ntwt_practise_strength(state->context, *((double *) exec_ptr));
		MOVEBY(exec_ptr, double, 1);
		POINTEDSTATE();
	}
	STATE (can_happen) {
		++exec_ptr;
		ntwt_practise_can_happen(state->context, *((double *) exec_ptr));
		MOVEBY(exec_ptr, double, 1);
		POINTEDSTATE();
	}
	STATE (unsatisfied) {
		++exec_ptr;
		ntwt_practise_unsatisfied(state->context, *((double *) exec_ptr));
		MOVEBY(exec_ptr, double, 1);
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
		save(state);
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
		MOVEBY(exec_ptr, unsigned int, 1);
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
		POP(package_num, exec_ptr, unsigned int);
		POP(action_max, exec_ptr, unsigned int);

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

		POP(package_num, exec_ptr, unsigned int);
		POP(id, exec_ptr, unsigned int);

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
