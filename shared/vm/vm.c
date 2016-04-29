#include <stdio.h>
#include <stdlib.h>
#include <uniconv.h>
#include <unistd.h>
#include <unistr.h>

#include "../unicode/unihelpers.h"
#include "plugin.h"
#include "vm.h"


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

void ntwt_interprete(struct ntwt_vm_state *restrict state,
		     const char *restrict exec_ptr,
		     const char *out_name)
{
	static const void *restrict const dtable[] = {
		[NTWT_OP_READ]        = &&read,
		[NTWT_OP_END]         = &&end,
		[NTWT_OP_TEST]        = &&test,
		[NTWT_OP_ECHO]        = &&echo,
		[NTWT_OP_EXEC]        = &&exec,
		[NTWT_OP_SAVE]        = &&save
	};

#define next_step() goto *dtable[(uint8_t) *++exec_ptr]

	goto *dtable[(uint8_t) *exec_ptr];

	STEP (read) {
		printf("This should never be read!\n");
		goto *dtable[(uint8_t) *++exec_ptr];
		next_step();
	}

	STEP (end) {
		return;
	}

	STEP (test) {
		printf("this is a test\n");
		next_step();
	}

#if ASSUME_UTF8
	STEP (echo) {
		++exec_ptr;
		printf("%s\n", exec_ptr);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr);
		next_step();
	}
#else
	STEP (echo) {
		char *io;

		++exec_ptr;
		printf("%s\n", io = u8_strconv_to_locale((uint8_t *) exec_ptr));
		free(io);
		exec_ptr += u8_strlen((uint8_t *) exec_ptr);
		next_step();
	}
#warning "Less efficient if your locale is utf8."
#endif

	STEP (exec) {
		uint8_t *str;
		size_t size;

		++exec_ptr;
		POPSETSTRING(str, size, exec_ptr);
		start_plugin((char *) str);
		next_step();
	}

	STEP (save) {
		ntwt_vm_save(state, out_name);
		next_step();
	}

}
