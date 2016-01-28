#ifndef NTWT_INTERPRETER
#define NTWT_INTERPRETER

#include <stdint.h>

#include "../practise/ntwt_practise.h"

enum { NTWT_OP_READ, NTWT_OP_END, NTWT_OP_CONTEXT, NTWT_OP_TEST,
       /* Awareness operations */
       NTWT_OP_AWAKE,
       /* Practise  operations */
       NTWT_OP_ACTION,
       NTWT_OP_STRENGTH, NTWT_OP_CAN_HAPPEN, NTWT_OP_UNSATISFIED,
       NTWT_OP_RUN, NTWT_OP_STRONGER,
       /* Writing operations */
       NTWT_OP_SAVE };

struct ntwt_instance {
	struct ntwt_practise *context;
	struct ntwt_practise *practises;
	pthread_t awareness;
};

void ntwt_interprete(struct ntwt_instance *state, const char code[]);

#endif
