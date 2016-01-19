#ifndef NTWT_INTERPRETER
#define NTWT_INTERPRETER

#include <stdint.h>

#include "../practise/ntwt_practise.h"

enum { READ, END, CONTEXT, RUN, TEST };

enum { PRAC_1 = 0, PRAC_2, PRAC_3};

void ntwt_interprete(const uint_fast8_t code[], uint_fast8_t stack[],
		     struct ntwt_practise prac[]);

#endif
