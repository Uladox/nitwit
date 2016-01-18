#ifndef NTWT_INTERPRETER
#define NTWT_INTERPRETER

#include <stdint.h>

enum { READ, END, PRINT, HI };

void ntwt_interprete(const uint_fast8_t code[], uint_fast8_t stack[]);

#endif
