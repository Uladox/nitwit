#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistr.h>
#include <uniconv.h>

#include "vm.h"

#include "../unicode/unihelpers.h"

#define STEP(x) x :
#define MOVEBY(POINTER, TYPE, AMOUNT)				\
	(POINTER = ((char *) (((TYPE *) POINTER) + (AMOUNT))))
#define COPY(VARIABLE, POINTER)		\
	(VARIABLE = *((typeof(VARIABLE) *) POINTER))
#define POPSET(VARIABLE, POINTER)			\
	do {						\
		COPY(VARIABLE, POINTER);		\
		MOVEBY(POINTER, typeof(VARIABLE), 1);	\
	} while (0)

#define POPSETSTRING(VARIABLE, SIZE, POINTER)				\
	do {								\
		SIZE = u8_strlen((uint8_t *) POINTER) + 1;		\
		VARIABLE = malloc(SIZE);				\
		u8_strcpy(VARIABLE, (uint8_t *) POINTER);		\
		POINTER += SIZE;					\
	} while (0)
#define POP(TYPE, POINTER)						\
	({								\
		TYPE temp;						\
		COPY(temp, POINTER);					\
		MOVEBY(POINTER, TYPE, 1);				\
		temp;							\
	})

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


void ntwt_interprete(struct ntwt_vm_state *restrict state,
		     const char *restrict exec_ptr,
		     const char *out_name)
{
	static const void *restrict const dtable[] = {
		[NTWT_OP_READ]        = &&read,
		[NTWT_OP_END]         = &&end,
		[NTWT_OP_CONTEXT]     = &&context,
		[NTWT_OP_TEST]        = &&test,
		[NTWT_OP_ECHO]        = &&echo,
		[NTWT_OP_AWAKE]       = &&awake,
		[NTWT_OP_INIT_PRAC]   = &&init_prac,
		[NTWT_OP_LOAD_PRAC]   = &&load_prac,
		[NTWT_OP_ACTION]      = &&action,
		[NTWT_OP_CAN_HAPPEN]  = &&can_happen,
		[NTWT_OP_STRENGTH]    = &&strength,
		[NTWT_OP_UNSATISFIED] = &&unsatisfied,
		[NTWT_OP_RUN]         = &&run,
		[NTWT_OP_STRONGER]    = &&stronger,
		[NTWT_OP_SAVE]        = &&save,
		[NTWT_OP_INIT_PACK]   = &&init_pack,
		[NTWT_OP_LOAD_PACK]   = &&load_pack,
		[NTWT_OP_LOAD_ACTION] = &&load_action
	};

	inline void next_step(void)
	{
		goto *dtable[(uint8_t) *++exec_ptr];
	}

	inline void pointed_step(void)
	{
		goto *dtable[(uint8_t) *exec_ptr];
	}

	goto *dtable[(uint8_t) *exec_ptr];

	STEP (read) {
		printf("This should never be read!\n");
		goto *dtable[(uint8_t) *++exec_ptr];
		next_step();
	}

	STEP (end) {
		return;
	}

	STEP (context) {
		++exec_ptr;
		state->context = state->practises + *exec_ptr;
		next_step();
	}

	STEP (test) {
		printf("this is a test\n");
		next_step();
	}

#if ASSUME_UTF8
	STEP (echo) {
		++exec_ptr;
		printf("%s\n", exec_ptr);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr) + 1;
		pointed_step();
	}
#else
	STEP (echo) {
		char *io;

		++exec_ptr;
		printf("%s\n", io = u8_strconv_to_locale((uint8_t *) exec_ptr));
		free(io);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr) + 1;
		pointed_step();
	}
#warning "Less efficient if your locale is utf8."
#endif

	STEP (awake) {
		pthread_create(&state->awareness, NULL,
			       threaded_awareness_run, NULL);
		next_step();
	}

	STEP (init_prac) {
		++exec_ptr;
		POPSET(state->practise_max, exec_ptr);

		state->practises = calloc(state->practise_max,
					 sizeof(*state->practises));
		pointed_step();
	}

	STEP (load_prac) {
		uint32_t prac_id;
		uint32_t pkg_pos;
		uint32_t action_id;
		double can_happen;
		double strength;
		double unsatisfied;
		struct ntwt_practise *prac;
		struct ntwt_action *action;

		++exec_ptr;
		POPSET(prac_id, exec_ptr);
		POPSET(pkg_pos, exec_ptr);
		POPSET(action_id, exec_ptr);
		POPSET(can_happen, exec_ptr);
		POPSET(strength, exec_ptr);
		POPSET(unsatisfied, exec_ptr);
		prac = state->practises + prac_id;
		action = (state->packages + pkg_pos)->actions + action_id;
		ntwt_practise_load(prac, action, can_happen,
				   strength, unsatisfied);
		pointed_step();
	}

	STEP (action) {
		uint32_t pkg_pos;
		uint32_t id;

		++exec_ptr;
		POPSET(pkg_pos, exec_ptr);
		POPSET(id, exec_ptr);
		state->context->action =
			(state->packages + pkg_pos)->actions + id;
		pointed_step();
	}


	STEP (can_happen) {
		++exec_ptr;
		ntwt_practise_can_happen(state->context, POP(double, exec_ptr));
		pointed_step();
	}

	STEP (strength) {
		++exec_ptr;
		ntwt_practise_strength(state->context, POP(double, exec_ptr));
		pointed_step();
	}

	STEP (unsatisfied) {
		++exec_ptr;
		ntwt_practise_unsatisfied(state->context,
					  POP(double, exec_ptr));
		pointed_step();
	}

	STEP (run) {
		pthread_create(&state->context->thread,
			       NULL, threaded_practise_run,
			       (void *) state->context);
		next_step();
	}

	STEP (stronger) {
		ntwt_practise_stronger(state->context, 0.1);
		next_step();
	}

	STEP (save) {
		ntwt_vm_save(state, out_name);
		next_step();
	}

	STEP (init_pack) {
		++exec_ptr;
		POPSET(state->package_max, exec_ptr);
		++state->package_max;
		state->packages = calloc
			(state->package_max,
			 sizeof(*state->packages));
		*state->packages = ntwt_std_package;
		state->package_ptr = 1;
		pointed_step();
	}

	STEP (load_pack) {
		uint32_t pkg_num;
		uint32_t action_max;
		uint8_t *path;
		size_t path_size;

		++exec_ptr;
		POPSET(pkg_num, exec_ptr);
		POPSET(action_max, exec_ptr);
		POPSETSTRING(path, path_size, exec_ptr);
		ntwt_vm_state_load_package(state, pkg_num, action_max, path);
		pointed_step();
	}

	STEP (load_action) {
		uint32_t pkg_num;
		uint32_t id;
		uint8_t *name;
		size_t name_size;

		++exec_ptr;
		POPSET(pkg_num, exec_ptr);
		POPSET(id, exec_ptr);
		POPSETSTRING(name, name_size, exec_ptr);
		ntwt_package_load_action(state->packages + pkg_num, id, name);
		pointed_step();
	}
}
