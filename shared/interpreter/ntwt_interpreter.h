#ifndef NTWT_INTERPRETER
#define NTWT_INTERPRETER

#include <stdint.h>

#include "../practise/ntwt_practise.h"

enum { READ, END, CONTEXT, RUN, TEST, STRONGER };

enum { PRAC_1 = 0, PRAC_2, PRAC_3};

void ntwt_interprete(const char code[], char stack[],
		     struct ntwt_practise prac[]);

#endif
