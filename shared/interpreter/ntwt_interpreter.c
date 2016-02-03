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

#define POPSTRING(VARIABLE, POINTER)					\
	do {								\
	        VARIABLE ## _size = strlen(POINTER) + 1;		\
		VARIABLE = malloc(VARIABLE ## _size);			\
		strcpy(VARIABLE, POINTER);				\
	        POINTER += VARIABLE ## _size;				\
	} while (0)

/* Included functions stored in other files for keeping this file clean */
#include "subfuncts/saving.c"
#include "subfuncts/threaded.c"

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
		char *location;
		size_t location_size;

		++exec_ptr;

		POP(package_num, exec_ptr, unsigned int);
		POP(action_max, exec_ptr, unsigned int);
		POPSTRING(location, exec_ptr);

	        ntwt_instance_load_package(state,
					   package_num,
					   action_max,
					   location);

		POINTEDSTATE();
	}

	/* package_num, id, action_name */
	STATE (load_action) {
		unsigned int package_num, id;
		char *action_name;
		size_t action_name_size;

		++exec_ptr;

		POP(package_num, exec_ptr, unsigned int);
		POP(id, exec_ptr, unsigned int);
		POPSTRING(action_name, exec_ptr);

	        ntwt_package_load_action(state->packages + package_num,
					 id, action_name);
		POINTEDSTATE();
	}
}

#undef STATE
#undef GOTOSTATE
#undef NEXTSTATE
#undef POINTEDSTATE
