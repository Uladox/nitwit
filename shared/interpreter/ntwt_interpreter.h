#ifndef NTWT_INTERPRETER
#define NTWT_INTERPRETER

#include <stdint.h>

#include "../practise/ntwt_practise.h"

enum { READ, END, CONTEXT, TEST,
       /* Awareness operations */
       AWAKE,
       /* Practise  operations */
       RUN, STRONGER,
       /* Writing operations */
       SAVE };

struct ntwt_instance {
	struct ntwt_practise *context;
	struct ntwt_practise *practises;
	pthread_t awareness;
};

void ntwt_interprete(struct ntwt_instance *state, const char code[]);

#endif
