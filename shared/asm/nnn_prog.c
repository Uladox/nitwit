#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include <nitlib/list.h>

#include <spar/core.h>
#include <spar/text_utils.h>

/* #include "../vm/state.h" */
/* #include "../vm/vm.h" */
#include "nnn_parser.h"
#include "nnn_expr.h"
#include "nnn_prog.h"

void
nnn_prog_empty(struct nnn_prog *prog)
{
	struct nnn_expr *expr = prog->expr;
	struct nnn_expr *tmp;

	delayed_foreach (tmp, expr) {
		if (tmp->type == NTWT_STRING)
			free(tmp->dat.string);
		free(tmp);
	}

	expr = prog->stack;

	delayed_foreach (tmp, expr)
		free(tmp);
}

static void
nnn_prog_recycle(struct nnn_prog *prog)
{
	struct nnn_expr *expr = prog->expr;
	struct nnn_expr *tmp;

	delayed_foreach (tmp, expr) {
		if (tmp->type == NTWT_STRING)
			free(tmp->dat.string);
	        LIST_CONS(tmp, prog->stack);
		prog->stack = tmp;
	}
	prog->expr = NULL;
	prog->size = 0;
}

static inline void
bad_cmd_skip(struct spar_lexinfo *info)
{
	for (;; ++info->dat.text) {
		switch (*info->dat.text) {
		case ';':
		case '\0':
			return;
		}
	}
}

static inline int
is_bad_cmd(struct nnn_expr *expr1, struct nnn_expr *expr2)
{
	return expr1->type == NTWT_SEMI &&
		expr2->type == NTWT_OP_CODE &&
	        expr2->dat.op_code == NTWT_OP_INVALID;
}

void
nnn_prog_get(struct nnn_prog *prog, uint8_t *code, int *error)
{
	struct nnn_expr *expr;
	struct nnn_expr *expr2;
	enum spar_parsed parsed = SPAR_OK;
	struct spar_token token = {
		.type = nnn_type_semi
	};
	struct spar_text_cue text_cue = {
		.lines = 0
	};
	struct spar_lexinfo info = {
		.dat.text = (char *) code,
		.cue.text = &text_cue,
		.error_leave = 0
	};

	nnn_prog_recycle(prog);
	prog->expr = (expr = nnn_expr_get(prog, &info, &token, &parsed));

	while (expr->type != NTWT_EOI) {
	        expr2 = nnn_expr_get(prog, &info, &token, &parsed);
		if (is_bad_cmd(expr, expr2)) {
			bad_cmd_skip(&info);
			LIST_CONS(expr2, prog->stack);
			prog->stack = expr2;
		} else {
			LIST_CONS(expr, expr2);
			expr = expr2;
		}
	}

        LIST_CONS(expr, NULL);

	if (parsed == SPAR_ERROR)
		*error = 1;
}

void
nnn_prog_print(struct nnn_prog *prog)
{
	struct nnn_expr *expr = prog->expr;

	foreach (expr) {
		switch (expr->type) {
		case NTWT_EOI:
			printf("EOI");
			break;
		case NTWT_SEMI:
			printf(";");
			break;
		case NTWT_OP_CODE:
			printf("OP");
			break;
		case NTWT_UINT:
			printf("UINT");
			break;
		case NTWT_INT:
			printf("INT");
			break;
		case NTWT_DOUBLE:
			printf("DOUBLE");
			break;
		case NTWT_STRING:
			printf("STRING");
			break;
		}
		printf(" ");
	}
	printf("\n");
}

void
nnn_prog_bytecode(struct nnn_prog *prog, struct nnn_bcode *bcode, int *error)
{
	bcode->size = prog->size;
	if (prog->size > bcode->max) {
		free(bcode->code);
		bcode->code = malloc(prog->size);
		bcode->max = prog->size;
	}

	char *code = bcode->code;
	struct nnn_expr *expr = prog->expr;

        foreach (expr) {
	        switch (expr->type) {
		case NTWT_EOI:
		case NTWT_SEMI:
			continue;
		case NTWT_STRING:
			memcpy(code, expr->dat.string, expr->size);
			code += expr->size;
			break;
		default:
			memcpy(code, &expr->dat, expr->size);
			code += expr->size;
			break;
		}
	}
}

/* void */
/* nnn_prog_type_check(struct nnn_prog *prog, int *error) */
/* { */
/* 	struct ntwt_asm_expr *term = cmd->contents.list; */
/* 	const unsigned int cmd_line = term->lineno; */
/* 	const char *op_name = ntwt_op_name[(uint8_t) term->contents.op_code]; */
/* 	const int *params = ntwt_op_args[(uint8_t) term->contents.op_code]; */

/* 	term = NIT_LIST_NEXT(term); */

/* 	int i = 0; */

/* 	nit_foreach (term) { */
/* 		++i; */
/* 		if (i > params[0]) { */
/* 			/\* Finds how many terms total *\/ */
/* 			term = NIT_LIST_NEXT(term); */
/* 			nit_foreach (term) */
/* 				++i; */

/* 			fprintf(stderr, */
/* 				"Error: too many arguments to %s on line %u, expected %u, got %i\n", */
/* 			        cmd_name, cmd_line, params[0], i); */
/* 			*error = 1; */
/* 			return; */
/* 		} */

/* 		if (params[i] != term->type) { */
/* 			fprintf(stderr, */
/* 				"Error: on line %u expected type %s, got %s\n", */
/* 				term->lineno, ntwt_type_name[params[i]], */
/* 				ntwt_type_name[term->type]); */
/* 			*error = 1; */
/* 		} */
/* 	} */

/* 	if (i < params[0]) { */
/* 		fprintf(stderr, */
/* 			"Error: too few arguments to %s on line %u, expected %u, got %u\n", */
/* 		        cmd_name, cmd_line, */
/* 			params[0], i); */
/* 		*error = 1; */
/* 	} */
/* } */

/* static inline void */
/* statement_type_check(struct nnn_expr *expr, int *error) */
/* { */
/* 	const char *op_name = ntwt_op_name[(uint8_t) expr->dat.op_code]; */
/* 	const int *params = ntwt_op_args[(uint8_t) expr->dat.op_code]; */

/* } */

/* void */
/* nnn_prog_type_check(struct nnn_prog *prog, int *error) */
/* { */
/* 	struct nnn_expr *expr = prog->expr; */
/* 	int args = -1; */

/* 	foreach (expr) { */
/* 		if (expr->type == NTWT_EOI) */
/* 			return; */
/* 		if (expr->type == NTWT_SEMI) { */
/* 			args = -1; */
/* 			continue; */
/* 		} */
/* 		if (args == -1) { */
/* 			args = 0; */
/* 		} */
/* 	} */
/* } */
