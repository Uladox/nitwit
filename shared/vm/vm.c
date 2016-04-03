#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistr.h>
#include <uniconv.h>

#include "vm.h"

#include "../unicode/unihelpers.h"

#define STATE(x)      s_##x :
#define GOTOSTATE(x)  goto s_##x
#define NEXTSTATE(POINTER)			\
	goto *dtable[(uint8_t) *++(POINTER)];
#define POINTEDSTATE(POINTER)			\
	goto *dtable[(uint8_t) *(POINTER)]
#define MOVEBY(POINTER, TYPE, AMOUNT)				\
	POINTER = ((char *) (((TYPE *) POINTER) + (AMOUNT)))
#define COPY(VARIABLE, POINTER)		\
	VARIABLE = *((typeof(VARIABLE) *) POINTER)
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


const unsigned int *restrict const ntwt_op_args[] = {
	[NTWT_OP_READ]        = (unsigned int []) { 0 },
	[NTWT_OP_END]         = (unsigned int []) { 0 },
	[NTWT_OP_CONTEXT]     = (unsigned int []) { 1, NTWT_CHAR },
	[NTWT_OP_TEST]        = (unsigned int []) { 0 },
	[NTWT_OP_ECHO]        = (unsigned int []) { 1, NTWT_STRING },
	[NTWT_OP_AWAKE]       = (unsigned int []) { 0 },
	[NTWT_OP_INIT_PRAC]   = (unsigned int []) { 1, NTWT_UINT },
	[NTWT_OP_LOAD_PRAC]   = (unsigned int []) { 6,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_DOUBLE,
						    NTWT_DOUBLE,
						    NTWT_DOUBLE },
	[NTWT_OP_ACTION]      = (unsigned int []) { 2,
						    NTWT_UINT,
						    NTWT_UINT },
	[NTWT_OP_CAN_HAPPEN]  = (unsigned int []) { 1,
						    NTWT_DOUBLE },
	[NTWT_OP_STRENGTH]    = (unsigned int []) { 1,
						    NTWT_DOUBLE },
	[NTWT_OP_UNSATISFIED] = (unsigned int []) { 1,
						    NTWT_DOUBLE },
	[NTWT_OP_RUN]         = (unsigned int []) { 0 },
	[NTWT_OP_STRONGER]    = (unsigned int []) { 0 },
	[NTWT_OP_SAVE]        = (unsigned int []) { 0 },
	[NTWT_OP_INIT_PACK]   = (unsigned int []) { 1,
						    NTWT_UINT },
	[NTWT_OP_LOAD_PACK]   = (unsigned int []) { 3,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_STRING },
	[NTWT_OP_LOAD_ACTION] = (unsigned int []) { 3,
						    NTWT_UINT,
						    NTWT_UINT,
						    NTWT_STRING }
};



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
		[NTWT_OP_READ]        = &&s_read,
		[NTWT_OP_END]         = &&s_end,
		[NTWT_OP_CONTEXT]     = &&s_context,
		[NTWT_OP_TEST]        = &&s_test,
		[NTWT_OP_ECHO]        = &&s_echo,
		[NTWT_OP_AWAKE]       = &&s_awake,
		[NTWT_OP_INIT_PRAC]   = &&s_init_prac,
		[NTWT_OP_LOAD_PRAC]   = &&s_load_prac,
		[NTWT_OP_ACTION]      = &&s_action,
		[NTWT_OP_CAN_HAPPEN]  = &&s_can_happen,
		[NTWT_OP_STRENGTH]    = &&s_strength,
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
		printf("This should never be read!\n");
		NEXTSTATE(exec_ptr);
	}

	STATE (end) {
		return;
	}

	STATE (context) {
		++exec_ptr;
		state->context = state->practises + *exec_ptr;
		NEXTSTATE(exec_ptr);
	}

	STATE (test) {
		printf("this is a test\n");
		NEXTSTATE(exec_ptr);
	}

#if ASSUME_UTF8
	STATE(echo) {
		++exec_ptr;
		printf("%s\n", exec_ptr);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr) + 1;
		POINTEDSTATE(exec_ptr);
	}
#else
	STATE (echo) {
		char *io;

		++exec_ptr;
		printf("%s\n", io = u8_strconv_to_locale((uint8_t *) exec_ptr));
		free(io);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr) + 1;
		POINTEDSTATE(exec_ptr);
	}
#warning "Less efficient if your locale is utf8."
#endif

	STATE (awake) {
		pthread_create(&state->awareness, NULL,
			       threaded_awareness_run, NULL);
		NEXTSTATE(exec_ptr);
	}

	STATE (init_prac) {
		++exec_ptr;
		POPSET(state->practise_max, exec_ptr);

		state->practises = calloc(state->practise_max,
					 sizeof(*state->practises));
		POINTEDSTATE(exec_ptr);
	}

	STATE (load_prac) {
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
		POINTEDSTATE(exec_ptr);
	}

	STATE (action) {
		uint32_t pkg_pos;
		uint32_t id;

		++exec_ptr;
		POPSET(pkg_pos, exec_ptr);
		POPSET(id, exec_ptr);
		state->context->action =
			(state->packages + pkg_pos)->actions + id;
		POINTEDSTATE(exec_ptr);
	}


	STATE (can_happen) {
		++exec_ptr;
		ntwt_practise_can_happen(state->context, POP(double, exec_ptr));
		POINTEDSTATE(exec_ptr);
	}

	STATE (strength) {
		++exec_ptr;
		ntwt_practise_strength(state->context, POP(double, exec_ptr));
		POINTEDSTATE(exec_ptr);
	}

	STATE (unsatisfied) {
		++exec_ptr;
		ntwt_practise_unsatisfied(state->context, POP(double, exec_ptr));
		POINTEDSTATE(exec_ptr);
	}

	STATE (run) {
		pthread_create(&state->context->thread,
			       NULL, threaded_practise_run,
			       (void *) state->context);
		NEXTSTATE(exec_ptr);
	}

	STATE (stronger) {
		ntwt_practise_stronger(state->context, 0.1);
		NEXTSTATE(exec_ptr);
	}

	STATE (save) {
		ntwt_vm_save(state, out_name);
		NEXTSTATE(exec_ptr);
	}

	STATE (init_pack) {
		++exec_ptr;
		POPSET(state->package_max, exec_ptr);
		++state->package_max;
		state->packages = calloc
			(state->package_max,
			 sizeof(*state->packages));
		*state->packages = ntwt_std_package;
		state->package_ptr = 1;
		POINTEDSTATE(exec_ptr);
	}

	STATE (load_pack) {
		uint32_t pkg_num;
		uint32_t action_max;
		uint8_t *path;
		size_t path_size;

		++exec_ptr;
		POPSET(pkg_num, exec_ptr);
		POPSET(action_max, exec_ptr);
		POPSETSTRING(path, path_size, exec_ptr);
		ntwt_vm_state_load_package(state, pkg_num, action_max, path);
		POINTEDSTATE(exec_ptr);
	}

	STATE (load_action) {
		uint32_t pkg_num;
		uint32_t id;
		uint8_t *name;
		size_t name_size;

		++exec_ptr;
		POPSET(pkg_num, exec_ptr);
		POPSET(id, exec_ptr);
		POPSETSTRING(name, name_size, exec_ptr);
		ntwt_package_load_action(state->packages + pkg_num, id, name);
		POINTEDSTATE(exec_ptr);
	}
}
