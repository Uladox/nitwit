#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uniconv.h>
#include <unistd.h>
#include <unistr.h>

#define NIT_SHORT_NAMES
#include <nitlib/io.h>

#define NTWT_SHORT_NAMES
#include "plugin.h"
#include "vm.h"
#include "state.h"

#define MOVEBY(POINTER, TYPE, AMOUNT)				\
	(POINTER = ((char *) (((TYPE *) POINTER) + (AMOUNT))))
#define COPY(VARIABLE, POINTER)				\
	(VARIABLE = *((typeof(VARIABLE) *) POINTER))
#define POPSET(VARIABLE, POINTER)			\
	do {						\
		COPY(VARIABLE, POINTER);		\
		MOVEBY(POINTER, typeof(VARIABLE), 1);	\
	} while (0)

#define POPSETSTRING(VARIABLE, SIZE, POINTER)				\
	do {								\
		SIZE = u8_strlen((uint8_t *) POINTER);			\
		VARIABLE = malloc(SIZE + 1);				\
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

void
ntwt_interprete(struct ntwt_vm_state *restrict state,
		const char *restrict exec_ptr)
{

/* If you look at this ifdef as a big block, it helps. */
#ifdef C99_COMPLIANT
	/* Uses while and switch for reading bytes */
#define next_step() do { ++exec_ptr; goto begin; } while (0)
#define STEP(x) case NTWT_OP_ ## x :
#define BEGIN()					\
	begin:					\
		switch (*exec_ptr) {
#define CONCLUDE() }
#else
	/* Uses non-standard gcc computed gotos */
	static const void *restrict const dtable[] = {
		[NTWT_OP_READ]        = &&READ,
		[NTWT_OP_END]         = &&END,
		[NTWT_OP_TEST]        = &&TEST,
		[NTWT_OP_ECHO]        = &&ECHO,
		[NTWT_OP_EXEC]        = &&EXEC,
		[NTWT_OP_SAVE]        = &&SAVE
	};

#define next_step() goto *dtable[(uint8_t) *++exec_ptr]
#define STEP(x) x :
#define BEGIN() goto *dtable[(uint8_t) *exec_ptr]
#define CONCLUDE()
#endif

	BEGIN();
	STEP (READ) {
		printf("This should never be read!\n");
		next_step();
	}

	STEP (END) {
		return;
	}

	STEP (TEST) {
		printf("this is a test\n");
		next_step();
	}

#ifdef ASSUME_UTF8
	STEP (ECHO) {
		++exec_ptr;
		printf("%s\n", exec_ptr);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr);
		next_step();
	}
#else
	STEP (ECHO) {
		char *io;

		++exec_ptr;
		printf("%s\n", io = u8_strconv_to_locale((uint8_t *) exec_ptr));
		free(io);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr);
		next_step();
	}
#warning "Less efficient if your locale is utf8."
#endif

	STEP (EXEC) {
		uint8_t *str;
		size_t size;

		++exec_ptr;
		POPSETSTRING(str, size, exec_ptr);
		ntwt_plugin_start(state->pass, (char *) str);
		next_step();
	}

	STEP (SAVE) {
		ntwt_vm_save(state, state->output);
		next_step();
	}

	CONCLUDE();
}
