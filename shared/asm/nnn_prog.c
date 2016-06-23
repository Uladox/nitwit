#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NIT_SHORT_NAMES
#include <nitlib/list.h>

#include <spar/core.h>
#include <spar/text_utils.h>

#include "../vm/vm.h"
#include "../vm/vm_data.h"
#include "nnn_parser.h"
#include "nnn_expr.h"
#include "nnn_prog.h"

void
nnn_prog_push(struct nnn_prog *prog, struct nnn_expr *expr)
{
	LIST_CONS(expr, prog->stack);
	prog->stack = expr;
}

struct nnn_expr *
nnn_prog_pop(struct nnn_prog *prog)
{
	if (prog->stack) {
		struct nnn_expr *tmp = prog->stack;

		prog->stack = LIST_NEXT(tmp);
		return tmp;
	}

	return malloc(sizeof(*prog->stack));
}

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
bad_cmd_skip(struct spar_lexinfo *info, struct spar_token *token)
{
	for (;; ++info->dat.text) {
		switch (*info->dat.text) {
		case ';':
			token->type = nnn_type_semi;
			++info->dat.text;
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

static inline int
extra_semi(struct nnn_expr *expr1, struct nnn_expr *expr2)
{
	return expr1->type == NTWT_SEMI &&
		expr2->type == NTWT_SEMI;
}

static inline struct nnn_expr *
first_expr(struct nnn_prog *prog, struct spar_lexinfo *info,
	   struct spar_token *token, enum spar_parsed *parsed)
{
	struct nnn_expr *expr = nnn_expr_get(prog, info, token, parsed);

	while (expr->type != NTWT_EOI &&
	       expr->dat.op_code == NTWT_OP_INVALID) {
		bad_cmd_skip(info, token);
		nnn_prog_push(prog, expr);
	        expr = nnn_expr_get(prog, info, token, parsed);
	}

	return expr;
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
	prog->expr = (expr = first_expr(prog, &info, &token, &parsed));

	while (expr->type != NTWT_EOI) {
	        expr2 = nnn_expr_get(prog, &info, &token, &parsed);

		if (is_bad_cmd(expr, expr2)) {
			bad_cmd_skip(&info, &token);
		        nnn_prog_push(prog, expr2);
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

void
nnn_prog_type_check(struct nnn_prog *prog, int *error)
{
	struct nnn_expr *expr = prog->expr;
	int args = -1;

	struct nnn_expr *op;
	const char *op_name;
	const int *params;

	foreach (expr) {
		if (expr->type == NTWT_EOI)
			return;

		if (expr->type == NTWT_SEMI) {
			if (args < params[0]) {
				*error = 1;
				fprintf(stderr,
					"Error: too few arguments to %s on "
					"line %zu, expected %u, got %u\n",
					op_name, op->line, params[0], args);
			}

			args = -1;
			continue;
		}

		++args;

		if (args == 0) {
			op = expr;
			op_name = ntwt_op_name[(uint8_t) op->dat.op_code];
			params = ntwt_op_args[(uint8_t) op->dat.op_code];
			continue;
		}


		if (args > params[0]) {
			*error = 1;

			expr = LIST_NEXT(expr);
			foreach (expr) {
				if (expr->type == NTWT_SEMI)
					break;
				++args;
			}

			fprintf(stderr,
				"Error: too many arguments to %s on line %zu, "
				"expected %u, got %i.\n",
			        op_name, op->line, params[0], args);

			args = -1;
			continue;
		}

		if (params[args] != expr->type) {
			*error = 1;
			fprintf(stderr,
				"Error: on line %zu expected type %s, got %s.\n",
				expr->line, ntwt_type_name[params[args]],
				ntwt_type_name[expr->type]);
		}
	}
}
