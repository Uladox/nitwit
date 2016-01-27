#ifndef NTWT_INTERPRETER
#define NTWT_INTERPRETER

#include <stdint.h>

#include "../practise/ntwt_practise.h"

enum { READ, END, CONTEXT, TEST,
       /* Awareness operations */
       AWAKE,
       /* Practise  operations */
       RUN, STRONGER };

void ntwt_interprete(const char code[], char stack[],
		     struct ntwt_practise prac[]);

#endif
